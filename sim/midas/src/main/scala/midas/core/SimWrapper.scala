// See LICENSE for license details.

package midas
package core


import midas.widgets.{BridgeIOAnnotation, TimestampedToken}
import midas.passes.fame
import midas.passes.fame.{FAMEChannelConnectionAnnotation, FAMEChannelInfo}
import midas.core.SimUtils._

// from rocketchip
import freechips.rocketchip.config.{Parameters, Field}

import chisel3._
import chisel3.util._
import chisel3.experimental.{Direction, chiselName, ChiselAnnotation, annotate}
import chisel3.experimental.DataMirror.directionOf
import firrtl.annotations.{SingleTargetAnnotation, ReferenceTarget}

import scala.collection.immutable.ListMap
import scala.collection.mutable.{ArrayBuffer}

case object SimWrapperKey extends Field[SimWrapperConfig]

private[midas] case class TargetBoxAnnotation(target: ReferenceTarget) extends SingleTargetAnnotation[ReferenceTarget] {
  def duplicate(rt: ReferenceTarget): TargetBoxAnnotation = TargetBoxAnnotation(rt)
}

// Regenerates the "bits" field of a target ready-valid interface from a list of flattened
// elements that include the "bits_" prefix. This is stripped off.
class PayloadRecord(elms: Seq[(String, Data)]) extends Record {
  override val elements = ListMap((elms map { case (name, data) => name.stripPrefix("bits_") -> data.cloneType }):_*)
  override def cloneType: this.type = new PayloadRecord(elms).asInstanceOf[this.type]
}

// The regenerated form of a Vec[Clock] that has been lowered. Use this to
// represent the IO on the transformed target.
class ClockRecord(numClocks: Int) extends Record {
  override val elements = ListMap(Seq.tabulate(numClocks)(i => s"_$i" -> Clock()):_*)
  override def cloneType = new ClockRecord(numClocks).asInstanceOf[this.type]
}

/**
  * Used to implement channel interfaces both on the transformed target (see
  * subclass [[TargetBoxIO]]) and the generated simulation wrapper (see
  * subclass [[SimWrapperChannels]]]), as function of FCCAs present on the
  * tranformed target.
  *
  * @param chAnnos FCCAs describing channel connectivity. Channels with both
  * sources and sinks (loopback) will have a pair of generated ports
  *
  * @param leafTypeMap A means to lookup the FIRRTL type FCCA ReferenceTargets
  * point at. This will then be used to regenerate a connection-complaint
  * chisel type.
  */
