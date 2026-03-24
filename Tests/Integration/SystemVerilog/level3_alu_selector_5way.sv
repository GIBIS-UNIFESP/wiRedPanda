// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


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

reg aux_mux_1 = 1'b0;
reg aux_mux_2 = 1'b0;
reg aux_mux_3 = 1'b0;
reg aux_mux_4 = 1'b0;

// Internal logic
    //Multiplexer
    always @(*)
    begin
        case({op0})
            1'd0: aux_mux_1 = result0;
            1'd1: aux_mux_1 = result1;
            default: aux_mux_1 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({op0})
            1'd0: aux_mux_2 = result2;
            1'd1: aux_mux_2 = result3;
            default: aux_mux_2 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({op1})
            1'd0: aux_mux_3 = aux_mux_1;
            1'd1: aux_mux_3 = aux_mux_2;
            default: aux_mux_3 = 1'b0;
        endcase
    end
    //End of Multiplexer
    //Multiplexer
    always @(*)
    begin
        case({op2})
            1'd0: aux_mux_4 = aux_mux_3;
            1'd1: aux_mux_4 = result4;
            default: aux_mux_4 = 1'b0;
        endcase
    end
    //End of Multiplexer

assign out = aux_mux_4;
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
