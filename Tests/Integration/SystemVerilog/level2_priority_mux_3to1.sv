// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for LEVEL2_PRIORITY_MUX_3TO1 (generated from level2_priority_mux_3to1.panda)
module level2_priority_mux_3to1_ic (
    input data0,
    input sel0,
    input data1,
    input sel1,
    input data2,
    output out
);

reg aux_mux_1 = 1'b0;
reg aux_mux_2 = 1'b0;

// Internal logic
    //Multiplexer
    always @(*)
    begin
        case({sel1})
            1'd0: aux_mux_1 = data2;
            1'd1: aux_mux_1 = data1;
            default: aux_mux_1 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({sel0})
            1'd0: aux_mux_2 = aux_mux_1;
            1'd1: aux_mux_2 = data0;
            default: aux_mux_2 = 1'b0;
        endcase
    end
    //End of Multiplexer

assign out = aux_mux_2;
endmodule

module level2_priority_mux_3to1 (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,
input input_switch4,
input input_switch5,

/* ========= Outputs ========== */
output led7_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL2_PRIORITY_MUX_3TO1 (level2_priority_mux_3to1_ic)
wire w_level2_priority_mux_3to1_ic_inst_1_out;


// Assigning aux variables. //
level2_priority_mux_3to1_ic level2_priority_mux_3to1_ic_inst_1 (
    .data0(input_switch1),
    .sel0(input_switch2),
    .data1(input_switch3),
    .sel1(input_switch4),
    .data2(input_switch5),
    .out(w_level2_priority_mux_3to1_ic_inst_1_out)
);

// Writing output data. //
assign led7_1 = w_level2_priority_mux_3to1_ic_inst_1_out;
endmodule
