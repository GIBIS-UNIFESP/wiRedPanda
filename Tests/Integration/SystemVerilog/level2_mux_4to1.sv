// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for LEVEL2_MUX_4TO1 (generated from level2_mux_4to1.panda)
module level2_mux_4to1_ic (
    input data0,
    input data1,
    input data2,
    input data3,
    input sel0,
    input sel1,
    output p_output
);

reg aux_mux_1 = 1'b0;

// Internal logic
    //Multiplexer
    always @(*)
    begin
        case({sel1, sel0})
            2'd0: aux_mux_1 = data0;
            2'd1: aux_mux_1 = data1;
            2'd2: aux_mux_1 = data2;
            2'd3: aux_mux_1 = data3;
            default: aux_mux_1 = 1'b0;
        endcase
    end
    //End of Multiplexer

assign p_output = aux_mux_1;
endmodule

module level2_mux_4to1 (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,
input input_switch4,
input input_switch5,
input input_switch6,

/* ========= Outputs ========== */
output led8_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL2_MUX_4TO1 (level2_mux_4to1_ic)
wire w_level2_mux_4to1_ic_inst_1_p_output;


// Assigning aux variables. //
level2_mux_4to1_ic level2_mux_4to1_ic_inst_1 (
    .data0(input_switch1),
    .data1(input_switch2),
    .data2(input_switch3),
    .data3(input_switch4),
    .sel0(input_switch5),
    .sel1(input_switch6),
    .p_output(w_level2_mux_4to1_ic_inst_1_p_output)
);

// Writing output data. //
assign led8_1 = w_level2_mux_4to1_ic_inst_1_p_output;
endmodule
