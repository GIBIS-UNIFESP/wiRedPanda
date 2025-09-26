// DEBUG: Selected FPGA: Generic-Small (Small generic FPGA (educational))
// DEBUG: Estimated resources: 16 LUTs, 74 FFs, 6 IOs
// ====================================================================
// ======= This Verilog code was generated automatically by wiRedPanda =======
// ====================================================================
//
// Module: register
// Generated: Fri Sep 26 03:03:47 2025
// Target FPGA: Generic-Small
// Resource Usage: 16/1000 LUTs, 74/1000 FFs, 6/50 IOs
//
// wiRedPanda - Digital Logic Circuit Simulator
// https://github.com/gibis-unifesp/wiredpanda
// ====================================================================

module register (
    // ========= Input Ports =========
// DEBUG: Input port: Clock -> input_clock1_1 (Element: Clock)
// DEBUG: Input port: Clock -> input_clock2_2 (Element: Clock)
    input wire input_clock1_1,
    input wire input_clock2_2,

    // ========= Output Ports =========
// DEBUG: Output port: LED[0] -> output_led1_0_3 (Element: LED)
// DEBUG: Output port: LED[0] -> output_led2_0_4 (Element: LED)
// DEBUG: Output port: LED[0] -> output_led3_0_5 (Element: LED)
// DEBUG: Output port: LED[0] -> output_led4_0_6 (Element: LED)
    output wire output_led1_0_3,
    output wire output_led2_0_4,
    output wire output_led3_0_5,
    output wire output_led4_0_6
);

    // ========= Internal Signals =========
    wire node_7_0;
    wire node_8_0;
    reg d_flip_flop_9_0_0 = 1'b0;
    reg d_flip_flop_10_1_1 = 1'b0;
    wire node_11_0;
    reg d_flip_flop_12_0_0 = 1'b0;
    reg d_flip_flop_13_1_1 = 1'b0;
    reg d_flip_flop_14_0_0 = 1'b0;
    reg d_flip_flop_15_1_1 = 1'b0;
    wire node_16_0;
    reg d_flip_flop_17_0_0 = 1'b0;
    reg d_flip_flop_18_1_1 = 1'b0;

    // ========= Logic Assignments =========
// DEBUG: Processing 14 top-level elements with topological sorting
// DEBUG: generateSequentialLogic: Processing DFlipFlop D-Flip-Flop (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: firstOut=d_flip_flop_17_0_0, secondOut=d_flip_flop_18_1_1 (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: Getting data signal from input port 0 (Element: D-Flip-Flop)
// DEBUG: otherPortName: Processing port from element D-Flip-Flop (type 17) (Element: D-Flip-Flop)
// DEBUG: otherPortName: Found connected port from element D-Flip-Flop (type 17) (Element: D-Flip-Flop)
// DEBUG: otherPortName: Checking varMap for final result (Element: D-Flip-Flop)
// DEBUG: otherPortName: varMap lookup result: 'd_flip_flop_14_0_0' (Element: D-Flip-Flop)
// DEBUG: otherPortName: Final result: d_flip_flop_14_0_0 (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: data signal = d_flip_flop_14_0_0 (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: Getting clock signal from input port 1 (Element: D-Flip-Flop)
// DEBUG: otherPortName: Processing port from element D-Flip-Flop (type 17) (Element: D-Flip-Flop)
// DEBUG: otherPortName: Found connected port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Node)
// DEBUG: otherPortName: Processing port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Found connected port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Node)
// DEBUG: otherPortName: Processing port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Found connected port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Node)
// DEBUG: otherPortName: Processing port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Found connected port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Node)
// DEBUG: otherPortName: Processing port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Found connected port from element Clock (type 9) (Element: Clock)
// DEBUG: otherPortName: Checking varMap for final result (Element: Clock)
// DEBUG: otherPortName: varMap lookup result: 'input_clock1_1' (Element: Clock)
// DEBUG: otherPortName: Final result: input_clock1_1 (Element: Clock)
// DEBUG: DFlipFlop D-Flip-Flop: CLOCK signal = input_clock1_1 (THIS IS CRITICAL!) (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: Getting preset signal from input port 2 (Element: D-Flip-Flop)
// DEBUG: otherPortName: Processing port from element D-Flip-Flop (type 17) (Element: D-Flip-Flop)
// DEBUG: otherPortName: port has no connections, returning default value (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: preset signal = 1'b1 (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: Getting clear signal from input port 3 (Element: D-Flip-Flop)
// DEBUG: otherPortName: Processing port from element D-Flip-Flop (type 17) (Element: D-Flip-Flop)
// DEBUG: otherPortName: port has no connections, returning default value (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: clear signal = 1'b1 (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: Building sensitivity list with clock=input_clock1_1 (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: Added 'posedge input_clock1_1' to sensitivity list (Element: D-Flip-Flop)
    // D FlipFlop: D-Flip-Flop
    always @(posedge input_clock1_1) begin
        begin
            d_flip_flop_17_0_0 <= d_flip_flop_14_0_0;
            d_flip_flop_18_1_1 <= ~d_flip_flop_14_0_0;
        end
    end

