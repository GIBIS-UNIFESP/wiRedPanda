// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


// Module for LEVEL2_MUX_2TO1 (generated from level2_mux_2to1.panda)
module level2_mux_2to1_ic (
    input data0,
    input data1,
    input sel0,
    output p_output
);

reg aux_mux_1 = 1'b0;

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

assign p_output = aux_mux_1;
endmodule

module level2_mux_2to1 (
/* ========= Inputs ========== */
input input_switch1,
input input_switch2,
input input_switch3,

/* ========= Outputs ========== */
output led5_1
);
/* ====== Aux. Variables ====== */
// IC instance: LEVEL2_MUX_2TO1 (level2_mux_2to1_ic)
wire w_level2_mux_2to1_ic_inst_1_p_output;


// Assigning aux variables. //
level2_mux_2to1_ic level2_mux_2to1_ic_inst_1 (
    .data0(input_switch1),
    .data1(input_switch2),
    .sel0(input_switch3),
    .p_output(w_level2_mux_2to1_ic_inst_1_p_output)
);

// Writing output data. //
assign led5_1 = w_level2_mux_2to1_ic_inst_1_p_output;
endmodule
