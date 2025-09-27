`timescale 1ns / 1ps

module tflipflop_tb;

    // Testbench signals - Updated to match actual module interface
    reg input_push_button1_t_1;
    reg input_clock2_c_2;
    reg input_input_switch3__preset_3;
    reg input_input_switch4__clear_4;
    wire output_led1_q_0_5;
    wire output_led2_q_0_6;

    // Test control
    integer test_count = 0;
    integer pass_count = 0;

    // Instantiate the Device Under Test (DUT) - Updated port mapping
    tflipflop dut (
        .input_push_button1_t_1(input_push_button1_t_1),
        .input_clock2_c_2(input_clock2_c_2),
        .input_input_switch3__preset_3(input_input_switch3__preset_3),
        .input_input_switch4__clear_4(input_input_switch4__clear_4),
        .output_led1_q_0_5(output_led1_q_0_5),
        .output_led2_q_0_6(output_led2_q_0_6)
    );

    // Clock generation
    initial begin
        input_clock2_c_2 = 0;
        forever #50 input_clock2_c_2 = ~input_clock2_c_2; // 10 MHz clock
    end

    // Test procedure for T flip-flop with toggle, preset, and clear inputs
    task test_tff(input t_val, input preset_val, input clear_val, input [31:0] test_num);
        begin
            test_count = test_count + 1;

            // Set control inputs
            input_push_button1_t_1 = t_val;
            input_input_switch3__preset_3 = preset_val;
            input_input_switch4__clear_4 = clear_val;

            // Wait for next clock edge (if not in preset/clear)
            if (preset_val && clear_val) begin
                @(posedge input_clock2_c_2);
                #10; // Small delay for propagation
            end else begin
                #20; // Asynchronous preset/clear
            end

            $display("Test %0d: T=%b, Preset=%b, Clear=%b => Q=%b, Qbar=%b",
                     test_num, t_val, preset_val, clear_val, output_led1_q_0_5, output_led2_q_0_6);

            // Verify T flip-flop behavior
            if (!preset_val) begin
                // Preset should set Q=1
                if (output_led1_q_0_5 == 1'b1 && output_led2_q_0_6 == 1'b0) begin
                    pass_count = pass_count + 1;
                    $display("      PASS: Preset behavior correct");
                end else begin
                    $display("      FAIL: Preset should set Q=1, Qbar=0");
                end
            end else if (!clear_val) begin
                // Clear should set Q=0
                if (output_led1_q_0_5 == 1'b0 && output_led2_q_0_6 == 1'b1) begin
                    pass_count = pass_count + 1;
                    $display("      PASS: Clear behavior correct");
                end else begin
                    $display("      FAIL: Clear should set Q=0, Qbar=1");
                end
            end else begin
                // Normal T operation - just check outputs are complementary
                if (output_led1_q_0_5 != output_led2_q_0_6) begin
                    pass_count = pass_count + 1;
                    $display("      PASS: T flip-flop outputs complementary");
                end else begin
                    $display("      FAIL: Outputs should be complementary");
                end
            end
        end
    endtask

    // Main test sequence
    initial begin
        $display("=== T FLIP-FLOP TESTBENCH ===");
        $display("Testing T flip-flop with toggle, preset, and clear inputs");

        // Initialize inputs (normal operation)
        input_push_button1_t_1 = 0;
        input_input_switch3__preset_3 = 1; // Not preset
        input_input_switch4__clear_4 = 1;  // Not clear

        // Wait for initialization
        #100;

        // Test T flip-flop functionality
        $display("\nTesting T flip-flop behavior:");

        // Test preset function
        test_tff(1'b0, 1'b0, 1'b1, 1);  // Preset active

        // Test clear function
        test_tff(1'b0, 1'b1, 1'b0, 2);  // Clear active

        // Test normal T operation
        test_tff(1'b0, 1'b1, 1'b1, 3);  // T=0 (hold)
        test_tff(1'b1, 1'b1, 1'b1, 4);  // T=1 (toggle)
        test_tff(1'b1, 1'b1, 1'b1, 5);  // T=1 (toggle again)
        test_tff(1'b0, 1'b1, 1'b1, 6);  // T=0 (hold)

        // Additional monitoring
        $display("\nFinal output state:");
        $display("output_led1_q_0_5 (Q) = %b", output_led1_q_0_5);
        $display("output_led2_q_0_6 (Qbar) = %b", output_led2_q_0_6);

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

        $display("T flip-flop testbench completed");
        $finish;
    end

    initial begin
        $dumpfile("tflipflop_tb.vcd");
        $dumpvars(0, tflipflop_tb);
    end

endmodule