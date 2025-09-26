// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dflipflop2
// Generated: Fri Sep 26 14:36:20 2025
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
    wire not_5;
    wire node_6;
    wire node_7;
    wire not_8;
    wire not_9;
    wire node_10;
    wire nand_11;
    wire node_12;
    wire nand_13;
    wire nand_14;
    wire node_15;
    wire node_16;
    wire nand_17;
    wire node_18;
    wire node_19;
    wire nand_20;
    wire nand_21;
    reg d_flip_flop_22_0_q = 1'b0;
    reg d_flip_flop_22_1_q = 1'b0;
    wire nand_23;
    wire nand_24;

    // ========= Logic Assignments =========
    // D FlipFlop: D-Flip-Flop
    always @(posedge input_clock1_1) begin
        begin
            d_flip_flop_22_0_q <= input_input_switch2_2;
            d_flip_flop_22_1_q <= ~input_input_switch2_2;
        end
    end

    /* verilator lint_off UNOPTFLAT */
    assign nand_21 = ((input_input_switch2_2 & ~input_clock1_1) & ~(nand_17 & input_clock1_1 & ~input_input_switch2_2) & input_clock1_1) & ~(nand_21 & ~(input_clock1_1 & ~((input_input_switch2_2 & ~input_clock1_1) & nand_13 & input_clock1_1 & ~input_input_switch2_2))); // Nand
    /* verilator lint_on UNOPTFLAT */
    /* verilator lint_off UNOPTFLAT */
    assign nand_24 = ~(((input_input_switch2_2 & ~input_clock1_1) & ~(nand_17 & input_clock1_1 & ~input_input_switch2_2) & input_clock1_1) & nand_24 & ~(input_clock1_1 & ~((input_input_switch2_2 & ~input_clock1_1) & nand_13 & input_clock1_1 & ~input_input_switch2_2))); // Nand
    /* verilator lint_on UNOPTFLAT */
    assign nand_23 = ~(input_clock1_1 & ~((input_input_switch2_2 & ~input_clock1_1) & nand_13 & input_clock1_1 & ~input_input_switch2_2)); // Nand
    assign nand_20 = ~((input_input_switch2_2 & ~input_clock1_1) & ~(nand_17 & input_clock1_1 & ~input_input_switch2_2) & input_clock1_1); // Nand
    assign node_19 = input_clock1_1; // Node
    assign node_18 = input_clock1_1; // Node
    assign node_16 = input_clock1_1; // Node
    assign node_15 = input_clock1_1; // Node
    /* verilator lint_off UNOPTFLAT */
    assign nand_13 = ~((input_input_switch2_2 & ~input_clock1_1) & nand_13 & input_clock1_1 & ~input_input_switch2_2); // Nand
    /* verilator lint_on UNOPTFLAT */
    /* verilator lint_off UNOPTFLAT */
    assign nand_17 = (input_input_switch2_2 & ~input_clock1_1) & ~(nand_17 & input_clock1_1 & ~input_input_switch2_2); // Nand
    /* verilator lint_on UNOPTFLAT */
    assign nand_14 = ~(input_input_switch2_2 & ~input_clock1_1); // Nand
    assign node_12 = input_input_switch2_2; // Node
    assign nand_11 = input_clock1_1 & ~input_input_switch2_2; // Nand
    assign node_10 = input_clock1_1; // Node
    assign not_9 = input_clock1_1; // Not
    assign not_8 = ~input_input_switch2_2; // Not
    assign node_7 = ~input_clock1_1; // Node
    assign node_6 = ~input_clock1_1; // Node
    assign not_5 = ~input_clock1_1; // Not

    // ========= Output Assignments =========
    assign output_led1_0_3 = d_flip_flop_22_0_q; // LED
    assign output_led2_0_4 = ((input_input_switch2_2 & ~input_clock1_1) & ~(nand_17 & input_clock1_1 & ~input_input_switch2_2) & input_clock1_1) & ~(nand_21 & ~(input_clock1_1 & ~((input_input_switch2_2 & ~input_clock1_1) & nand_13 & input_clock1_1 & ~input_input_switch2_2))); // LED

endmodule // dflipflop2

// ====================================================================
// Module dflipflop2 generation completed successfully
// Elements processed: 24
// Inputs: 2, Outputs: 2
// ====================================================================