// DEBUG: otherPortName: Processing port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Found connected port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Node)
// DEBUG: otherPortName: Processing port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Found connected port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Node)
// DEBUG: otherPortName: Processing port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Found connected port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Node)
// DEBUG: otherPortName: Processing port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Found connected port from element Clock (type 9) (Element: Clock)
// DEBUG: otherPortName: Checking varMap for final result (Element: Clock)
// DEBUG: otherPortName: varMap lookup result: 'input_clock1_1' (Element: Clock)
// DEBUG: otherPortName: Final result: input_clock1_1 (Element: Clock)
    assign node_16_0 = input_clock1_1; // Node
// DEBUG: generateSequentialLogic: Processing DFlipFlop D-Flip-Flop (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: firstOut=d_flip_flop_14_0_0, secondOut=d_flip_flop_15_1_1 (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: Getting data signal from input port 0 (Element: D-Flip-Flop)
// DEBUG: otherPortName: Processing port from element D-Flip-Flop (type 17) (Element: D-Flip-Flop)
// DEBUG: otherPortName: Found connected port from element D-Flip-Flop (type 17) (Element: D-Flip-Flop)
// DEBUG: otherPortName: Checking varMap for final result (Element: D-Flip-Flop)
// DEBUG: otherPortName: varMap lookup result: 'd_flip_flop_12_0_0' (Element: D-Flip-Flop)
// DEBUG: otherPortName: Final result: d_flip_flop_12_0_0 (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: data signal = d_flip_flop_12_0_0 (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: Getting clock signal from input port 1 (Element: D-Flip-Flop)
// DEBUG: otherPortName: Processing port from element D-Flip-Flop (type 17) (Element: D-Flip-Flop)
// DEBUG: otherPortName: Found connected port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Node)
// DEBUG: otherPortName: Processing port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Found connected port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Node)
// DEBUG: otherPortName: Processing port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Found connected port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Node)
// DEBUG: otherPortName: Processing port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Found connected port from element Clock (type 9) (Element: Clock)
// DEBUG: otherPortName: Checking varMap for final result (Element: Clock)
// DEBUG: otherPortName: varMap lookup result: 'input_clock1_1' (Element: Clock)
// DEBUG: otherPortName: Final result: input_clock1_1 (Element: Clock)
// DEBUG: DFlipFlop D-Flip-Flop: CLOCK signal = input_clock1_1 (THIS IS CRITICAL!) (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: Getting preset signal from input port 2 (Element: D-Flip-Flop)
// DEBUG: otherPortName: Processing port from element D-Flip-Flop (type 17) (Element: D-Flip-Flop)
// DEBUG: otherPortName: port has no connections, returning default value (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: preset signal = 1'b1 (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: Getting clear signal from input port 3 (Element: D-Flip-Flop)
// DEBUG: otherPortName: Processing port from element D-Flip-Flop (type 17) (Element: D-Flip-Flop)
// DEBUG: otherPortName: port has no connections, returning default value (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: clear signal = 1'b1 (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: Building sensitivity list with clock=input_clock1_1 (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: Added 'posedge input_clock1_1' to sensitivity list (Element: D-Flip-Flop)
    // D FlipFlop: D-Flip-Flop
    always @(posedge input_clock1_1) begin
        begin
            d_flip_flop_14_0_0 <= d_flip_flop_12_0_0;
            d_flip_flop_15_1_1 <= ~d_flip_flop_12_0_0;
        end
    end

