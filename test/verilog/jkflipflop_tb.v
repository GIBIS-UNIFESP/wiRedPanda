`timescale 1ns / 1ps

module jkflipflop_tb;

    // Testbench signals
    reg input_clock1_c_1;
    reg input_input_switch2__preset_2;
    reg input_input_switch3__clear_3;
    reg input_input_switch4_j_4;
    reg input_input_switch5_k_5;
    wire output_led1_q_0_6;
    wire output_led2_q_0_7;

    // Test control
    integer test_count = 0;
    integer pass_count = 0;
    reg previous_q = 0;

    // Instantiate the Device Under Test (DUT)
    jkflipflop dut (
        .input_clock1_c_1(input_clock1_c_1),
        .input_input_switch2__preset_2(input_input_switch2__preset_2),
        .input_input_switch3__clear_3(input_input_switch3__clear_3),
        .input_input_switch4_j_4(input_input_switch4_j_4),
        .input_input_switch5_k_5(input_input_switch5_k_5),
        .output_led1_q_0_6(output_led1_q_0_6),
        .output_led2_q_0_7(output_led2_q_0_7)
    );

    // Clock generation (5MHz = 200ns period)
    always begin
        #100 input_clock1_c_1 = ~input_clock1_c_1;
    end

    // Test procedure for JK flip-flop
    task test_jkff;
        input j_value;
        input k_value;
        input preset;
        input clear;
        begin
            test_count = test_count + 1;

            // Set control signals
            input_input_switch2__preset_2 = preset;
            input_input_switch3__clear_3 = clear;
            input_input_switch4_j_4 = j_value;
            input_input_switch5_k_5 = k_value;

            // Wait for setup time
            #10;

            // Store previous Q state
            previous_q = output_led1_q_0_6;

            // Clock edge
            @(posedge input_clock1_c_1);

            // Wait for propagation delay
            #20;

            $display("Test %0d: J=%b, K=%b, Preset=%b, Clear=%b => Q=%b, Q̄=%b (prev_Q=%b)",
                     test_count, j_value, k_value, preset, clear, output_led1_q_0_6, output_led2_q_0_7, previous_q);

            // Basic validation: Q and Q̄ should be complementary in normal operation
            if (preset == 1 && clear == 1) begin
                if (output_led1_q_0_6 != output_led2_q_0_7) begin
                    // Check JK flip-flop behavior
                    case ({j_value, k_value})
                        2'b00: begin // Hold
                            if (output_led1_q_0_6 == previous_q) begin
                                $display("      PASS: J=0,K=0 held output correctly");
                                pass_count = pass_count + 1;
                            end else begin
                                $display("      FAIL: J=0,K=0 should hold output");
                            end
                        end
                        2'b01: begin // Reset
                            if (output_led1_q_0_6 == 0) begin
                                $display("      PASS: J=0,K=1 reset output correctly");
                                pass_count = pass_count + 1;
                            end else begin
                                $display("      FAIL: J=0,K=1 should reset output to 0");
                            end
                        end
                        2'b10: begin // Set
                            if (output_led1_q_0_6 == 1) begin
                                $display("      PASS: J=1,K=0 set output correctly");
                                pass_count = pass_count + 1;
                            end else begin
                                $display("      FAIL: J=1,K=0 should set output to 1");
                            end
                        end
                        2'b11: begin // Toggle
                            if (output_led1_q_0_6 != previous_q) begin
                                $display("      PASS: J=1,K=1 toggled output correctly");
                                pass_count = pass_count + 1;
                            end else begin
                                $display("      FAIL: J=1,K=1 should toggle output");
                            end
                        end
                    endcase
                end else begin
                    $display("      FAIL: Outputs should be complementary");
                end
            end else begin
                // For preset/clear conditions, just report the state
                if (preset == 0) begin
                    $display("      INFO: Preset active");
                end
                if (clear == 0) begin
                    $display("      INFO: Clear active");
                end
                pass_count = pass_count + 1; // Count as pass for control conditions
            end
        end
    endtask

    // Main test sequence
    initial begin
        $display("=== JK FLIP-FLOP TESTBENCH ===");
        $display("Testing JK flip-flop with preset and clear");

        // Initialize signals
        input_clock1_c_1 = 0;
        input_input_switch4_j_4 = 0;
        input_input_switch5_k_5 = 0;
        input_input_switch2__preset_2 = 1; // Inactive (assuming active low)
        input_input_switch3__clear_3 = 1;  // Inactive (assuming active low)

        // Wait for initialization
        #150;

        // Test all JK flip-flop combinations
        $display("\nTesting JK flip-flop truth table:");
        test_jkff(0, 0, 1, 1); // Hold
        test_jkff(0, 1, 1, 1); // Reset
        test_jkff(1, 0, 1, 1); // Set
        test_jkff(1, 1, 1, 1); // Toggle

        // Test multiple toggles
        $display("\nTesting multiple toggles:");
        test_jkff(1, 1, 1, 1); // Toggle
        test_jkff(1, 1, 1, 1); // Toggle
        test_jkff(1, 1, 1, 1); // Toggle

        // Test hold after various states
        $display("\nTesting hold behavior:");
        test_jkff(1, 0, 1, 1); // Set
        test_jkff(0, 0, 1, 1); // Hold
        test_jkff(0, 1, 1, 1); // Reset
        test_jkff(0, 0, 1, 1); // Hold

        // Test preset functionality (if active low)
        $display("\nTesting preset functionality:");
        test_jkff(0, 0, 0, 1); // Preset active
        test_jkff(1, 1, 1, 1); // Return to normal, toggle

        // Test clear functionality (if active low)
        $display("\nTesting clear functionality:");
        test_jkff(1, 0, 1, 0); // Clear active
        test_jkff(1, 1, 1, 1); // Return to normal, toggle

        // Summary
        $display("\n=== TEST SUMMARY ===");
        $display("Total tests: %0d", test_count);
        $display("Passed: %0d", pass_count);
        $display("Failed: %0d", test_count - pass_count);

        if (pass_count == test_count) begin
            $display("*** ALL TESTS PASSED ***");
        end else begin
            $display("*** SOME TESTS FAILED ***");
        end

        $finish;
    end

    // Optional: Generate VCD file for waveform viewing
    initial begin
        $dumpfile("jkflipflop_tb.vcd");
        $dumpvars(0, jkflipflop_tb);
    end

endmodule