abstract class ChannelizedWrapperIO(
    chAnnos: Seq[FAMEChannelConnectionAnnotation],
    leafTypeMap: Map[ReferenceTarget, firrtl.ir.Port]) extends Record {

  /**
    * Clocks have different types coming off the target (where they retain
    * their target-native Clock type) vs leaving the simulation wrapper
    * (where they have been coerced to Bool).
    */
  def regenClockType(refTargets: Seq[ReferenceTarget]): TimestampedToken[_]

  def regenTypesFromField(name: String, tpe: firrtl.ir.Type): Seq[(String, Data)] = tpe match {
    case firrtl.ir.BundleType(fields) => fields.flatMap(f => regenTypesFromField(prefixWith(name, f.name), f.tpe))
    case firrtl.ir.UIntType(width: firrtl.ir.IntWidth) => Seq(name -> UInt(width.width.toInt.W))
    case firrtl.ir.SIntType(width: firrtl.ir.IntWidth) => Seq(name -> SInt(width.width.toInt.W))
    case _ => throw new RuntimeException(s"Unexpected type in token payload: ${tpe}.")
  }

  def regenTypes(refTargets: Seq[ReferenceTarget]): Seq[(String, Data)] = {
    val port = leafTypeMap(refTargets.head.copy(component = Seq()))
    val fieldName = refTargets.head.component match {
      case firrtl.annotations.TargetToken.Field(fName) :: Nil => fName
      case firrtl.annotations.TargetToken.Field(fName) :: fields => fName
      case _ => throw new RuntimeException("Expected only a bits field in ReferenceTarget's component.")
    }

    val bitsField = port.tpe match {
      case a: firrtl.ir.BundleType => a.fields.filter(_.name == fieldName).head
      case _ => throw new RuntimeException("ReferenceTargets should point at the channel's bundle.")
    }

    regenTypesFromField("", bitsField.tpe)
  }

  def regenPayloadType(refTargets: Seq[ReferenceTarget]): Data = {
    require(!refTargets.isEmpty)
    // Reject all (String -> Data) pairs not included in the refTargets
    // Use this to remove target valid
    val targetLeafNames = refTargets.map(_.component.reverse.head.value).toSet
    val elements = regenTypes(refTargets).filter({ case (name, f)  => targetLeafNames(name) })
    elements match {
      case (name, field) :: Nil => field // If there's only a single field, just pass out the type
      case elms => new PayloadRecord(elms)
    }
  }

  def regenWireType(chInfo: FAMEChannelInfo, refTargets: Seq[ReferenceTarget]): Data = {
    chInfo match {
      case fame.TargetClockChannel(_) =>  regenClockType(refTargets)
      case fame.ClockControlChannel   =>
        require(refTargets.size == 1, "FIXME: Handle aggregated wires")
        new TimestampedToken(regenTypes(refTargets).head._2)
      case fame.PipeChannel(_) =>
        require(refTargets.size == 1, "FIXME: Handle aggregated wires")
        regenTypes(refTargets).head._2
      case o => ???
    }
  }

  val payloadTypeMap: Map[FAMEChannelConnectionAnnotation, Data] = chAnnos.collect({
    // Target Decoupled Channels need to have their target-valid ReferenceTarget removed
    case ch @ FAMEChannelConnectionAnnotation(_,fame.DecoupledForwardChannel(_,Some(vsrc),_,_), _, Some(srcs),_) =>
      ch -> regenPayloadType(srcs.filterNot(_ == vsrc))
    case ch @ FAMEChannelConnectionAnnotation(_,fame.DecoupledForwardChannel(_,_,_,Some(vsink)), _, _, Some(sinks)) =>
      ch -> regenPayloadType(sinks.filterNot(_ == vsink))
  }).toMap

  val wireElements = ArrayBuffer[(String, ReadyValidIO[Data])]()
  val wireLikeFCCAs = chAnnos.collect {
    case ch @ FAMEChannelConnectionAnnotation(_,fame.PipeChannel(_),_,_,_) => ch
    case ch @ FAMEChannelConnectionAnnotation(_,fame.ClockControlChannel,_,_,_) => ch
    case ch @ FAMEChannelConnectionAnnotation(_,fame.TargetClockChannel(_),_,_,_) => ch
  }

  val wireTypeMap: Map[FAMEChannelConnectionAnnotation, Data] = wireLikeFCCAs.collect({
    case ch @ FAMEChannelConnectionAnnotation(_,chInfo,_,Some(srcs),_) => ch -> regenWireType(chInfo, srcs)
    case ch @ FAMEChannelConnectionAnnotation(_,chInfo,_,_,Some(sinks)) => ch -> regenWireType(chInfo, sinks)
  }).toMap


  val wirePortMap: Map[String, WirePortTuple] = wireLikeFCCAs.map({ chAnno =>
    val FAMEChannelConnectionAnnotation(globalName, chInfo, _, sources, sinks) = chAnno
    val sinkP = sinks.map({ tRefs =>
      val name = tRefs.head.ref.stripSuffix("_bits")
      val port = Flipped(Decoupled(wireTypeMap(chAnno)))
      wireElements += name -> port
      port
    })
    val sourceP = sources.map({ tRefs =>
      val name = tRefs.head.ref.stripSuffix("_bits")
      val port = Decoupled(wireTypeMap(chAnno))
      wireElements += name -> port
      port
    })
    (globalName -> WirePortTuple(sourceP, sinkP))
  }).toMap

  // Looks up a  channel based on a channel name
  val wireOutputPortMap = wirePortMap.collect({
    case (name, portTuple) if portTuple.isOutput => name -> portTuple.source.get
  })

  val wireInputPortMap = wirePortMap.collect({
    case (name, portTuple) if portTuple.isInput => name -> portTuple.sink.get
  })


  val rvElements = ArrayBuffer[(String, ReadyValidIO[Data])]()

  // Using a channel's globalName; look up it's associated port tuple
  val rvPortMap: Map[String, TargetRVPortTuple] = chAnnos.collect({
    case ch @ FAMEChannelConnectionAnnotation(globalName, info@fame.DecoupledForwardChannel(_,_,_,_), _, leafSources, leafSinks) =>
      val sourcePortPair = leafSources.map({ tRefs =>
        require(!tRefs.isEmpty, "FIXME: Are empty decoupleds OK?")
        val validTRef: ReferenceTarget = info.validSource.getOrElse(throw new RuntimeException(
          "Target RV port has leaves but no TRef to a validSource"))
        val readyTRef: ReferenceTarget = info.readySink.getOrElse(throw new RuntimeException(
           "Target RV port has leaves but no TRef to a readySink"))

        val fwdName = validTRef.ref
        val fwdPort = Decoupled(Valid(payloadTypeMap(ch)))
        val revName = readyTRef.ref
        val revPort = Flipped(Decoupled(Bool()))
        rvElements ++= Seq((fwdName -> fwdPort), (revName -> revPort))
        (fwdPort, revPort)
      })

      val sinkPortPair = leafSinks.map({ tRefs =>
        require(!tRefs.isEmpty, "FIXME: Are empty decoupleds OK?")
        val validTRef: ReferenceTarget = info.validSink.getOrElse(throw new RuntimeException(
          "Target RV port has payload sinks but no TRef to a validSink"))
        val readyTRef: ReferenceTarget = info.readySource.getOrElse(throw new RuntimeException(
          "Target RV port has payload sinks but no TRef to a readySource"))

        val fwdName = validTRef.ref
        val fwdPort = Flipped(Decoupled(Valid(payloadTypeMap(ch))))
        val revName = readyTRef.ref
        val revPort = Decoupled(Bool())
        rvElements ++= Seq((fwdName -> fwdPort), (revName -> revPort))
        (fwdPort, revPort)
      })
      globalName -> TargetRVPortTuple(sourcePortPair, sinkPortPair)
  }).toMap

  // Looks up a  channel based on a channel name
  val rvOutputPortMap = rvPortMap.collect({
    case (name, portTuple) if portTuple.isOutput => name -> portTuple.source.get
  })

  val rvInputPortMap = rvPortMap.collect({
    case (name, portTuple) if portTuple.isInput => name -> portTuple.sink.get
  })

  // Looks up a FCCA based on a global channel name
  val chNameToAnnoMap = chAnnos.map(anno => anno.globalName -> anno)
}

