//See LICENSE for license details
#include "firesim_top.h"

// FireSim library bridge drivers
// From firesim-lib/src/main/cc/bridges
#include "bridges/serial.h"
#include "bridges/uart.h"
#include "bridges/simplenic.h"
#include "bridges/blockdev.h"
#include "bridges/tracerv.h"
#include "bridges/groundtest.h"
#include "bridges/autocounter.h"

// Golden Gate provided bridge drivers
#include "bridges/fpga_model.h"
#include "bridges/fased_memory_timing_model.h"
#include "bridges/synthesized_assertions.h"
#include "bridges/synthesized_prints.h"

firesim_top_t::firesim_top_t(int argc, char** argv)
{
    std::vector<std::string> args(argv + 1, argv + argc);
    max_cycles = -1;
    profile_interval = max_cycles;

    for (auto &arg: args) {
        if (arg.find("+max-cycles=") == 0) {
            max_cycles = atoi(arg.c_str()+12);
        }
        if (arg.find("+profile-interval=") == 0) {
            profile_interval = atoi(arg.c_str()+18);
        }
        if (arg.find("+zero-out-dram") == 0) {
            do_zero_out_dram = true;
        }
    }


// DOC include start: UART Bridge Driver Registration
    // Here we instantiate our driver once for each bridge in the target
    // Golden Gate emits a <BridgeModuleClassName>_<id>_PRESENT macro for each instance
    // which you may use to conditionally instantiate your driver
    #ifdef UARTBRIDGEMODULE_0_PRESENT
    // Create an instance of the constructor argument (this has all of
    // addresses of the BridgeModule's memory mapped registers)
    UARTBRIDGEMODULE_0_substruct_create;
    // Instantiate the driver; register it in the main simulation class
    add_bridge_driver(new uart_t(this, UARTBRIDGEMODULE_0_substruct, 0));
    #endif

    // Repeat the code above with modified indices as many times as necessary
    // to support the maximum expected number of bridge instances
    #ifdef UARTBRIDGEMODULE_1_PRESENT
    UARTBRIDGEMODULE_1_substruct_create;
    add_bridge_driver(new uart_t(this, UARTBRIDGEMODULE_1_substruct, 1));
    #endif
// DOC include end: UART Bridge Driver Registration
    #ifdef UARTBRIDGEMODULE_2_PRESENT
    UARTBRIDGEMODULE_2_substruct_create;
    add_bridge_driver(new uart_t(this, UARTBRIDGEMODULE_2_substruct, 2));
    #endif
    #ifdef UARTBRIDGEMODULE_3_PRESENT
    UARTBRIDGEMODULE_3_substruct_create;
    add_bridge_driver(new uart_t(this, UARTBRIDGEMODULE_3_substruct, 3));
    #endif
    #ifdef UARTBRIDGEMODULE_4_PRESENT
    UARTBRIDGEMODULE_4_substruct_create;
    add_bridge_driver(new uart_t(this, UARTBRIDGEMODULE_4_substruct, 4));
    #endif
    #ifdef UARTBRIDGEMODULE_5_PRESENT
    UARTBRIDGEMODULE_5_substruct_create;
    add_bridge_driver(new uart_t(this, UARTBRIDGEMODULE_5_substruct, 5));
    #endif
    #ifdef UARTBRIDGEMODULE_6_PRESENT
    UARTBRIDGEMODULE_6_substruct_create;
    add_bridge_driver(new uart_t(this, UARTBRIDGEMODULE_6_substruct, 6));
    #endif
    #ifdef UARTBRIDGEMODULE_7_PRESENT
    UARTBRIDGEMODULE_7_substruct_create;
    add_bridge_driver(new uart_t(this, UARTBRIDGEMODULE_7_substruct, 7));
    #endif

std::vector<uint64_t> host_mem_offsets;
uint64_t host_mem_offset = -0x80000000LL;
uint64_t lastChannel;
#ifdef FASEDMEMORYTIMINGMODEL_0
    fpga_models.push_back(new FASEDMemoryTimingModel(
                this,
                // Casts are required for now since the emitted type can change...
                AddressMap(FASEDMEMORYTIMINGMODEL_0_R_num_registers,
                    (const unsigned int*) FASEDMEMORYTIMINGMODEL_0_R_addrs,
                    (const char* const*) FASEDMEMORYTIMINGMODEL_0_R_names,
                    FASEDMEMORYTIMINGMODEL_0_W_num_registers,
                    (const unsigned int*) FASEDMEMORYTIMINGMODEL_0_W_addrs,
                    (const char* const*) FASEDMEMORYTIMINGMODEL_0_W_names),
                argc, argv, "memory_stats.csv",
                1L << FASEDMEMORYTIMINGMODEL_0_target_addr_bits,
                host_mem_offset, "_0"));
     host_mem_offsets.push_back(host_mem_offset);
     lastChannel = FASEDMEMORYTIMINGMODEL_0_last_channel;
     host_mem_offset += (lastChannel << FASEDMEMORYTIMINGMODEL_0_target_addr_bits);
#endif

#ifdef FASEDMEMORYTIMINGMODEL_1
    fpga_models.push_back(new FASEDMemoryTimingModel(
                this,
                // Casts are required for now since the emitted type can change...
                AddressMap(FASEDMEMORYTIMINGMODEL_1_R_num_registers,
                    (const unsigned int*) FASEDMEMORYTIMINGMODEL_1_R_addrs,
                    (const char* const*) FASEDMEMORYTIMINGMODEL_1_R_names,
                    FASEDMEMORYTIMINGMODEL_1_W_num_registers,
                    (const unsigned int*) FASEDMEMORYTIMINGMODEL_1_W_addrs,
                    (const char* const*) FASEDMEMORYTIMINGMODEL_1_W_names),
                argc, argv, "memory_stats1.csv",
                1L << FASEDMEMORYTIMINGMODEL_1_target_addr_bits,
                host_mem_offset, "_1"));
     host_mem_offsets.push_back(host_mem_offset);
     lastChannel = FASEDMEMORYTIMINGMODEL_1_last_channel;
     host_mem_offset += (lastChannel << FASEDMEMORYTIMINGMODEL_1_target_addr_bits);
#endif

#ifdef FASEDMEMORYTIMINGMODEL_2
    fpga_models.push_back(new FASEDMemoryTimingModel(
                this,
                // Casts are required for now since the emitted type can change...
                AddressMap(FASEDMEMORYTIMINGMODEL_2_R_num_registers,
                    (const unsigned int*) FASEDMEMORYTIMINGMODEL_2_R_addrs,
                    (const char* const*) FASEDMEMORYTIMINGMODEL_2_R_names,
                    FASEDMEMORYTIMINGMODEL_2_W_num_registers,
                    (const unsigned int*) FASEDMEMORYTIMINGMODEL_2_W_addrs,
                    (const char* const*) FASEDMEMORYTIMINGMODEL_2_W_names),
                argc, argv, "memory_stats2.csv",
                1L << FASEDMEMORYTIMINGMODEL_2_target_addr_bits,
                host_mem_offset, "_2"));
     host_mem_offsets.push_back(host_mem_offset);
     lastChannel = FASEDMEMORYTIMINGMODEL_2_last_channel;
     host_mem_offset += (lastChannel << FASEDMEMORYTIMINGMODEL_2_target_addr_bits);
#endif

#ifdef FASEDMEMORYTIMINGMODEL_3
    fpga_models.push_back(new FASEDMemoryTimingModel(
                this,
                // Casts are required for now since the emitted type can change...
                AddressMap(FASEDMEMORYTIMINGMODEL_3_R_num_registers,
                    (const unsigned int*) FASEDMEMORYTIMINGMODEL_3_R_addrs,
                    (const char* const*) FASEDMEMORYTIMINGMODEL_3_R_names,
                    FASEDMEMORYTIMINGMODEL_3_W_num_registers,
                    (const unsigned int*) FASEDMEMORYTIMINGMODEL_3_W_addrs,
                    (const char* const*) FASEDMEMORYTIMINGMODEL_3_W_names),
                argc, argv, "memory_stats3.csv",
                1L << FASEDMEMORYTIMINGMODEL_3_target_addr_bits,
                host_mem_offset, "_3"));
     host_mem_offsets.push_back(host_mem_offset);
     lastChannel = FASEDMEMORYTIMINGMODEL_3_last_channel;
     host_mem_offset += (lastChannel << FASEDMEMORYTIMINGMODEL_3_target_addr_bits);
#endif

#ifdef FASEDMEMORYTIMINGMODEL_4
    fpga_models.push_back(new FASEDMemoryTimingModel(
                this,
                // Casts are required for now since the emitted type can change...
                AddressMap(FASEDMEMORYTIMINGMODEL_4_R_num_registers,
                    (const unsigned int*) FASEDMEMORYTIMINGMODEL_4_R_addrs,
                    (const char* const*) FASEDMEMORYTIMINGMODEL_4_R_names,
                    FASEDMEMORYTIMINGMODEL_4_W_num_registers,
                    (const unsigned int*) FASEDMEMORYTIMINGMODEL_4_W_addrs,
                    (const char* const*) FASEDMEMORYTIMINGMODEL_4_W_names),
                argc, argv, "memory_stats4.csv",
                1L << FASEDMEMORYTIMINGMODEL_4_target_addr_bits,
                host_mem_offset, "_4"));
     host_mem_offsets.push_back(host_mem_offset);
     lastChannel = FASEDMEMORYTIMINGMODEL_4_last_channel;
     host_mem_offset += (lastChannel << FASEDMEMORYTIMINGMODEL_4_target_addr_bits);
#endif

#ifdef FASEDMEMORYTIMINGMODEL_5
    fpga_models.push_back(new FASEDMemoryTimingModel(
                this,
                // Casts are required for now since the emitted type can change...
                AddressMap(FASEDMEMORYTIMINGMODEL_5_R_num_registers,
                    (const unsigned int*) FASEDMEMORYTIMINGMODEL_5_R_addrs,
                    (const char* const*) FASEDMEMORYTIMINGMODEL_5_R_names,
                    FASEDMEMORYTIMINGMODEL_5_W_num_registers,
                    (const unsigned int*) FASEDMEMORYTIMINGMODEL_5_W_addrs,
                    (const char* const*) FASEDMEMORYTIMINGMODEL_5_W_names),
                argc, argv, "memory_stats5.csv",
                1L << FASEDMEMORYTIMINGMODEL_5_target_addr_bits,
                host_mem_offset, "_5"));
     host_mem_offsets.push_back(host_mem_offset);
     lastChannel = FASEDMEMORYTIMINGMODEL_5_last_channel;
     host_mem_offset += (lastChannel << FASEDMEMORYTIMINGMODEL_5_target_addr_bits);
#endif

#ifdef FASEDMEMORYTIMINGMODEL_6
    fpga_models.push_back(new FASEDMemoryTimingModel(
                this,
                // Casts are required for now since the emitted type can change...
                AddressMap(FASEDMEMORYTIMINGMODEL_6_R_num_registers,
                    (const unsigned int*) FASEDMEMORYTIMINGMODEL_6_R_addrs,
                    (const char* const*) FASEDMEMORYTIMINGMODEL_6_R_names,
                    FASEDMEMORYTIMINGMODEL_6_W_num_registers,
                    (const unsigned int*) FASEDMEMORYTIMINGMODEL_6_W_addrs,
                    (const char* const*) FASEDMEMORYTIMINGMODEL_6_W_names),
                argc, argv, "memory_stats6.csv",
                1L << FASEDMEMORYTIMINGMODEL_6_target_addr_bits,
                host_mem_offset, "_6"));
     host_mem_offsets.push_back(host_mem_offset);
     lastChannel = FASEDMEMORYTIMINGMODEL_6_last_channel;
     host_mem_offset += (lastChannel << FASEDMEMORYTIMINGMODEL_6_target_addr_bits);
#endif

#ifdef FASEDMEMORYTIMINGMODEL_7
    fpga_models.push_back(new FASEDMemoryTimingModel(
                this,
                // Casts are required for now since the emitted type can change...
                AddressMap(FASEDMEMORYTIMINGMODEL_7_R_num_registers,
                    (const unsigned int*) FASEDMEMORYTIMINGMODEL_7_R_addrs,
                    (const char* const*) FASEDMEMORYTIMINGMODEL_7_R_names,
                    FASEDMEMORYTIMINGMODEL_7_W_num_registers,
                    (const unsigned int*) FASEDMEMORYTIMINGMODEL_7_W_addrs,
                    (const char* const*) FASEDMEMORYTIMINGMODEL_7_W_names),
                argc, argv, "memory_stats7.csv",
                1L << FASEDMEMORYTIMINGMODEL_7_target_addr_bits,
                host_mem_offset, "_7"));
     host_mem_offsets.push_back(host_mem_offset);
     lastChannel = FASEDMEMORYTIMINGMODEL_7_last_channel;
     host_mem_offset += (lastChannel << FASEDMEMORYTIMINGMODEL_7_target_addr_bits);
#endif

#ifdef SERIALBRIDGEMODULE_struct_guard
    #ifdef SERIALBRIDGEMODULE_0_PRESENT
    SERIALBRIDGEMODULE_0_substruct_create;
    add_bridge_driver(new serial_t(this, args, SERIALBRIDGEMODULE_0_substruct, 0, host_mem_offsets[0]));
    #endif
    #ifdef SERIALBRIDGEMODULE_1_PRESENT
    SERIALBRIDGEMODULE_1_substruct_create;
    add_bridge_driver(new serial_t(this, args, SERIALBRIDGEMODULE_1_substruct, 1, host_mem_offsets[1]));
    #endif
    #ifdef SERIALBRIDGEMODULE_2_PRESENT
    SERIALBRIDGEMODULE_2_substruct_create;
    add_bridge_driver(new serial_t(this, args, SERIALBRIDGEMODULE_2_substruct, 2, host_mem_offsets[2]));
    #endif
    #ifdef SERIALBRIDGEMODULE_3_PRESENT
    SERIALBRIDGEMODULE_3_substruct_create;
    add_bridge_driver(new serial_t(this, args, SERIALBRIDGEMODULE_3_substruct, 3, host_mem_offsets[3]));
    #endif
    #ifdef SERIALBRIDGEMODULE_4_PRESENT
    SERIALBRIDGEMODULE_4_substruct_create;
    add_bridge_driver(new serial_t(this, args, SERIALBRIDGEMODULE_4_substruct, 4, host_mem_offsets[4]));
    #endif
    #ifdef SERIALBRIDGEMODULE_5_PRESENT
    SERIALBRIDGEMODULE_5_substruct_create;
    add_bridge_driver(new serial_t(this, args, SERIALBRIDGEMODULE_5_substruct, 5, host_mem_offsets[5]));
    #endif
    #ifdef SERIALBRIDGEMODULE_6_PRESENT
    SERIALBRIDGEMODULE_6_substruct_create;
    add_bridge_driver(new serial_t(this, args, SERIALBRIDGEMODULE_6_substruct, 6, host_mem_offsets[6]));
    #endif
    #ifdef SERIALBRIDGEMODULE_7_PRESENT
    SERIALBRIDGEMODULE_7_substruct_create;
    add_bridge_driver(new serial_t(this, args, SERIALBRIDGEMODULE_7_substruct, 7, host_mem_offsets[7]));
    #endif
#endif

#ifdef BLOCKDEVBRIDGEMODULE_struct_guard
    #ifdef BLOCKDEVBRIDGEMODULE_0_PRESENT
    BLOCKDEVBRIDGEMODULE_0_substruct_create;
    add_bridge_driver(new blockdev_t(this, args, BLOCKDEVBRIDGEMODULE_0_num_trackers, BLOCKDEVBRIDGEMODULE_0_latency_bits, BLOCKDEVBRIDGEMODULE_0_substruct, 0));
    #endif
    #ifdef BLOCKDEVBRIDGEMODULE_1_PRESENT
    BLOCKDEVBRIDGEMODULE_1_substruct_create;
    add_bridge_driver(new blockdev_t(this, args, BLOCKDEVBRIDGEMODULE_1_num_trackers, BLOCKDEVBRIDGEMODULE_1_latency_bits, BLOCKDEVBRIDGEMODULE_1_substruct, 1));
    #endif
    #ifdef BLOCKDEVBRIDGEMODULE_2_PRESENT
    BLOCKDEVBRIDGEMODULE_2_substruct_create;
    add_bridge_driver(new blockdev_t(this, args, BLOCKDEVBRIDGEMODULE_2_num_trackers, BLOCKDEVBRIDGEMODULE_2_latency_bits, BLOCKDEVBRIDGEMODULE_2_substruct, 2));
    #endif
    #ifdef BLOCKDEVBRIDGEMODULE_3_PRESENT
    BLOCKDEVBRIDGEMODULE_3_substruct_create;
    add_bridge_driver(new blockdev_t(this, args, BLOCKDEVBRIDGEMODULE_3_num_trackers, BLOCKDEVBRIDGEMODULE_3_latency_bits, BLOCKDEVBRIDGEMODULE_3_substruct, 3));
    #endif
    #ifdef BLOCKDEVBRIDGEMODULE_4_PRESENT
    BLOCKDEVBRIDGEMODULE_4_substruct_create;
    add_bridge_driver(new blockdev_t(this, args, BLOCKDEVBRIDGEMODULE_4_num_trackers, BLOCKDEVBRIDGEMODULE_4_latency_bits, BLOCKDEVBRIDGEMODULE_4_substruct, 4));
    #endif
    #ifdef BLOCKDEVBRIDGEMODULE_5_PRESENT
    BLOCKDEVBRIDGEMODULE_5_substruct_create;
    add_bridge_driver(new blockdev_t(this, args, BLOCKDEVBRIDGEMODULE_5_num_trackers, BLOCKDEVBRIDGEMODULE_5_latency_bits, BLOCKDEVBRIDGEMODULE_5_substruct, 5));
    #endif
    #ifdef BLOCKDEVBRIDGEMODULE_6_PRESENT
    BLOCKDEVBRIDGEMODULE_6_substruct_create;
    add_bridge_driver(new blockdev_t(this, args, BLOCKDEVBRIDGEMODULE_6_num_trackers, BLOCKDEVBRIDGEMODULE_6_latency_bits, BLOCKDEVBRIDGEMODULE_6_substruct, 6));
    #endif
    #ifdef BLOCKDEVBRIDGEMODULE_7_PRESENT
    BLOCKDEVBRIDGEMODULE_7_substruct_create;
    add_bridge_driver(new blockdev_t(this, args, BLOCKDEVBRIDGEMODULE_7_num_trackers, BLOCKDEVBRIDGEMODULE_7_latency_bits, BLOCKDEVBRIDGEMODULE_7_substruct, 7));
    #endif
#endif

#ifdef SIMPLENICBRIDGEMODULE_struct_guard
    #ifdef SIMPLENICBRIDGEMODULE_0_PRESENT
    SIMPLENICBRIDGEMODULE_0_substruct_create;
    add_bridge_driver(new simplenic_t(this, args, SIMPLENICBRIDGEMODULE_0_substruct, 0, SIMPLENICBRIDGEMODULE_0_DMA_ADDR));
    #endif
    #ifdef SIMPLENICBRIDGEMODULE_1_PRESENT
    SIMPLENICBRIDGEMODULE_1_substruct_create;
    add_bridge_driver(new simplenic_t(this, args, SIMPLENICBRIDGEMODULE_1_substruct, 1, SIMPLENICBRIDGEMODULE_1_DMA_ADDR));
    #endif
    #ifdef SIMPLENICBRIDGEMODULE_2_PRESENT
    SIMPLENICBRIDGEMODULE_2_substruct_create;
    add_bridge_driver(new simplenic_t(this, args, SIMPLENICBRIDGEMODULE_2_substruct, 2, SIMPLENICBRIDGEMODULE_2_DMA_ADDR));
    #endif
    #ifdef SIMPLENICBRIDGEMODULE_3_PRESENT
    SIMPLENICBRIDGEMODULE_3_substruct_create;
    add_bridge_driver(new simplenic_t(this, args, SIMPLENICBRIDGEMODULE_3_substruct, 3, SIMPLENICBRIDGEMODULE_3_DMA_ADDR));
    #endif
    #ifdef SIMPLENICBRIDGEMODULE_4_PRESENT
    SIMPLENICBRIDGEMODULE_4_substruct_create;
    add_bridge_driver(new simplenic_t(this, args, SIMPLENICBRIDGEMODULE_4_substruct, 4, SIMPLENICBRIDGEMODULE_4_DMA_ADDR));
    #endif
    #ifdef SIMPLENICBRIDGEMODULE_5_PRESENT
    SIMPLENICBRIDGEMODULE_5_substruct_create;
    add_bridge_driver(new simplenic_t(this, args, SIMPLENICBRIDGEMODULE_5_substruct, 5, SIMPLENICBRIDGEMODULE_5_DMA_ADDR));
    #endif
    #ifdef SIMPLENICBRIDGEMODULE_6_PRESENT
    SIMPLENICBRIDGEMODULE_6_substruct_create;
    add_bridge_driver(new simplenic_t(this, args, SIMPLENICBRIDGEMODULE_6_substruct, 6, SIMPLENICBRIDGEMODULE_6_DMA_ADDR));
    #endif
    #ifdef SIMPLENICBRIDGEMODULE_7_PRESENT
    SIMPLENICBRIDGEMODULE_7_substruct_create;
    add_bridge_driver(new simplenic_t(this, args, SIMPLENICBRIDGEMODULE_7_substruct, 7, SIMPLENICBRIDGEMODULE_7_DMA_ADDR));
    #endif
#endif

#ifdef TRACERVBRIDGEMODULE_struct_guard
    #ifdef TRACERVBRIDGEMODULE_0_PRESENT
    INSTANTIATE_TRACERV(add_bridge_driver, 0)
    #endif
    #ifdef TRACERVBRIDGEMODULE_1_PRESENT
    INSTANTIATE_TRACERV(add_bridge_driver, 1)
    #endif
    #ifdef TRACERVBRIDGEMODULE_2_PRESENT
    INSTANTIATE_TRACERV(add_bridge_driver, 2)
    #endif
    #ifdef TRACERVBRIDGEMODULE_3_PRESENT
    INSTANTIATE_TRACERV(add_bridge_driver, 3)
    #endif
    #ifdef TRACERVBRIDGEMODULE_4_PRESENT
    INSTANTIATE_TRACERV(add_bridge_driver, 4)
    #endif
    #ifdef TRACERVBRIDGEMODULE_5_PRESENT
    INSTANTIATE_TRACERV(add_bridge_driver, 5)
    #endif
    #ifdef TRACERVBRIDGEMODULE_6_PRESENT
    INSTANTIATE_TRACERV(add_bridge_driver, 6)
    #endif
    #ifdef TRACERVBRIDGEMODULE_7_PRESENT
    INSTANTIATE_TRACERV(add_bridge_driver, 7)
    #endif
    #ifdef TRACERVBRIDGEMODULE_8_PRESENT
    INSTANTIATE_TRACERV(add_bridge_driver, 8)
    #endif
    #ifdef TRACERVBRIDGEMODULE_9_PRESENT
    INSTANTIATE_TRACERV(add_bridge_driver, 9)
    #endif
    #ifdef TRACERVBRIDGEMODULE_10_PRESENT
    INSTANTIATE_TRACERV(add_bridge_driver, 10)
    #endif
    #ifdef TRACERVBRIDGEMODULE_11_PRESENT
    INSTANTIATE_TRACERV(add_bridge_driver, 11)
    #endif
    #ifdef TRACERVBRIDGEMODULE_12_PRESENT
    INSTANTIATE_TRACERV(add_bridge_driver, 12)
    #endif
    #ifdef TRACERVBRIDGEMODULE_13_PRESENT
    INSTANTIATE_TRACERV(add_bridge_driver, 13)
    #endif
    #ifdef TRACERVBRIDGEMODULE_14_PRESENT
    INSTANTIATE_TRACERV(add_bridge_driver, 14)
    #endif
    #ifdef TRACERVBRIDGEMODULE_15_PRESENT
    INSTANTIATE_TRACERV(add_bridge_driver, 15)
    #endif
#endif

#ifdef GROUNDTESTBRIDGEMODULE_struct_guard
    #ifdef GROUNDTESTBRIDGEMODULE_0_PRESENT
    GROUNDTESTBRIDGEMODULE_0_substruct_create;
    add_bridge_driver(new groundtest_t(
            this, args, GROUNDTESTBRIDGEMODULE_0_substruct));
    #endif
    #ifdef GROUNDTESTBRIDGEMODULE_1_PRESENT
    GROUNDTESTBRIDGEMODULE_1_substruct_create;
    add_bridge_driver(new groundtest_t(
            this, args, GROUNDTESTBRIDGEMODULE_1_substruct));
    #endif
    #ifdef GROUNDTESTBRIDGEMODULE_2_PRESENT
    GROUNDTESTBRIDGEMODULE_2_substruct_create;
    add_bridge_driver(new groundtest_t(
            this, args, GROUNDTESTBRIDGEMODULE_2_substruct));
    #endif
    #ifdef GROUNDTESTBRIDGEMODULE_3_PRESENT
    GROUNDTESTBRIDGEMODULE_3_substruct_create;
    add_bridge_driver(new groundtest_t(
            this, args, GROUNDTESTBRIDGEMODULE_3_substruct));
    #endif
    #ifdef GROUNDTESTBRIDGEMODULE_4_PRESENT
    GROUNDTESTBRIDGEMODULE_4_substruct_create;
    add_bridge_driver(new groundtest_t(
            this, args, GROUNDTESTBRIDGEMODULE_4_substruct));
    #endif
    #ifdef GROUNDTESTBRIDGEMODULE_5_PRESENT
    GROUNDTESTBRIDGEMODULE_5_substruct_create;
    add_bridge_driver(new groundtest_t(
            this, args, GROUNDTESTBRIDGEMODULE_5_substruct));
    #endif
    #ifdef GROUNDTESTBRIDGEMODULE_6_PRESENT
    GROUNDTESTBRIDGEMODULE_6_substruct_create;
    add_bridge_driver(new groundtest_t(
            this, args, GROUNDTESTBRIDGEMODULE_6_substruct));
    #endif
    #ifdef GROUNDTESTBRIDGEMODULE_7_PRESENT
    GROUNDTESTBRIDGEMODULE_7_substruct_create;
    add_bridge_driver(new groundtest_t(
            this, args, GROUNDTESTBRIDGEMODULE_7_substruct));
    #endif
#endif

#ifdef AUTOCOUNTERBRIDGEMODULE_0_PRESENT
    INSTANTIATE_AUTOCOUNTER(add_bridge_driver, 0)
#endif
#ifdef AUTOCOUNTERBRIDGEMODULE_1_PRESENT
    INSTANTIATE_AUTOCOUNTER(add_bridge_driver, 1)
#endif
#ifdef AUTOCOUNTERBRIDGEMODULE_2_PRESENT
    INSTANTIATE_AUTOCOUNTER(add_bridge_driver, 2)
#endif
#ifdef AUTOCOUNTERBRIDGEMODULE_3_PRESENT
    INSTANTIATE_AUTOCOUNTER(add_bridge_driver, 3)
#endif
#ifdef AUTOCOUNTERBRIDGEMODULE_4_PRESENT
    INSTANTIATE_AUTOCOUNTER(add_bridge_driver, 4)
#endif
#ifdef AUTOCOUNTERBRIDGEMODULE_5_PRESENT
    INSTANTIATE_AUTOCOUNTER(add_bridge_driver, 5)
#endif
#ifdef AUTOCOUNTERBRIDGEMODULE_6_PRESENT
    INSTANTIATE_AUTOCOUNTER(add_bridge_driver, 6)
#endif
#ifdef AUTOCOUNTERBRIDGEMODULE_7_PRESENT
    INSTANTIATE_AUTOCOUNTER(add_bridge_driver, 7)
#endif

#ifdef ASSERTBRIDGEMODULE_0_PRESENT
    ASSERTBRIDGEMODULE_0_substruct_create
    add_bridge_driver(new synthesized_assertions_t(this,
                                                   ASSERTBRIDGEMODULE_0_substruct,
                                                   ASSERTBRIDGEMODULE_0_assert_count,
                                                   ASSERTBRIDGEMODULE_0_assert_messages));
#endif
#ifdef ASSERTBRIDGEMODULE_1_PRESENT
    ASSERTBRIDGEMODULE_1_substruct_create
    add_bridge_driver(new synthesized_assertions_t(this,
                                                   ASSERTBRIDGEMODULE_1_substruct,
                                                   ASSERTBRIDGEMODULE_1_assert_count,
                                                   ASSERTBRIDGEMODULE_1_assert_messages));
#endif
#ifdef ASSERTBRIDGEMODULE_2_PRESENT
    ASSERTBRIDGEMODULE_2_substruct_create
    add_bridge_driver(new synthesized_assertions_t(this,
                                                   ASSERTBRIDGEMODULE_2_substruct,
                                                   ASSERTBRIDGEMODULE_2_assert_count,
                                                   ASSERTBRIDGEMODULE_2_assert_messages));
#endif
#ifdef ASSERTBRIDGEMODULE_3_PRESENT
    ASSERTBRIDGEMODULE_3_substruct_create
    add_bridge_driver(new synthesized_assertions_t(this,
                                                   ASSERTBRIDGEMODULE_3_substruct,
                                                   ASSERTBRIDGEMODULE_3_assert_count,
                                                   ASSERTBRIDGEMODULE_3_assert_messages));
#endif
#ifdef ASSERTBRIDGEMODULE_3_PRESENT
    ASSERTBRIDGEMODULE_3_substruct_create
    add_bridge_driver(new synthesized_assertions_t(this,
                                                   ASSERTBRIDGEMODULE_3_substruct,
                                                   ASSERTBRIDGEMODULE_3_assert_count,
                                                   ASSERTBRIDGEMODULE_3_assert_messages));
#endif
#ifdef ASSERTBRIDGEMODULE_4_PRESENT
    ASSERTBRIDGEMODULE_4_substruct_create
    add_bridge_driver(new synthesized_assertions_t(this,
                                                   ASSERTBRIDGEMODULE_4_substruct,
                                                   ASSERTBRIDGEMODULE_4_assert_count,
                                                   ASSERTBRIDGEMODULE_4_assert_messages));
#endif
#ifdef ASSERTBRIDGEMODULE_5_PRESENT
    ASSERTBRIDGEMODULE_5_substruct_create
    add_bridge_driver(new synthesized_assertions_t(this,
                                                   ASSERTBRIDGEMODULE_5_substruct,
                                                   ASSERTBRIDGEMODULE_5_assert_count,
                                                   ASSERTBRIDGEMODULE_5_assert_messages));
#endif
#ifdef ASSERTBRIDGEMODULE_6_PRESENT
    ASSERTBRIDGEMODULE_6_substruct_create
    add_bridge_driver(new synthesized_assertions_t(this,
                                                   ASSERTBRIDGEMODULE_6_substruct,
                                                   ASSERTBRIDGEMODULE_6_assert_count,
                                                   ASSERTBRIDGEMODULE_6_assert_messages));
#endif
#ifdef ASSERTBRIDGEMODULE_7_PRESENT
    ASSERTBRIDGEMODULE_7_substruct_create
    add_bridge_driver(new synthesized_assertions_t(this,
                                                   ASSERTBRIDGEMODULE_7_substruct,
                                                   ASSERTBRIDGEMODULE_7_assert_count,
                                                   ASSERTBRIDGEMODULE_7_assert_messages));
#endif

#ifdef PRINTBRIDGEMODULE_0_PRESENT
    INSTANTIATE_PRINTF(add_bridge_driver,0)
#endif
#ifdef PRINTBRIDGEMODULE_1_PRESENT
    INSTANTIATE_PRINTF(add_bridge_driver,1)
#endif
#ifdef PRINTBRIDGEMODULE_2_PRESENT
    INSTANTIATE_PRINTF(add_bridge_driver,2)
#endif
#ifdef PRINTBRIDGEMODULE_3_PRESENT
    INSTANTIATE_PRINTF(add_bridge_driver,3)
#endif
#ifdef PRINTBRIDGEMODULE_4_PRESENT
    INSTANTIATE_PRINTF(add_bridge_driver,4)
#endif
#ifdef PRINTBRIDGEMODULE_5_PRESENT
    INSTANTIATE_PRINTF(add_bridge_driver,5)
#endif
#ifdef PRINTBRIDGEMODULE_6_PRESENT
    INSTANTIATE_PRINTF(add_bridge_driver,6)
#endif
#ifdef PRINTBRIDGEMODULE_7_PRESENT
    INSTANTIATE_PRINTF(add_bridge_driver,7)
#endif
    // Add functions you'd like to periodically invoke on a paused simulator here.
    if (profile_interval != -1) {
        register_task([this](){ return this->profile_models();}, 0);
    }
}

