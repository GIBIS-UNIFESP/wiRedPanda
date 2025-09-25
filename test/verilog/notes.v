// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: notes
// Generated: Thu Sep 25 21:23:11 2025
// Target FPGA: Generic-Small
// Resource Usage: 16/1000 LUTs, 38/1000 FFs, 9/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module notes (
    // ========= Input Ports =========
    input wire input_clock1_1,

    // ========= Output Ports =========
    output wire output_buzzer1_g6_2,
    output wire output_buzzer2_f6_3,
    output wire output_buzzer3_d6_4,
    output wire output_buzzer4_b7_5,
    output wire output_buzzer5_c6_6,
    output wire output_buzzer6_a7_7,
    output wire output_buzzer7_e6_8,
    output wire output_buzzer8_c7_9
);

    // ========= Internal Signals =========
wire ic_jkflipflop_jkflipflop_ic_in_0_10; // IC input 0
assign ic_jkflipflop_jkflipflop_ic_in_0_10 = 1'b1;
wire ic_jkflipflop_jkflipflop_ic_in_1_11; // IC input 1
assign ic_jkflipflop_jkflipflop_ic_in_1_11 = 1'b1;
wire ic_jkflipflop_jkflipflop_ic_in_2_12; // IC input 2
assign ic_jkflipflop_jkflipflop_ic_in_2_12 = input_clock1_1;
wire ic_jkflipflop_jkflipflop_ic_in_3_13; // IC input 3
assign ic_jkflipflop_jkflipflop_ic_in_3_13 = 1'b1;
wire ic_jkflipflop_jkflipflop_ic_in_4_14; // IC input 4
assign ic_jkflipflop_jkflipflop_ic_in_4_14 = 1'b1;
wire ic_jkflipflop_jkflipflop_ic_out_0_15; // IC output 0
wire ic_jkflipflop_jkflipflop_ic_out_1_16; // IC output 1
wire ic_decoder_decoder_ic_in_0_17; // IC input 0
assign ic_decoder_decoder_ic_in_0_17 = 1'b1;
wire ic_decoder_decoder_ic_in_1_18; // IC input 1
assign ic_decoder_decoder_ic_in_1_18 = 1'b1;
wire ic_decoder_decoder_ic_in_2_19; // IC input 2
assign ic_decoder_decoder_ic_in_2_19 = 1'b1;
wire ic_decoder_decoder_ic_out_0_20; // IC output 0
wire ic_decoder_decoder_ic_out_1_21; // IC output 1
wire ic_decoder_decoder_ic_out_2_22; // IC output 2
wire ic_decoder_decoder_ic_out_3_23; // IC output 3
wire ic_decoder_decoder_ic_out_4_24; // IC output 4
wire ic_decoder_decoder_ic_out_5_25; // IC output 5
wire ic_decoder_decoder_ic_out_6_26; // IC output 6
wire ic_decoder_decoder_ic_out_7_27; // IC output 7
    reg jk_flip_flop_28_0_q = 1'b0;
    reg jk_flip_flop_28_1_q = 1'b0;
    reg jk_flip_flop_29_0_q = 1'b0;
    reg jk_flip_flop_29_1_q = 1'b0;
    reg jk_flip_flop_30_0_q = 1'b0;
    reg jk_flip_flop_30_1_q = 1'b0;

    // ========= Logic Assignments =========
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_29_0_q) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_30_0_q <= 1'b0; jk_flip_flop_30_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_30_0_q <= 1'b1; jk_flip_flop_30_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_30_0_q <= jk_flip_flop_30_1_q; jk_flip_flop_30_1_q <= jk_flip_flop_30_0_q; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_28_0_q) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_29_0_q <= 1'b0; jk_flip_flop_29_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_29_0_q <= 1'b1; jk_flip_flop_29_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_29_0_q <= jk_flip_flop_29_1_q; jk_flip_flop_29_1_q <= jk_flip_flop_29_0_q; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    always @(posedge ic_jkflipflop_jkflipflop_ic_out_0_15) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_28_0_q <= 1'b0; jk_flip_flop_28_1_q <= 1'b1; end
                2'b10: begin jk_flip_flop_28_0_q <= 1'b1; jk_flip_flop_28_1_q <= 1'b0; end
                2'b11: begin jk_flip_flop_28_0_q <= jk_flip_flop_28_1_q; jk_flip_flop_28_1_q <= jk_flip_flop_28_0_q; end // toggle
            endcase
        end
    end


    // ========= Output Assignments =========
    assign output_buzzer1_g6_2 = ic_decoder_decoder_ic_out_4_24; // Buzzer
    assign output_buzzer2_f6_3 = ic_decoder_decoder_ic_out_3_23; // Buzzer
    assign output_buzzer3_d6_4 = ic_decoder_decoder_ic_out_1_21; // Buzzer
    assign output_buzzer4_b7_5 = ic_decoder_decoder_ic_out_6_26; // Buzzer
    assign output_buzzer5_c6_6 = ic_decoder_decoder_ic_out_0_20; // Buzzer
    assign output_buzzer6_a7_7 = ic_decoder_decoder_ic_out_5_25; // Buzzer
    assign output_buzzer7_e6_8 = ic_decoder_decoder_ic_out_2_22; // Buzzer
    assign output_buzzer8_c7_9 = ic_decoder_decoder_ic_out_7_27; // Buzzer

endmodule // notes

// ====================================================================
// Module notes generation completed successfully
// Elements processed: 14
// Inputs: 1, Outputs: 8
// Warnings: 5
//   IC JKFLIPFLOP input 0 is not connected
//   IC JKFLIPFLOP input 1 is not connected
//   IC JKFLIPFLOP input 3 is not connected
//   IC JKFLIPFLOP input 4 is not connected
//   IC JKFLIPFLOP output 1 is not connected
// ====================================================================