/**
  * A chisel Record that when elaborated and lowered should match the I/O
  * coming off the transformed target, such that it can be linked against
  * simulation wrapper without FIRRTL type errors.
  */
class TargetBoxIO(val chAnnos: Seq[FAMEChannelConnectionAnnotation],
                   leafTypeMap: Map[ReferenceTarget, firrtl.ir.Port])
                  extends ChannelizedWrapperIO(chAnnos, leafTypeMap) {

  def regenClockType(refTargets: Seq[ReferenceTarget]): TimestampedToken[Data] = new TimestampedToken(refTargets.size match {
    // "Aggregate-ness" of single-field vecs and bundles are removed by the
    // fame transform (their only field is provided as bits) leading to the
    // special casing here
    case 1 => Clock()
    case size => new ClockRecord(refTargets.size)
  })

  val hostClock = Input(Clock())
  val hostReset = Input(Bool())
  override val elements = ListMap((wireElements ++ rvElements):_*) ++
    // Untokenized ports
    ListMap("hostClock" -> hostClock, "hostReset" -> hostReset)
  override def cloneType: this.type = new TargetBoxIO(chAnnos, leafTypeMap).asInstanceOf[this.type]
}

/**
  * A blackbox representing the transformed target. This will be replaced during
  * linking with the actual transformed target's module hierarchy.
  */
