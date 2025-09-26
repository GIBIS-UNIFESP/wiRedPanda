// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: notes
// Generated: Fri Sep 26 00:42:05 2025
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
wire ic_jkflipflop_jkflipflop_ic_in_0_13; // IC input 0
assign ic_jkflipflop_jkflipflop_ic_in_0_13 = 1'b1;
wire ic_jkflipflop_jkflipflop_ic_in_1_14; // IC input 1
assign ic_jkflipflop_jkflipflop_ic_in_1_14 = 1'b1;
wire ic_jkflipflop_jkflipflop_ic_in_2_15; // IC input 2
assign ic_jkflipflop_jkflipflop_ic_in_2_15 = input_clock1_1;
wire ic_jkflipflop_jkflipflop_ic_in_3_16; // IC input 3
assign ic_jkflipflop_jkflipflop_ic_in_3_16 = 1'b1;
wire ic_jkflipflop_jkflipflop_ic_in_4_17; // IC input 4
assign ic_jkflipflop_jkflipflop_ic_in_4_17 = 1'b1;
wire ic_jkflipflop_jkflipflop_ic_out_0_18; // IC output 0
wire ic_jkflipflop_jkflipflop_ic_out_1_19; // IC output 1
wire ic_decoder_decoder_ic_in_0_20; // IC input 0
assign ic_decoder_decoder_ic_in_0_20 = jk_flip_flop_10_1_1;
wire ic_decoder_decoder_ic_in_1_21; // IC input 1
assign ic_decoder_decoder_ic_in_1_21 = jk_flip_flop_11_1_1;
wire ic_decoder_decoder_ic_in_2_22; // IC input 2
assign ic_decoder_decoder_ic_in_2_22 = jk_flip_flop_12_1_1;
wire ic_decoder_decoder_ic_out_0_23; // IC output 0
wire ic_decoder_decoder_ic_out_1_24; // IC output 1
wire ic_decoder_decoder_ic_out_2_25; // IC output 2
wire ic_decoder_decoder_ic_out_3_26; // IC output 3
wire ic_decoder_decoder_ic_out_4_27; // IC output 4
wire ic_decoder_decoder_ic_out_5_28; // IC output 5
wire ic_decoder_decoder_ic_out_6_29; // IC output 6
wire ic_decoder_decoder_ic_out_7_30; // IC output 7
    reg jk_flip_flop_12_0_0 = 1'b0;
    reg jk_flip_flop_12_1_1 = 1'b0;
    reg jk_flip_flop_11_0_0 = 1'b0;
    reg jk_flip_flop_11_1_1 = 1'b0;
    reg jk_flip_flop_10_0_0 = 1'b0;
    reg jk_flip_flop_10_1_1 = 1'b0;

    // ========= Logic Assignments =========
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_11_0_0) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_10_0_0 <= 1'b0; jk_flip_flop_10_1_1 <= 1'b1; end
                2'b10: begin jk_flip_flop_10_0_0 <= 1'b1; jk_flip_flop_10_1_1 <= 1'b0; end
                2'b11: begin jk_flip_flop_10_0_0 <= jk_flip_flop_10_1_1; jk_flip_flop_10_1_1 <= jk_flip_flop_10_0_0; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_12_0_0) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_11_0_0 <= 1'b0; jk_flip_flop_11_1_1 <= 1'b1; end
                2'b10: begin jk_flip_flop_11_0_0 <= 1'b1; jk_flip_flop_11_1_1 <= 1'b0; end
                2'b11: begin jk_flip_flop_11_0_0 <= jk_flip_flop_11_1_1; jk_flip_flop_11_1_1 <= jk_flip_flop_11_0_0; end // toggle
            endcase
        end
    end

    // JK FlipFlop: JK-Flip-Flop
    always @(posedge ic_jkflipflop_jkflipflop_ic_out_0_18) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_12_0_0 <= 1'b0; jk_flip_flop_12_1_1 <= 1'b1; end
                2'b10: begin jk_flip_flop_12_0_0 <= 1'b1; jk_flip_flop_12_1_1 <= 1'b0; end
                2'b11: begin jk_flip_flop_12_0_0 <= jk_flip_flop_12_1_1; jk_flip_flop_12_1_1 <= jk_flip_flop_12_0_0; end // toggle
            endcase
        end
    end


    // ========= Output Assignments =========
    assign output_buzzer1_g6_2 = ic_decoder_decoder_ic_out_4_27; // Buzzer
    assign output_buzzer2_f6_3 = ic_decoder_decoder_ic_out_3_26; // Buzzer
    assign output_buzzer3_d6_4 = ic_decoder_decoder_ic_out_1_24; // Buzzer
    assign output_buzzer4_b7_5 = ic_decoder_decoder_ic_out_6_29; // Buzzer
    assign output_buzzer5_c6_6 = ic_decoder_decoder_ic_out_0_23; // Buzzer
    assign output_buzzer6_a7_7 = ic_decoder_decoder_ic_out_5_28; // Buzzer
    assign output_buzzer7_e6_8 = ic_decoder_decoder_ic_out_2_25; // Buzzer
    assign output_buzzer8_c7_9 = ic_decoder_decoder_ic_out_7_30; // Buzzer

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