// DEBUG: generateSequentialLogic: Processing DFlipFlop D-Flip-Flop (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: firstOut=d_flip_flop_12_0_0, secondOut=d_flip_flop_13_1_1 (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: Getting data signal from input port 0 (Element: D-Flip-Flop)
// DEBUG: otherPortName: Processing port from element D-Flip-Flop (type 17) (Element: D-Flip-Flop)
// DEBUG: otherPortName: Found connected port from element D-Flip-Flop (type 17) (Element: D-Flip-Flop)
// DEBUG: otherPortName: Checking varMap for final result (Element: D-Flip-Flop)
// DEBUG: otherPortName: varMap lookup result: 'd_flip_flop_9_0_0' (Element: D-Flip-Flop)
// DEBUG: otherPortName: Final result: d_flip_flop_9_0_0 (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: data signal = d_flip_flop_9_0_0 (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: Getting clock signal from input port 1 (Element: D-Flip-Flop)
// DEBUG: otherPortName: Processing port from element D-Flip-Flop (type 17) (Element: D-Flip-Flop)
// DEBUG: otherPortName: Found connected port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Node)
// DEBUG: otherPortName: Processing port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Found connected port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Node)
// DEBUG: otherPortName: Processing port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Found connected port from element Clock (type 9) (Element: Clock)
// DEBUG: otherPortName: Checking varMap for final result (Element: Clock)
// DEBUG: otherPortName: varMap lookup result: 'input_clock1_1' (Element: Clock)
// DEBUG: otherPortName: Final result: input_clock1_1 (Element: Clock)
// DEBUG: DFlipFlop D-Flip-Flop: CLOCK signal = input_clock1_1 (THIS IS CRITICAL!) (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: Getting preset signal from input port 2 (Element: D-Flip-Flop)
// DEBUG: otherPortName: Processing port from element D-Flip-Flop (type 17) (Element: D-Flip-Flop)
// DEBUG: otherPortName: port has no connections, returning default value (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: preset signal = 1'b1 (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: Getting clear signal from input port 3 (Element: D-Flip-Flop)
// DEBUG: otherPortName: Processing port from element D-Flip-Flop (type 17) (Element: D-Flip-Flop)
// DEBUG: otherPortName: port has no connections, returning default value (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: clear signal = 1'b1 (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: Building sensitivity list with clock=input_clock1_1 (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: Added 'posedge input_clock1_1' to sensitivity list (Element: D-Flip-Flop)
    // D FlipFlop: D-Flip-Flop
    always @(posedge input_clock1_1) begin
        begin
            d_flip_flop_12_0_0 <= d_flip_flop_9_0_0;
            d_flip_flop_13_1_1 <= ~d_flip_flop_9_0_0;
        end
    end

// DEBUG: otherPortName: Processing port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Found connected port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Node)
// DEBUG: otherPortName: Processing port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Found connected port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Node)
// DEBUG: otherPortName: Processing port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Found connected port from element Clock (type 9) (Element: Clock)
// DEBUG: otherPortName: Checking varMap for final result (Element: Clock)
// DEBUG: otherPortName: varMap lookup result: 'input_clock1_1' (Element: Clock)
// DEBUG: otherPortName: Final result: input_clock1_1 (Element: Clock)
    assign node_11_0 = input_clock1_1; // Node
// DEBUG: generateSequentialLogic: Processing DFlipFlop D-Flip-Flop (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: firstOut=d_flip_flop_9_0_0, secondOut=d_flip_flop_10_1_1 (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: Getting data signal from input port 0 (Element: D-Flip-Flop)
// DEBUG: otherPortName: Processing port from element D-Flip-Flop (type 17) (Element: D-Flip-Flop)
// DEBUG: otherPortName: Found connected port from element Clock (type 9) (Element: Clock)
// DEBUG: otherPortName: Checking varMap for final result (Element: Clock)
// DEBUG: otherPortName: varMap lookup result: 'input_clock2_2' (Element: Clock)
// DEBUG: otherPortName: Final result: input_clock2_2 (Element: Clock)
// DEBUG: DFlipFlop D-Flip-Flop: data signal = input_clock2_2 (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: Getting clock signal from input port 1 (Element: D-Flip-Flop)
// DEBUG: otherPortName: Processing port from element D-Flip-Flop (type 17) (Element: D-Flip-Flop)
// DEBUG: otherPortName: Found connected port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Node)
// DEBUG: otherPortName: Processing port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Found connected port from element Clock (type 9) (Element: Clock)
// DEBUG: otherPortName: Checking varMap for final result (Element: Clock)
// DEBUG: otherPortName: varMap lookup result: 'input_clock1_1' (Element: Clock)
// DEBUG: otherPortName: Final result: input_clock1_1 (Element: Clock)
// DEBUG: DFlipFlop D-Flip-Flop: CLOCK signal = input_clock1_1 (THIS IS CRITICAL!) (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: Getting preset signal from input port 2 (Element: D-Flip-Flop)
// DEBUG: otherPortName: Processing port from element D-Flip-Flop (type 17) (Element: D-Flip-Flop)
// DEBUG: otherPortName: port has no connections, returning default value (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: preset signal = 1'b1 (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: Getting clear signal from input port 3 (Element: D-Flip-Flop)
// DEBUG: otherPortName: Processing port from element D-Flip-Flop (type 17) (Element: D-Flip-Flop)
// DEBUG: otherPortName: port has no connections, returning default value (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: clear signal = 1'b1 (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: Building sensitivity list with clock=input_clock1_1 (Element: D-Flip-Flop)
// DEBUG: DFlipFlop D-Flip-Flop: Added 'posedge input_clock1_1' to sensitivity list (Element: D-Flip-Flop)
    // D FlipFlop: D-Flip-Flop
    always @(posedge input_clock1_1) begin
        begin
            d_flip_flop_9_0_0 <= input_clock2_2;
            d_flip_flop_10_1_1 <= ~input_clock2_2;
        end
    end