class TargetBox(chAnnos: Seq[FAMEChannelConnectionAnnotation],
               leafTypeMap: Map[ReferenceTarget, firrtl.ir.Port]) extends BlackBox {
  val io = IO(new TargetBoxIO(chAnnos, leafTypeMap))
}

class SimWrapperChannels(val chAnnos: Seq[FAMEChannelConnectionAnnotation],
                         val bridgeAnnos: Seq[BridgeIOAnnotation],
                         leafTypeMap: Map[ReferenceTarget, firrtl.ir.Port])
    extends ChannelizedWrapperIO(chAnnos, leafTypeMap) {

  def regenClockType(refTargets: Seq[ReferenceTarget]): TimestampedToken[Data] = {
    new TimestampedToken(if (refTargets.size == 1) Bool() else Vec(refTargets.size, Bool()))
  }

  override val elements = ListMap((wireElements ++ rvElements):_*)
  override def cloneType: this.type = new SimWrapperChannels(chAnnos, bridgeAnnos, leafTypeMap).asInstanceOf[this.type]
}

case class SimWrapperConfig(chAnnos: Seq[FAMEChannelConnectionAnnotation],
                         bridgeAnnos: Seq[BridgeIOAnnotation],
                         leafTypeMap: Map[ReferenceTarget, firrtl.ir.Port])

/**
  * Instantiates all simulation channels based on
  * FAMEChannelConnectionAnnotations provided by the transformed target
  * circuit. There are three types of connection directivity:
  *
  * FPGATop  |    SimWrapper  | Target
  * Bridge  => Channel Queue =>  Hub Model  (1) Output Channels: FCCA.sinks == None
  * Bridge  <= Channel Queue <=  Hub Model  (2) Input Channels: FCCA.sources == None
  *                 Channel  <=   ModelA    (3) Loopback Channels
  *                 Queue    =>   ModelB
  *
  */
class SimWrapper(config: SimWrapperConfig)(implicit val p: Parameters) extends MultiIOModule {

  outer =>
  val SimWrapperConfig(chAnnos, bridgeAnnos, leafTypeMap) = config

  // Remove all FCAs that are loopback channels. All non-loopback FCAs connect
  // to bridges and will be presented in the SimWrapper's IO
  val bridgeChAnnos = chAnnos.collect({
    case fca @ FAMEChannelConnectionAnnotation(_,_,_,_,None) => fca
    case fca @ FAMEChannelConnectionAnnotation(_,_,_,None,_) => fca
  })

  val channelPorts = IO(new SimWrapperChannels(bridgeChAnnos, bridgeAnnos, leafTypeMap))
  val target = Module(new TargetBox(chAnnos, leafTypeMap))

  // Indicates SimulationMapping which module we want to replace with the simulator
  annotate(new ChiselAnnotation { def toFirrtl =
    TargetBoxAnnotation(outer.toNamed.toTarget.ref(target.instanceName))
  })

  target.io.hostReset := reset.toBool
  target.io.hostClock := clock
  import chisel3.ExplicitCompileOptions.NotStrict // FIXME

  def getPipeChannelType(chAnno: FAMEChannelConnectionAnnotation): ChLeafType = {
    if (chAnno.sources.isEmpty || chAnno.sinks.isEmpty) {
      // User the outer-type if non-loopback because here clocks have been coerced to Bool
      channelPorts.wireTypeMap(chAnno)
    } else {
      // But defer to inner-type on loopback channels because the outer-type is not defined
      target.io.wireTypeMap(chAnno)
    }
  }

