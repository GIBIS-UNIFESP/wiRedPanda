// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: dflipflop2
// Generated: Fri Sep 26 14:59:07 2025
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
    always @(posedge node_19) begin
        begin
            d_flip_flop_22_0_q <= node_12;
            d_flip_flop_22_1_q <= ~node_12;
        end
    end

    assign nand_21 = ~(nand_20 & nand_24); // Nand
    assign nand_24 = ~(nand_21 & nand_23); // Nand
    assign nand_23 = ~(node_15 & nand_13); // Nand
    assign nand_20 = ~(nand_17 & node_16); // Nand
    assign node_19 = node_18; // Node
    assign node_18 = input_clock1_1; // Node
    assign node_16 = node_15; // Node
    assign node_15 = node_10; // Node
    assign nand_13 = ~(nand_17 & nand_11); // Nand
    assign nand_17 = ~(nand_14 & nand_13); // Nand
    assign nand_14 = ~(node_12 & not_5); // Nand
    assign node_12 = input_input_switch2_2; // Node
    assign nand_11 = ~(node_6 & not_8); // Nand
    assign node_10 = not_9; // Node
    assign not_9 = ~node_7; // Not
    assign not_8 = ~input_input_switch2_2; // Not
    assign node_7 = node_6; // Node
    assign node_6 = not_5; // Node
    assign not_5 = ~input_clock1_1; // Not

    // ========= Output Assignments =========
    assign output_led1_0_3 = d_flip_flop_22_0_q; // LED
    assign output_led2_0_4 = nand_21; // LED

endmodule // dflipflop2

// ====================================================================
// Module dflipflop2 generation completed successfully
// Elements processed: 24
// Inputs: 2, Outputs: 2
// ====================================================================
