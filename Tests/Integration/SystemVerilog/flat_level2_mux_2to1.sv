// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


module flat_level2_mux_2to1 (
/* ========= Inputs ========== */
input input_switch1_data0,
input input_switch2_data1,
input input_switch3_sel0,
input input_switch4_enable,

/* ========= Outputs ========== */
output led7_output_1
);
/* ====== Aux. Variables ====== */
reg aux_mux_1 = 1'b0;
wire aux_and_2;


// Assigning aux variables. //
    //Multiplexer
    always @(*)
    begin
        case({input_switch3_sel0})
            1'd0: aux_mux_1 = input_switch1_data0;
            1'd1: aux_mux_1 = input_switch2_data1;
            default: aux_mux_1 = 1'b0;
        endcase
    end
    //End of Multiplexer
assign aux_and_2 = (aux_mux_1 & input_switch4_enable);

// Writing output data. //
assign led7_output_1 = aux_and_2;
endmodule