  def genPipeChannel(chAnno: FAMEChannelConnectionAnnotation, latency: Int = 1): PipeChannel[ChLeafType] = { require(chAnno.sources == None || chAnno.sources.get.size == 1, "Can't aggregate wire-type channels yet")
    val channel = Module(new PipeChannel(getPipeChannelType(chAnno), latency))
    channel suggestName s"PipeChannel_${chAnno.globalName}"

    val portTuple = target.io.wirePortMap(chAnno.globalName)
    portTuple.source match {
      case Some(srcP) => channel.io.in <> srcP
      case None => channel.io.in <> channelPorts.elements(s"${chAnno.globalName}_sink")
    }

    portTuple.sink match {
      case Some(sinkP) =>
        // Splay out the assignment so that we can coerce the bridge-side types
        // (which use Bool in place of Clock, AsyncReset) to the hub model-side
        // types which match those natively present in the target
        sinkP.valid := channel.io.out.valid
        sinkP.bits := channel.io.out.bits.asUInt.asTypeOf(sinkP.bits)
        channel.io.out.ready := sinkP.ready
      case None => channelPorts.elements(s"${chAnno.globalName}_source") <> channel.io.out
    }
    channel
  }

  // Helper functions to attach legacy SimReadyValidIO to true, dual-channel implementations of target ready-valid
  def bindRVChannelEnq[T <: Data](enq: SimReadyValidIO[T], port: TargetRVPortType): Unit = {
    val (fwdPort, revPort) = port
    enq.fwd.hValid   := fwdPort.valid
    enq.target.valid := fwdPort.bits.valid
    enq.target.bits  := fwdPort.bits.bits  // Yeah, i know
    fwdPort.ready := enq.fwd.hReady

    // Connect up the target-ready token channel
    revPort.valid := enq.rev.hValid
    revPort.bits  := enq.target.ready
    enq.rev.hReady := revPort.ready
  }

  def bindRVChannelDeq[T <: Data](deq: SimReadyValidIO[T], port: TargetRVPortType): Unit = {
    val (fwdPort, revPort) = port
    deq.fwd.hReady := fwdPort.ready
    fwdPort.valid      := deq.fwd.hValid
    fwdPort.bits.valid := deq.target.valid
    fwdPort.bits.bits  := deq.target.bits

    // Connect up the target-ready token channel
    deq.rev.hValid   := revPort.valid
    deq.target.ready := revPort.bits
    revPort.ready := deq.rev.hReady
  }


  def getReadyValidChannelType(chAnno: FAMEChannelConnectionAnnotation): Data = {
    target.io.payloadTypeMap(chAnno)
  }

  def genReadyValidChannel(chAnno: FAMEChannelConnectionAnnotation): ReadyValidChannel[Data] = {
      val chName = chAnno.globalName
      val strippedName = chName.stripSuffix("_fwd")
      // A channel is considered "flipped" if it's sunk by the tranformed RTL (sourced by an bridge)
      val channel = Module(new ReadyValidChannel(getReadyValidChannelType(chAnno).cloneType))

      channel.suggestName(s"ReadyValidChannel_$strippedName")

      val enqPortPair = (chAnno.sources match {
        case Some(_) => target.io.rvOutputPortMap(chName)
        case None => channelPorts.rvInputPortMap(chName)
      })
      bindRVChannelEnq(channel.io.enq, enqPortPair)

      val deqPortPair = (chAnno.sinks match {
        case Some(_) => target.io.rvInputPortMap(chName)
        case None => channelPorts.rvOutputPortMap(chName)
      })
      bindRVChannelDeq(channel.io.deq, deqPortPair)

      channel.io.targetReset.bits := false.B
      channel.io.targetReset.valid := true.B
      channel
  }

  // Generate all ready-valid channels
  val rvChannels = chAnnos.collect({
    case ch @ FAMEChannelConnectionAnnotation(_,fame.DecoupledForwardChannel(_,_,_,_),_,_,_) => genReadyValidChannel(ch)
  })

  // Generate all other non-RV channels
  chAnnos.collect({
    case ch @ FAMEChannelConnectionAnnotation(name, fame.PipeChannel(latency),_,_,_)  => genPipeChannel(ch, latency)
    case ch @ FAMEChannelConnectionAnnotation(name, fame.ClockControlChannel,_,_,_)  => genPipeChannel(ch, 0)
    case ch @ FAMEChannelConnectionAnnotation(_, fame.TargetClockChannel(_),_,_,_)  => genPipeChannel(ch, 0)
  })
}
