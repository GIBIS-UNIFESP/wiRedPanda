// ==================================================================== //
// ======= This code was generated automatically by wiRedPanda ======== //
// ==================================================================== //


module flat_level2_mux_8to1 (
/* ========= Inputs ========== */
input input_switch1_data0,
input input_switch2_data1,
input input_switch3_data2,
input input_switch4_data3,
input input_switch5_data4,
input input_switch6_data5,
input input_switch7_data6,
input input_switch8_data7,
input input_switch9_sel0,
input input_switch10_sel1,
input input_switch11_sel2,
input input_switch12_enable,

/* ========= Outputs ========== */
output led15_output_1
);
/* ====== Aux. Variables ====== */
reg aux_mux_1 = 1'b0;
wire aux_and_2;


// Assigning aux variables. //
    //Multiplexer
    always @(*)
    begin
        case({input_switch11_sel2, input_switch10_sel1, input_switch9_sel0})
            3'd0: aux_mux_1 = input_switch1_data0;
            3'd1: aux_mux_1 = input_switch2_data1;
            3'd2: aux_mux_1 = input_switch3_data2;
            3'd3: aux_mux_1 = input_switch4_data3;
            3'd4: aux_mux_1 = input_switch5_data4;
            3'd5: aux_mux_1 = input_switch6_data5;
            3'd6: aux_mux_1 = input_switch7_data6;
            3'd7: aux_mux_1 = input_switch8_data7;
            default: aux_mux_1 = 1'b0;
        endcase
    end
    //End of Multiplexer
assign aux_and_2 = (aux_mux_1 & input_switch12_enable);

// Writing output data. //
assign led15_output_1 = aux_and_2;
endmodule
