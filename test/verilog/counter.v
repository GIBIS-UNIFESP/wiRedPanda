// DEBUG: Selected FPGA: Generic-Small (Small generic FPGA (educational))
// DEBUG: Estimated resources: 8 LUTs, 38 FFs, 4 IOs
// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: counter
// Generated: Fri Sep 26 03:03:37 2025
// Target FPGA: Generic-Small
// Resource Usage: 8/1000 LUTs, 38/1000 FFs, 4/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module counter (
    // ========= Input Ports =========
// DEBUG: Input port: Clock -> input_clock1_1 (Element: Clock)
    input wire input_clock1_1,

    // ========= Output Ports =========
// DEBUG: Output port: LED[0] -> output_led1_0_2 (Element: LED)
// DEBUG: Output port: LED[0] -> output_led2_0_3 (Element: LED)
// DEBUG: Output port: LED[0] -> output_led3_0_4 (Element: LED)
    output wire output_led1_0_2,
    output wire output_led2_0_3,
    output wire output_led3_0_4
);

    // ========= Internal Signals =========
    reg jk_flip_flop_5_0_0 = 1'b0;
    reg jk_flip_flop_6_1_1 = 1'b0;
    reg jk_flip_flop_7_0_0 = 1'b0;
    reg jk_flip_flop_8_1_1 = 1'b0;
    reg jk_flip_flop_9_0_0 = 1'b0;
    reg jk_flip_flop_10_1_1 = 1'b0;

    // ========= Logic Assignments =========
// DEBUG: Processing 7 top-level elements with topological sorting
// DEBUG: otherPortName: Processing port from element JK-Flip-Flop (type 18) (Element: JK-Flip-Flop)
// DEBUG: otherPortName: port has no connections, returning default value (Element: JK-Flip-Flop)
// DEBUG: otherPortName: Processing port from element JK-Flip-Flop (type 18) (Element: JK-Flip-Flop)
// DEBUG: otherPortName: Found connected port from element JK-Flip-Flop (type 18) (Element: JK-Flip-Flop)
// DEBUG: otherPortName: Checking varMap for final result (Element: JK-Flip-Flop)
// DEBUG: otherPortName: varMap lookup result: 'jk_flip_flop_7_0_0' (Element: JK-Flip-Flop)
// DEBUG: otherPortName: Final result: jk_flip_flop_7_0_0 (Element: JK-Flip-Flop)
// DEBUG: otherPortName: Processing port from element JK-Flip-Flop (type 18) (Element: JK-Flip-Flop)
// DEBUG: otherPortName: port has no connections, returning default value (Element: JK-Flip-Flop)
// DEBUG: otherPortName: Processing port from element JK-Flip-Flop (type 18) (Element: JK-Flip-Flop)
// DEBUG: otherPortName: port has no connections, returning default value (Element: JK-Flip-Flop)
// DEBUG: otherPortName: Processing port from element JK-Flip-Flop (type 18) (Element: JK-Flip-Flop)
// DEBUG: otherPortName: port has no connections, returning default value (Element: JK-Flip-Flop)
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_7_0_0) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_9_0_0 <= 1'b0; jk_flip_flop_10_1_1 <= 1'b1; end
                2'b10: begin jk_flip_flop_9_0_0 <= 1'b1; jk_flip_flop_10_1_1 <= 1'b0; end
                2'b11: begin jk_flip_flop_9_0_0 <= jk_flip_flop_10_1_1; jk_flip_flop_10_1_1 <= jk_flip_flop_9_0_0; end // toggle
            endcase
        end
    end

// DEBUG: otherPortName: Processing port from element JK-Flip-Flop (type 18) (Element: JK-Flip-Flop)
// DEBUG: otherPortName: port has no connections, returning default value (Element: JK-Flip-Flop)
// DEBUG: otherPortName: Processing port from element JK-Flip-Flop (type 18) (Element: JK-Flip-Flop)
// DEBUG: otherPortName: Found connected port from element JK-Flip-Flop (type 18) (Element: JK-Flip-Flop)
// DEBUG: otherPortName: Checking varMap for final result (Element: JK-Flip-Flop)
// DEBUG: otherPortName: varMap lookup result: 'jk_flip_flop_5_0_0' (Element: JK-Flip-Flop)
// DEBUG: otherPortName: Final result: jk_flip_flop_5_0_0 (Element: JK-Flip-Flop)
// DEBUG: otherPortName: Processing port from element JK-Flip-Flop (type 18) (Element: JK-Flip-Flop)
// DEBUG: otherPortName: port has no connections, returning default value (Element: JK-Flip-Flop)
// DEBUG: otherPortName: Processing port from element JK-Flip-Flop (type 18) (Element: JK-Flip-Flop)
// DEBUG: otherPortName: port has no connections, returning default value (Element: JK-Flip-Flop)
// DEBUG: otherPortName: Processing port from element JK-Flip-Flop (type 18) (Element: JK-Flip-Flop)
// DEBUG: otherPortName: port has no connections, returning default value (Element: JK-Flip-Flop)
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge jk_flip_flop_5_0_0) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_7_0_0 <= 1'b0; jk_flip_flop_8_1_1 <= 1'b1; end
                2'b10: begin jk_flip_flop_7_0_0 <= 1'b1; jk_flip_flop_8_1_1 <= 1'b0; end
                2'b11: begin jk_flip_flop_7_0_0 <= jk_flip_flop_8_1_1; jk_flip_flop_8_1_1 <= jk_flip_flop_7_0_0; end // toggle
            endcase
        end
    end

