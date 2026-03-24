// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for LEVEL3_REGISTER_1BIT (generated from level3_register_1bit.panda)
module level3_register_1bit_ic (
    input data,
    input clock,
    input writeenable,
    input reset,
    output q,
    output notq
);

wire aux_not_1;
wire aux_not_2;
reg aux_mux_3 = 1'b0;
reg aux_d_flip_flop_4_0_q = 1'b0;
reg aux_d_flip_flop_4_1_q = 1'b1;

// Internal logic
assign aux_not_1 = ~reset;
assign aux_not_2 = ~writeenable;
    //Multiplexer
    always @(*)
    begin
        case({aux_not_2})
            1'd0: aux_mux_3 = data;
            1'd1: aux_mux_3 = aux_d_flip_flop_4_0_q;
            default: aux_mux_3 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //D FlipFlop
    always @(posedge clock or negedge aux_not_1)
    begin
        if (~aux_not_1)
        begin
            aux_d_flip_flop_4_0_q <= 1'b0;
            aux_d_flip_flop_4_1_q <= 1'b1;
        end
        else
        begin
            aux_d_flip_flop_4_0_q <= aux_mux_3;
            aux_d_flip_flop_4_1_q <= ~aux_mux_3;
        end
    end
    //End of D FlipFlop

assign q = aux_d_flip_flop_4_0_q;
assign notq = aux_d_flip_flop_4_1_q;
endmodule

module level3_register_1bit (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,
input input_switch4,

/* ========= Outputs ========== */
output led6_1,
output led7_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL3_REGISTER_1BIT (level3_register_1bit_ic)
wire w_level3_register_1bit_ic_inst_1_q;
wire w_level3_register_1bit_ic_inst_1_notq;


// Assigning aux variables. //
level3_register_1bit_ic level3_register_1bit_ic_inst_1 (
    .data(input_switch1),
    .clock(input_switch2),
    .writeenable(input_switch3),
    .reset(input_switch4),
    .q(w_level3_register_1bit_ic_inst_1_q),
    .notq(w_level3_register_1bit_ic_inst_1_notq)
);

// Writing output data. //
assign led6_1 = w_level3_register_1bit_ic_inst_1_q;
assign led7_1 = w_level3_register_1bit_ic_inst_1_notq;
endmodule
