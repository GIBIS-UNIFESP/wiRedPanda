`timescale 1ns / 1ps

module sequential_tb;

    // Testbench signals
    reg input_push_button1_reset_1;
    reg input_clock2_slow_clk_2;
    reg input_clock3_fast_clk_3;
    wire output_led1_load_shift_0_4;
    wire output_led2_l1_0_5;
    wire output_led3_l3_0_6;
    wire output_led4_l2_0_7;
    wire output_led5_l0_0_8;

    // Test control
    integer test_count = 0;
    integer pass_count = 0;
    integer cycle_count = 0;
    reg [4:0] output_state;
    reg [4:0] previous_state;

    // Instantiate the Device Under Test (DUT)
    sequential dut (
        .input_push_button1_reset_1(input_push_button1_reset_1),
        .input_clock2_slow_clk_2(input_clock2_slow_clk_2),
        .input_clock3_fast_clk_3(input_clock3_fast_clk_3),
        .output_led1_load_shift_0_4(output_led1_load_shift_0_4),
        .output_led2_l1_0_5(output_led2_l1_0_5),
        .output_led3_l3_0_6(output_led3_l3_0_6),
        .output_led4_l2_0_7(output_led4_l2_0_7),
        .output_led5_l0_0_8(output_led5_l0_0_8)
    );

    // Slow clock generation (1MHz = 1000ns period)
    always begin
        #500 input_clock2_slow_clk_2 = ~input_clock2_slow_clk_2;
    end

    // Fast clock generation (4MHz = 250ns period)
    always begin
        #125 input_clock3_fast_clk_3 = ~input_clock3_fast_clk_3;
    end

    // Pack outputs into a single state for analysis
    always @(*) begin
        output_state = {output_led5_l0_0_8, output_led4_l2_0_7, output_led3_l3_0_6,
                       output_led2_l1_0_5, output_led1_load_shift_0_4};
    end

    // Monitor state changes
    always @(output_state) begin
        if (output_state != previous_state) begin
            cycle_count = cycle_count + 1;
            $display("Cycle %0d: Reset=%b, SlowClk=%b, FastClk=%b => State: %5b (LoadShift=%b, L1=%b, L3=%b, L2=%b, L0=%b)",
                     cycle_count, input_push_button1_reset_1, input_clock2_slow_clk_2, input_clock3_fast_clk_3,
                     output_state, output_led1_load_shift_0_4, output_led2_l1_0_5, output_led3_l3_0_6,
                     output_led4_l2_0_7, output_led5_l0_0_8);
            previous_state = output_state;
        end
    end

    // Test reset functionality
    task test_reset;
        begin
            $display("\nTesting reset functionality...");
            test_count = test_count + 1;

            input_push_button1_reset_1 = 1; // Assert reset
            #200;

            $display("Reset asserted: State = %5b", output_state);

            input_push_button1_reset_1 = 0; // Release reset
            #200;

            $display("Reset released: State = %5b", output_state);
            pass_count = pass_count + 1;
        end
    endtask

    // Test clock interactions
    task test_clock_interaction;
        input integer num_cycles;
        begin
            $display("\nTesting clock interactions for %0d cycles...", num_cycles);
            test_count = test_count + 1;

            // Let clocks run for specified cycles
            repeat(num_cycles) begin
                @(posedge input_clock2_slow_clk_2 or posedge input_clock3_fast_clk_3);
            end

            $display("After %0d clock cycles: State = %5b", num_cycles, output_state);
            pass_count = pass_count + 1;
        end
    endtask

    // Main test sequence
    initial begin
        $display("=== SEQUENTIAL LOGIC TESTBENCH ===");
        $display("Testing sequential circuit with dual clocks and reset");

        // Initialize signals
        input_push_button1_reset_1 = 0;
        input_clock2_slow_clk_2 = 0;
        input_clock3_fast_clk_3 = 0;
        previous_state = 5'b00000;

        // Wait for initialization
        #100;

        // Test reset functionality
        test_reset();

        // Test normal operation with both clocks
        test_clock_interaction(10);

        // Test reset during operation
        $display("\nTesting reset during operation...");
        test_reset();

        // More operation testing
        test_clock_interaction(15);

        // Test different reset scenarios
        $display("\nTesting multiple reset cycles...");
        repeat(3) begin
            test_reset();
            test_clock_interaction(5);
        end

        // Final observation period
        $display("\nFinal observation period...");
        test_clock_interaction(20);

        // Stop clocks and observe final state
        input_clock2_slow_clk_2 = 0;
        input_clock3_fast_clk_3 = 0;
        #200;

        $display("\nFinal state: %5b", output_state);
        $display("Load/Shift signal: %b", output_led1_load_shift_0_4);
        $display("Data outputs: L1=%b, L3=%b, L2=%b, L0=%b",
                 output_led2_l1_0_5, output_led3_l3_0_6, output_led4_l2_0_7, output_led5_l0_0_8);

        // Summary
        $display("\n=== TEST SUMMARY ===");
        $display("Total tests: %0d", test_count);
        $display("Passed: %0d", pass_count);
        $display("Failed: %0d", test_count - pass_count);
        $display("State changes observed: %0d", cycle_count);

        if (pass_count == test_count) begin
            $display("*** ALL TESTS PASSED ***");
        end else begin
            $display("*** SOME TESTS FAILED ***");
        end

        $finish;
    end

    // Optional: Generate VCD file for waveform viewing
    initial begin
        $dumpfile("sequential_tb.vcd");
        $dumpvars(0, sequential_tb);
    end

endmodule