// DEBUG: otherPortName: Processing port from element JK-Flip-Flop (type 18) (Element: JK-Flip-Flop)
// DEBUG: otherPortName: port has no connections, returning default value (Element: JK-Flip-Flop)
// DEBUG: otherPortName: Processing port from element JK-Flip-Flop (type 18) (Element: JK-Flip-Flop)
// DEBUG: otherPortName: Found connected port from element Clock (type 9) (Element: Clock)
// DEBUG: otherPortName: Checking varMap for final result (Element: Clock)
// DEBUG: otherPortName: varMap lookup result: 'input_clock1_1' (Element: Clock)
// DEBUG: otherPortName: Final result: input_clock1_1 (Element: Clock)
// DEBUG: otherPortName: Processing port from element JK-Flip-Flop (type 18) (Element: JK-Flip-Flop)
// DEBUG: otherPortName: port has no connections, returning default value (Element: JK-Flip-Flop)
// DEBUG: otherPortName: Processing port from element JK-Flip-Flop (type 18) (Element: JK-Flip-Flop)
// DEBUG: otherPortName: port has no connections, returning default value (Element: JK-Flip-Flop)
// DEBUG: otherPortName: Processing port from element JK-Flip-Flop (type 18) (Element: JK-Flip-Flop)
// DEBUG: otherPortName: port has no connections, returning default value (Element: JK-Flip-Flop)
    // JK FlipFlop: JK-Flip-Flop
    always @(posedge input_clock1_1) begin
        begin
            case ({1'b1, 1'b1})
                2'b00: begin /* hold */ end
                2'b01: begin jk_flip_flop_5_0_0 <= 1'b0; jk_flip_flop_6_1_1 <= 1'b1; end
                2'b10: begin jk_flip_flop_5_0_0 <= 1'b1; jk_flip_flop_6_1_1 <= 1'b0; end
                2'b11: begin jk_flip_flop_5_0_0 <= jk_flip_flop_6_1_1; jk_flip_flop_6_1_1 <= jk_flip_flop_5_0_0; end // toggle
            endcase
        end
    end


    // ========= Output Assignments =========
// DEBUG: otherPortName: Processing port from element LED (type 3) (Element: LED)
// DEBUG: otherPortName: Found connected port from element JK-Flip-Flop (type 18) (Element: JK-Flip-Flop)
// DEBUG: otherPortName: Checking varMap for final result (Element: JK-Flip-Flop)
// DEBUG: otherPortName: varMap lookup result: 'jk_flip_flop_10_1_1' (Element: JK-Flip-Flop)
// DEBUG: otherPortName: Final result: jk_flip_flop_10_1_1 (Element: JK-Flip-Flop)
    assign output_led1_0_2 = jk_flip_flop_10_1_1; // LED
// DEBUG: otherPortName: Processing port from element LED (type 3) (Element: LED)
// DEBUG: otherPortName: Found connected port from element JK-Flip-Flop (type 18) (Element: JK-Flip-Flop)
// DEBUG: otherPortName: Checking varMap for final result (Element: JK-Flip-Flop)
// DEBUG: otherPortName: varMap lookup result: 'jk_flip_flop_8_1_1' (Element: JK-Flip-Flop)
// DEBUG: otherPortName: Final result: jk_flip_flop_8_1_1 (Element: JK-Flip-Flop)
    assign output_led2_0_3 = jk_flip_flop_8_1_1; // LED
// DEBUG: otherPortName: Processing port from element LED (type 3) (Element: LED)
// DEBUG: otherPortName: Found connected port from element JK-Flip-Flop (type 18) (Element: JK-Flip-Flop)
// DEBUG: otherPortName: Checking varMap for final result (Element: JK-Flip-Flop)
// DEBUG: otherPortName: varMap lookup result: 'jk_flip_flop_6_1_1' (Element: JK-Flip-Flop)
// DEBUG: otherPortName: Final result: jk_flip_flop_6_1_1 (Element: JK-Flip-Flop)
    assign output_led3_0_4 = jk_flip_flop_6_1_1; // LED

endmodule // counter

// ====================================================================
// Module counter generation completed successfully
// Elements processed: 7
// Inputs: 1, Outputs: 3
// ====================================================================
