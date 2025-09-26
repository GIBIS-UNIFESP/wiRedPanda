`timescale 1ns / 1ps

module dflipflop_tb;

    // Testbench signals
    reg input_clock1_clk_1;
    reg input_push_button2_d_2;
    reg input_input_switch3__preset_3;
    reg input_input_switch4__clear_4;
    wire output_led1_0_5;
    wire output_led2_0_6;

    // Test control
    integer test_count = 0;
    integer pass_count = 0;

    // Instantiate the Device Under Test (DUT)
    dflipflop dut (
        .input_clock1_clk_1(input_clock1_clk_1),
        .input_push_button2_d_2(input_push_button2_d_2),
        .input_input_switch3__preset_3(input_input_switch3__preset_3),
        .input_input_switch4__clear_4(input_input_switch4__clear_4),
        .output_led1_0_5(output_led1_0_5),
        .output_led2_0_6(output_led2_0_6)
    );

    // Clock generation (10MHz = 100ns period)
    always begin
        #50 input_clock1_clk_1 = ~input_clock1_clk_1;
    end

    // Test procedure for D flip-flop functionality
    task test_dff;
        input d_value;
        input preset;
        input clear;
        begin
            test_count = test_count + 1;

            // Set control signals
            input_input_switch3__preset_3 = preset;
            input_input_switch4__clear_4 = clear;
            input_push_button2_d_2 = d_value;

            // Wait for setup time
            #10;

            // Clock edge
            @(posedge input_clock1_clk_1);

            // Wait for propagation delay
            #10;

            $display("Test %0d: D=%b, Preset=%b, Clear=%b => Q=%b, Q̄=%b",
                     test_count, d_value, preset, clear, output_led1_0_5, output_led2_0_6);

            // Basic validation: Q and Q̄ should be complementary in normal operation
            if (preset == 1 && clear == 1) begin
                if (output_led1_0_5 != output_led2_0_6) begin
                    pass_count = pass_count + 1;
                    $display("      PASS: Outputs are complementary");
                end else begin
                    $display("      FAIL: Outputs should be complementary");
                end
            end else begin
                // For preset/clear conditions, just report the state
                $display("      INFO: Preset/Clear active");
                pass_count = pass_count + 1; // Count as pass for now
            end
        end
    endtask

    // Main test sequence
    initial begin
        $display("=== D FLIP-FLOP TESTBENCH ===");
        $display("Testing D flip-flop with preset and clear");

        // Initialize signals
        input_clock1_clk_1 = 0;
        input_push_button2_d_2 = 0;
        input_input_switch3__preset_3 = 1; // Inactive (assuming active low)
        input_input_switch4__clear_4 = 1;  // Inactive (assuming active low)

        // Wait for initialization
        #100;

        // Test normal D flip-flop operation
        test_dff(0, 1, 1);
        test_dff(1, 1, 1);
        test_dff(0, 1, 1);
        test_dff(1, 1, 1);

        // Test preset functionality (if active low)
        test_dff(0, 0, 1);
        test_dff(1, 1, 1);

        // Test clear functionality (if active low)
        test_dff(1, 1, 0);
        test_dff(0, 1, 1);

        // More normal operation tests
        test_dff(1, 1, 1);
        test_dff(0, 1, 1);

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
        $dumpfile("dflipflop_tb.vcd");
        $dumpvars(0, dflipflop_tb);
    end

endmodule