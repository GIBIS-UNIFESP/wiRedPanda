// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


module flat_level2_mux_4to1 (
/* ========= Inputs ========== */
input input_switch1_data0,
input input_switch2_data1,
input input_switch3_data2,
input input_switch4_data3,
input input_switch5_sel0,
input input_switch6_sel1,
input input_switch7_enable,

/* ========= Outputs ========== */
output led10_output_1
);
/* ====== Aux. Variables ====== */
reg aux_mux_1 = 1'b0;
wire aux_and_2;


// Assigning aux variables. //
    //Multiplexer
    always @(*)
    begin
        case({input_switch6_sel1, input_switch5_sel0})
            2'd0: aux_mux_1 = input_switch1_data0;
            2'd1: aux_mux_1 = input_switch2_data1;
            2'd2: aux_mux_1 = input_switch3_data2;
            2'd3: aux_mux_1 = input_switch4_data3;
            default: aux_mux_1 = 1'b0;
        endcase
    end
    //End of Multiplexer
assign aux_and_2 = (aux_mux_1 & input_switch7_enable);

// Writing output data. //
assign led10_output_1 = aux_and_2;
endmodule
