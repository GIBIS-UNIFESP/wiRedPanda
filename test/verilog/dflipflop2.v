// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dflipflop2
// Generated: Fri Sep 26 00:41:58 2025
// Target FPGA: Generic-Small
// Resource Usage: 28/1000 LUTs, 36/1000 FFs, 4/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module dflipflop2 (
    // ========= Input Ports =========
    input wire input_clock1_1,
    input wire input_input_switch2_2,

    // ========= Output Ports =========
    output wire output_led1_0_3,
    output wire output_led2_0_4
);

    // ========= Internal Signals =========
    wire not_24_0;
    wire node_23_0;
    wire node_22_0;
    wire not_21_0;
    wire not_20_0;
    wire node_19_0;
    wire nand_18_0;
    wire node_17_0;
    wire nand_14_0;
    wire nand_16_0;
    wire node_13_0;
    wire node_12_0;
    wire nand_15_0;
    wire node_11_0;
    wire node_10_0;
    wire nand_9_0;
    wire nand_6_0;
    reg d_flip_flop_5_0_0 = 1'b0;
    reg d_flip_flop_5_1_1 = 1'b0;
    wire nand_8_0;
    wire nand_7_0;

    // ========= Logic Assignments =========
    // D FlipFlop: D-Flip-Flop
    always @(posedge input_clock1_1) begin
        begin
            d_flip_flop_5_0_0 <= input_input_switch2_2;
            d_flip_flop_5_1_1 <= ~input_input_switch2_2;
        end
    end

    assign nand_6_0 = ~(~(~(~(input_input_switch2_2 & ~input_clock1_1) & ~(nand_15_0 & ~(~input_clock1_1 & ~input_input_switch2_2))) & ~~input_clock1_1) & ~(nand_6_0 & ~(~~input_clock1_1 & ~(~(~(input_input_switch2_2 & ~input_clock1_1) & nand_14_0) & ~(~input_clock1_1 & ~input_input_switch2_2))))); // Nand
    assign nand_7_0 = ~(~(~(~(~(input_input_switch2_2 & ~input_clock1_1) & ~(nand_15_0 & ~(~input_clock1_1 & ~input_input_switch2_2))) & ~~input_clock1_1) & nand_7_0) & ~(~~input_clock1_1 & ~(~(~(input_input_switch2_2 & ~input_clock1_1) & nand_14_0) & ~(~input_clock1_1 & ~input_input_switch2_2)))); // Nand
    assign nand_8_0 = ~(~~input_clock1_1 & ~(~(~(input_input_switch2_2 & ~input_clock1_1) & nand_14_0) & ~(~input_clock1_1 & ~input_input_switch2_2))); // Nand
    assign nand_9_0 = ~(~(~(input_input_switch2_2 & ~input_clock1_1) & ~(nand_15_0 & ~(~input_clock1_1 & ~input_input_switch2_2))) & ~~input_clock1_1); // Nand
    assign node_10_0 = input_clock1_1; // Node
    assign node_11_0 = input_clock1_1; // Node
    assign node_12_0 = ~~input_clock1_1; // Node
    assign node_13_0 = ~~input_clock1_1; // Node
    assign nand_14_0 = ~(~(~(input_input_switch2_2 & ~input_clock1_1) & nand_14_0) & ~(~input_clock1_1 & ~input_input_switch2_2)); // Nand
    assign nand_15_0 = ~(~(input_input_switch2_2 & ~input_clock1_1) & ~(nand_15_0 & ~(~input_clock1_1 & ~input_input_switch2_2))); // Nand
    assign nand_16_0 = ~(input_input_switch2_2 & ~input_clock1_1); // Nand
    assign node_17_0 = input_input_switch2_2; // Node
    assign nand_18_0 = ~(~input_clock1_1 & ~input_input_switch2_2); // Nand
    assign node_19_0 = ~~input_clock1_1; // Node
    assign not_20_0 = ~~input_clock1_1; // Not
    assign not_21_0 = ~input_input_switch2_2; // Not
    assign node_22_0 = ~input_clock1_1; // Node
    assign node_23_0 = ~input_clock1_1; // Node
    assign not_24_0 = ~input_clock1_1; // Not

    // ========= Output Assignments =========
    assign output_led1_0_3 = d_flip_flop_5_0_0; // LED
    assign output_led2_0_4 = ~(~(~(~(input_input_switch2_2 & ~input_clock1_1) & ~(nand_15_0 & ~(~input_clock1_1 & ~input_input_switch2_2))) & ~~input_clock1_1) & ~(nand_6_0 & ~(~~input_clock1_1 & ~(~(~(input_input_switch2_2 & ~input_clock1_1) & nand_14_0) & ~(~input_clock1_1 & ~input_input_switch2_2))))); // LED

endmodule // dflipflop2

// ====================================================================
// Module dflipflop2 generation completed successfully
// Elements processed: 24
// Inputs: 2, Outputs: 2
// ====================================================================
