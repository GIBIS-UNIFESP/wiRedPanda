// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for LEVEL2_MUX_8TO1 (generated from level2_mux_8to1.panda)
module level2_mux_8to1_ic (
    input data0,
    input data1,
    input data2,
    input data3,
    input data4,
    input data5,
    input data6,
    input data7,
    input sel0,
    input sel1,
    input sel2,
    output p_output
);

reg aux_mux_1 = 1'b0;

// Internal logic
    //Multiplexer
    always @(*)
    begin
        case({sel2, sel1, sel0})
            3'd0: aux_mux_1 = data0;
            3'd1: aux_mux_1 = data1;
            3'd2: aux_mux_1 = data2;
            3'd3: aux_mux_1 = data3;
            3'd4: aux_mux_1 = data4;
            3'd5: aux_mux_1 = data5;
            3'd6: aux_mux_1 = data6;
            3'd7: aux_mux_1 = data7;
            default: aux_mux_1 = 1'b0;
        endcase
    end
    //End of Multiplexer

assign p_output = aux_mux_1;
endmodule

module level2_mux_8to1 (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,
input input_switch4,
input input_switch5,
input input_switch6,
input input_switch7,
input input_switch8,
input input_switch9,
input input_switch10,
input input_switch11,

/* ========= Outputs ========== */
output led13_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL2_MUX_8TO1 (level2_mux_8to1_ic)
wire w_level2_mux_8to1_ic_inst_1_p_output;


// Assigning aux variables. //
level2_mux_8to1_ic level2_mux_8to1_ic_inst_1 (
    .data0(input_switch1),
    .data1(input_switch2),
    .data2(input_switch3),
    .data3(input_switch4),
    .data4(input_switch5),
    .data5(input_switch6),
    .data6(input_switch7),
    .data7(input_switch8),
    .sel0(input_switch9),
    .sel1(input_switch10),
    .sel2(input_switch11),
    .p_output(w_level2_mux_8to1_ic_inst_1_p_output)
);

// Writing output data. //
assign led13_1 = w_level2_mux_8to1_ic_inst_1_p_output;
endmodule
