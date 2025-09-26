// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: counter
// Generated: Fri Sep 26 21:28:04 2025
// Target FPGA: Generic-Small
// Resource Usage: 8/1000 LUTs, 38/1000 FFs, 4/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

`timescale 1ns/1ps

module counter (
    // ========= Input Ports =========

    // ========= Output Ports =========
    output wire output_led1_0_1,
    output wire output_led2_0_2,
    output wire output_led3_0_3
);

    // ========= Internal Signals =========
    reg jk_flip_flop_4_0_q = 1'b0;
    reg jk_flip_flop_4_1_q = 1'b0;
    reg jk_flip_flop_5_0_q = 1'b0;
    reg jk_flip_flop_5_1_q = 1'b0;
    reg jk_flip_flop_6_0_q = 1'b0;
    reg jk_flip_flop_6_1_q = 1'b0;

    // ========= Logic Assignments =========
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_5_0_q) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_6_0_q <= 1'b0; jk_flip_flop_6_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_6_0_q <= 1'b1; jk_flip_flop_6_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_6_0_q <= jk_flip_flop_6_1_q; jk_flip_flop_6_1_q <= jk_flip_flop_6_0_q; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_4_0_q) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_5_0_q <= 1'b0; jk_flip_flop_5_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_5_0_q <= 1'b1; jk_flip_flop_5_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_5_0_q <= jk_flip_flop_5_1_q; jk_flip_flop_5_1_q <= jk_flip_flop_5_0_q; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    // JK FlipFlop with constant clock: JK-Flip-Flop
    initial begin // Clock always low - hold state
        jk_flip_flop_4_0_q = 1'b0;
        jk_flip_flop_4_1_q = 1'b1;
    end


    // ========= Output Assignments =========
    assign output_led1_0_1 = jk_flip_flop_6_1_q; // LED
    assign output_led2_0_2 = jk_flip_flop_5_1_q; // LED
    assign output_led3_0_3 = jk_flip_flop_4_1_q; // LED

endmodule // counter

// ====================================================================
// Module counter generation completed successfully
// Elements processed: 7
// Inputs: 0, Outputs: 3
// ====================================================================
