`timescale 1ns / 1ps

module tflipflop_tb;

    // Testbench signals - Updated for input port dead code elimination
    wire output_led1_q_0_5;
    wire output_led2_q_0_6;

    // Test control
    integer test_count = 0;
    integer pass_count = 0;

    // Instantiate the Device Under Test (DUT) - Self-contained after input port dead code elimination
    tflipflop dut (
        .output_led1_q_0_5(output_led1_q_0_5),
        .output_led2_q_0_6(output_led2_q_0_6)
    );

    // Simple monitoring for self-contained T flip-flop
    initial begin
        // Monitor outputs over time
        forever begin
            #100;
            $display("Time %0t: Q=%b, Qbar=%b", $time, output_led1_q_0_5, output_led2_q_0_6);
        end
    end

    // Main test sequence
    initial begin
        $display("=== T FLIP-FLOP TESTBENCH ===");
        $display("Testing self-contained T flip-flop after input port dead code elimination");

        // Wait for initialization
        #100;

        $display("\nMonitoring self-contained T flip-flop behavior:");
        $display("Q=%b, Qbar=%b", output_led1_q_0_5, output_led2_q_0_6);

        // Run for sufficient time to observe any changes
        #1000;

        // Final state
        $display("\nFinal output state:");
        $display("output_led1_q_0_5 (Q) = %b", output_led1_q_0_5);
        $display("output_led2_q_0_6 (Qbar) = %b", output_led2_q_0_6);

        // Basic validation - outputs should be complementary
        if (output_led1_q_0_5 != output_led2_q_0_6) begin
            $display("*** VALIDATION PASSED: Outputs are complementary ***");
        end else begin
            $display("*** VALIDATION FAILED: Outputs should be complementary ***");
        end

        $display("Self-contained T flip-flop testbench completed");
        $finish;
    end

    initial begin
        $dumpfile("tflipflop_tb.vcd");
        $dumpvars(0, tflipflop_tb);
    end

endmodule