// DEBUG: otherPortName: Processing port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Found connected port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Connected to logic gate, generating expression (Element: Node)
// DEBUG: otherPortName: Processing port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Found connected port from element Clock (type 9) (Element: Clock)
// DEBUG: otherPortName: Checking varMap for final result (Element: Clock)
// DEBUG: otherPortName: varMap lookup result: 'input_clock1_1' (Element: Clock)
// DEBUG: otherPortName: Final result: input_clock1_1 (Element: Clock)
    assign node_8_0 = input_clock1_1; // Node
// DEBUG: otherPortName: Processing port from element Node (type 23) (Element: Node)
// DEBUG: otherPortName: Found connected port from element Clock (type 9) (Element: Clock)
// DEBUG: otherPortName: Checking varMap for final result (Element: Clock)
// DEBUG: otherPortName: varMap lookup result: 'input_clock1_1' (Element: Clock)
// DEBUG: otherPortName: Final result: input_clock1_1 (Element: Clock)
    assign node_7_0 = input_clock1_1; // Node

    // ========= Output Assignments =========
// DEBUG: otherPortName: Processing port from element LED (type 3) (Element: LED)
// DEBUG: otherPortName: Found connected port from element D-Flip-Flop (type 17) (Element: D-Flip-Flop)
// DEBUG: otherPortName: Checking varMap for final result (Element: D-Flip-Flop)
// DEBUG: otherPortName: varMap lookup result: 'd_flip_flop_9_0_0' (Element: D-Flip-Flop)
// DEBUG: otherPortName: Final result: d_flip_flop_9_0_0 (Element: D-Flip-Flop)
    assign output_led1_0_3 = d_flip_flop_9_0_0; // LED
// DEBUG: otherPortName: Processing port from element LED (type 3) (Element: LED)
// DEBUG: otherPortName: Found connected port from element D-Flip-Flop (type 17) (Element: D-Flip-Flop)
// DEBUG: otherPortName: Checking varMap for final result (Element: D-Flip-Flop)
// DEBUG: otherPortName: varMap lookup result: 'd_flip_flop_14_0_0' (Element: D-Flip-Flop)
// DEBUG: otherPortName: Final result: d_flip_flop_14_0_0 (Element: D-Flip-Flop)
    assign output_led2_0_4 = d_flip_flop_14_0_0; // LED
// DEBUG: otherPortName: Processing port from element LED (type 3) (Element: LED)
// DEBUG: otherPortName: Found connected port from element D-Flip-Flop (type 17) (Element: D-Flip-Flop)
// DEBUG: otherPortName: Checking varMap for final result (Element: D-Flip-Flop)
// DEBUG: otherPortName: varMap lookup result: 'd_flip_flop_17_0_0' (Element: D-Flip-Flop)
// DEBUG: otherPortName: Final result: d_flip_flop_17_0_0 (Element: D-Flip-Flop)
    assign output_led3_0_5 = d_flip_flop_17_0_0; // LED
// DEBUG: otherPortName: Processing port from element LED (type 3) (Element: LED)
// DEBUG: otherPortName: Found connected port from element D-Flip-Flop (type 17) (Element: D-Flip-Flop)
// DEBUG: otherPortName: Checking varMap for final result (Element: D-Flip-Flop)
// DEBUG: otherPortName: varMap lookup result: 'd_flip_flop_12_0_0' (Element: D-Flip-Flop)
// DEBUG: otherPortName: Final result: d_flip_flop_12_0_0 (Element: D-Flip-Flop)
    assign output_led4_0_6 = d_flip_flop_12_0_0; // LED

endmodule // register

// ====================================================================
// Module register generation completed successfully
// Elements processed: 14
// Inputs: 2, Outputs: 4
// ====================================================================
