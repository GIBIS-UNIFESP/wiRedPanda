// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for l1_mux1 (generated from level2_mux_2to1.panda)
module level2_mux_2to1 (
    input data0,
    input data1,
    input sel0,
    input enable,
    output p_output
);

reg aux_mux_1 = 1'b0;
wire aux_and_2;

// Internal logic
    //Multiplexer
    always @(*)
    begin
        case({sel0})
            1'd0: aux_mux_1 = data0;
            1'd1: aux_mux_1 = data1;
            default: aux_mux_1 = 1'b0;
        endcase
    end
    //End of Multiplexer
assign aux_and_2 = (aux_mux_1 & enable);

assign p_output = aux_and_2;
endmodule

// Module for LEVEL3_ALU_SELECTOR_5WAY (generated from level3_alu_selector_5way.panda)
module level3_alu_selector_5way_ic (
    input result0,
    input result1,
    input result2,
    input result3,
    input result4,
    input op0,
    input op1,
    input op2,
    output out
);

// IC instance: l1_mux1 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_2_p_output;
// IC instance: l1_mux2 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_3_p_output;
// IC instance: l2_mux1 (level2_mux_2to1)
wire w_level2_mux_2to1_inst_4_p_output;
// IC instance: l3_mux (level2_mux_2to1)
wire w_level2_mux_2to1_inst_5_p_output;

// Internal logic
level2_mux_2to1 level2_mux_2to1_inst_2 (
    .data0(result0),
    .data1(result1),
    .sel0(op0),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_2_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_3 (
    .data0(result2),
    .data1(result3),
    .sel0(op0),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_3_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_4 (
    .data0(w_level2_mux_2to1_inst_2_p_output),
    .data1(w_level2_mux_2to1_inst_3_p_output),
    .sel0(op1),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_4_p_output)
);
level2_mux_2to1 level2_mux_2to1_inst_5 (
    .data0(w_level2_mux_2to1_inst_4_p_output),
    .data1(result4),
    .sel0(op2),
    .enable(1'b1),
    .p_output(w_level2_mux_2to1_inst_5_p_output)
);

assign out = w_level2_mux_2to1_inst_5_p_output;
endmodule

module level3_alu_selector_5way (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,
input input_switch4,
input input_switch5,
input input_switch6,
input input_switch7,
input input_switch8,

/* ========= Outputs ========== */
output led10_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL3_ALU_SELECTOR_5WAY (level3_alu_selector_5way_ic)
wire w_level3_alu_selector_5way_ic_inst_1_out;


// Assigning aux variables. //
level3_alu_selector_5way_ic level3_alu_selector_5way_ic_inst_1 (
    .result0(input_switch1),
    .result1(input_switch2),
    .result2(input_switch3),
    .result3(input_switch4),
    .result4(input_switch5),
    .op0(input_switch6),
    .op1(input_switch7),
    .op2(input_switch8),
    .out(w_level3_alu_selector_5way_ic_inst_1_out)
);

// Writing output data. //
assign led10_1 = w_level3_alu_selector_5way_ic_inst_1_out;
endmodule