bool firesim_top_t::simulation_complete() {
    bool is_complete = false;
    for (auto &e: bridges) {
        is_complete |= e->terminate();
    }
    return is_complete;
}

uint64_t firesim_top_t::profile_models(){
    for (auto mod: fpga_models) {
        mod->profile();
    }
    return profile_interval;
}

int firesim_top_t::exit_code(){
    for (auto &e: bridges) {
        if (e->exit_code())
            return e->exit_code();
    }
    return 0;
}


void firesim_top_t::run() {
    for (auto &e: fpga_models) {
        e->init();
    }

    for (auto &e: bridges) {
        e->init();
    }

    if (do_zero_out_dram) {
        fprintf(stderr, "Zeroing out FPGA DRAM. This will take a few minutes...\n");
        zero_out_dram();
    }
    fprintf(stderr, "Commencing simulation.\n");
    uint64_t start_hcycle = hcycle();
    uint64_t start_time = timestamp();

    // Assert reset T=0 -> 50
    target_reset(50);

    while (!simulation_complete() && !has_timed_out()) {
        run_scheduled_tasks();
        step(get_largest_stepsize(), false);
        while(!done() && !simulation_complete()){
            for (auto &e: bridges) e->tick();
        }
    }

    uint64_t end_time = timestamp();
    uint64_t end_cycle = actual_tcycle();
    uint64_t hcycles = hcycle() - start_hcycle;
    double sim_time = diff_secs(end_time, start_time);
    double sim_speed = ((double) end_cycle) / (sim_time * 1000.0);
    // always print a newline after target's output
    fprintf(stderr, "\n");
    int exitcode = exit_code();
    if (exitcode) {
        fprintf(stderr, "*** FAILED *** (code = %d) after %llu cycles\n", exitcode, end_cycle);
    } else if (!simulation_complete() && has_timed_out()) {
        fprintf(stderr, "*** FAILED *** (timeout) after %llu cycles\n", end_cycle);
    } else {
        fprintf(stderr, "*** PASSED *** after %llu cycles\n", end_cycle);
    }
    if (sim_speed > 1000.0) {
        fprintf(stderr, "time elapsed: %.1f s, simulation speed = %.2f MHz\n", sim_time, sim_speed / 1000.0);
    } else {
        fprintf(stderr, "time elapsed: %.1f s, simulation speed = %.2f KHz\n", sim_time, sim_speed);
    }
    double fmr = ((double) hcycles / end_cycle);
    // This returns the FMR of the fastest target clock
    fprintf(stderr, "FPGA-Cycles-to-Model-Cycles Ratio (FMR): %.2f\n", fmr);
    expect(!exitcode, NULL);

    for (auto &e: fpga_models) {
        e->finish();
    }

    for (auto &e: bridges) {
        e->finish();
    }
}

