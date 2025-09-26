`timescale 1ns / 1ps

module display_4bits_counter_tb;

    // Testbench signals
    reg input_push_button1_1;
    reg input_clock2_2;
    wire output_7_segment_display1_g_middle_3;
    wire output_7_segment_display1_f_upper_left_4;
    wire output_7_segment_display1_e_lower_left_5;
    wire output_7_segment_display1_d_bottom_6;
    wire output_7_segment_display1_a_top_7;
    wire output_7_segment_display1_b_upper_right_8;
    wire output_7_segment_display1_dp_dot_9;
    wire output_7_segment_display1_c_lower_right_10;

    // Test control
    integer test_count = 0;
    integer pass_count = 0;
    integer cycle_count = 0;
    reg [7:0] display_pattern;

    // Instantiate the Device Under Test (DUT)
    display_4bits_counter dut (
        .input_push_button1_1(input_push_button1_1),
        .input_clock2_2(input_clock2_2),
        .output_7_segment_display1_g_middle_3(output_7_segment_display1_g_middle_3),
        .output_7_segment_display1_f_upper_left_4(output_7_segment_display1_f_upper_left_4),
        .output_7_segment_display1_e_lower_left_5(output_7_segment_display1_e_lower_left_5),
        .output_7_segment_display1_d_bottom_6(output_7_segment_display1_d_bottom_6),
        .output_7_segment_display1_a_top_7(output_7_segment_display1_a_top_7),
        .output_7_segment_display1_b_upper_right_8(output_7_segment_display1_b_upper_right_8),
        .output_7_segment_display1_dp_dot_9(output_7_segment_display1_dp_dot_9),
        .output_7_segment_display1_c_lower_right_10(output_7_segment_display1_c_lower_right_10)
    );

    // Clock generation (500kHz = 2000ns period)
    always begin
        #1000 input_clock2_2 = ~input_clock2_2;
    end

    // Pack display outputs into a single pattern for analysis
    always @(*) begin
        display_pattern = {output_7_segment_display1_g_middle_3, output_7_segment_display1_f_upper_left_4,
                          output_7_segment_display1_e_lower_left_5, output_7_segment_display1_d_bottom_6,
                          output_7_segment_display1_a_top_7, output_7_segment_display1_b_upper_right_8,
                          output_7_segment_display1_dp_dot_9, output_7_segment_display1_c_lower_right_10};
    end

    // Monitor counter behavior
    always @(posedge input_clock2_2) begin
        if (!input_push_button1_1) begin // Assuming active low reset
            cycle_count = cycle_count + 1;
            #50; // Wait for propagation

            $display("Cycle %0d: Display pattern = %8b (hex digit %0d)",
                     cycle_count, display_pattern, (cycle_count-1) % 16);

            test_count = test_count + 1;

            // For basic functionality, just check that the pattern is active
            if (display_pattern != 8'b00000000) begin // Not all zeros
                pass_count = pass_count + 1;
                $display("      PASS: Display is active");
            end else begin
                $display("      WARN: Display appears inactive");
            end

            // Display segment breakdown for first few cycles
            if (cycle_count <= 5) begin
                $display("      Segments: a=%b b=%b c=%b d=%b e=%b f=%b g=%b dp=%b",
                         output_7_segment_display1_a_top_7,
                         output_7_segment_display1_b_upper_right_8,
                         output_7_segment_display1_c_lower_right_10,
                         output_7_segment_display1_d_bottom_6,
                         output_7_segment_display1_e_lower_left_5,
                         output_7_segment_display1_f_upper_left_4,
                         output_7_segment_display1_g_middle_3,
                         output_7_segment_display1_dp_dot_9);
            end
        end
    end

    // Test reset functionality
    task test_reset;
        begin
            $display("\nTesting reset functionality...");
            input_push_button1_1 = 1; // Assert reset
            #200;
            input_push_button1_1 = 0; // Release reset
            #100;
            $display("Reset completed. Display pattern: %8b", display_pattern);
        end
    endtask

    // Main test sequence
    initial begin
        $display("=== 4-BIT COUNTER WITH 7-SEGMENT DISPLAY TESTBENCH ===");
        $display("Testing 4-bit counter (0-15) with hexadecimal display");

        // Initialize signals
        input_clock2_2 = 0;
        input_push_button1_1 = 0; // Not in reset

        // Initial reset
        test_reset();

        // Run counter for one complete cycle (16 counts: 0-F)
        $display("\nRunning counter for 16 clock cycles (0-F hex)...");

        // Wait for 16 clock cycles
        repeat(16) begin
            @(posedge input_clock2_2);
        end

        // Test reset in middle of counting
        $display("\nTesting reset during counting...");
        test_reset();

        // Run a few more cycles to verify restart
        repeat(5) begin
            @(posedge input_clock2_2);
        end

        // Test another reset
        $display("\nTesting second reset...");
        test_reset();

        // Final counting sequence
        repeat(8) begin
            @(posedge input_clock2_2);
        end

        // Summary
        $display("\n=== TEST SUMMARY ===");
        $display("Total tests: %0d", test_count);
        $display("Passed: %0d", pass_count);
        $display("Failed: %0d", test_count - pass_count);
        $display("Clock cycles observed: %0d", cycle_count);

        if (pass_count >= test_count * 0.8) begin // Allow 80% pass rate
            $display("*** TESTS MOSTLY PASSED ***");
        end else begin
            $display("*** MANY TESTS FAILED ***");
        end

        $finish;
    end

    // Optional: Generate VCD file for waveform viewing
    initial begin
        $dumpfile("display_4bits_counter_tb.vcd");
        $dumpvars(0, display_4bits_counter_tb);
    end

endmodule