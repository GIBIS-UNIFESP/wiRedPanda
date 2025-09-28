//========================================================================
// ULTRATHINK Parameterized Testbench Template for wiRedPanda Modules
//========================================================================
// This template provides standardized timing parameters and best practices
// for creating maintainable and configurable testbenches.

`timescale 1ns/1ps

module testbench_template_parameterized;

    //====================================================================
    // TIMING PARAMETERS - ULTRATHINK Parameterization
    //====================================================================
    // Clock timing parameters (in ns)
    parameter CLK_PERIOD_10MHZ = 100;   // 10 MHz clock (100ns period)
    parameter CLK_PERIOD_5MHZ  = 200;   // 5 MHz clock (200ns period)
    parameter CLK_PERIOD_20MHZ = 50;    // 20 MHz clock (50ns period)

    // Test timing parameters (in ns)
    parameter SETUP_TIME       = 10;    // Setup time before clock edge
    parameter HOLD_TIME        = 5;     // Hold time after clock edge
    parameter PROP_DELAY       = 20;    // Propagation delay for combinational logic
    parameter TEST_INTERVAL    = 100;   // Time between test cases
    parameter RESET_DURATION   = 200;   // Reset pulse duration
    parameter SIMULATION_TIME  = 2000;  // Total simulation time

    // Test configuration parameters
    parameter VERBOSE_MODE     = 1;     // Enable detailed logging
    parameter AUTO_FINISH      = 1;     // Automatically finish simulation

    //====================================================================
    // SIGNAL DECLARATIONS
    //====================================================================
    // Input stimulus signals
    reg clk;
    reg reset;
    reg [3:0] test_inputs;

    // Output monitoring signals
    wire [7:0] test_outputs;

    // Test control signals
    integer test_count = 0;
    integer pass_count = 0;
    integer fail_count = 0;
    reg test_running = 0;

    //====================================================================
    // DEVICE UNDER TEST INSTANTIATION
    //====================================================================
    // Replace with actual module instantiation
    // Example:
    // module_name dut (
    //     .clk(clk),
    //     .reset(reset),
    //     .inputs(test_inputs),
    //     .outputs(test_outputs)
    // );

    //====================================================================
    // CLOCK GENERATION - Parameterized
    //====================================================================
    initial begin
        clk = 0;
        forever #(CLK_PERIOD_10MHZ/2) clk = ~clk;
    end

    //====================================================================
    // RESET GENERATION - Parameterized
    //====================================================================
    initial begin
        reset = 1;
        #RESET_DURATION;
        reset = 0;
        if (VERBOSE_MODE) $display("[%0t] Reset released", $time);
    end

    //====================================================================
    // TEST STIMULUS GENERATION
    //====================================================================
    initial begin
        // Initialize signals
        test_inputs = 0;
        test_running = 1;

        // Wait for reset to complete
        wait(!reset);
        #SETUP_TIME;

        if (VERBOSE_MODE) $display("[%0t] Starting test sequence", $time);

        // Test case loop
        for (integer i = 0; i < 16; i = i + 1) begin
            test_count = test_count + 1;
            test_inputs = i;

            if (VERBOSE_MODE)
                $display("[%0t] Test %0d: inputs=%b", $time, test_count, test_inputs);

            #PROP_DELAY; // Wait for propagation

            // Add test verification here
            check_outputs(test_inputs, test_outputs);

            #TEST_INTERVAL; // Wait before next test
        end

        // Test completion
        test_running = 0;
        report_results();

        if (AUTO_FINISH) begin
            #(TEST_INTERVAL * 2);
            $finish;
        end
    end

    //====================================================================
    // TEST CHECKING FUNCTIONS
    //====================================================================
    task check_outputs;
        input [3:0] inputs;
        input [7:0] outputs;

        reg [7:0] expected;
        reg test_pass;

        begin
            // Replace with actual expected value calculation
            expected = ~inputs; // Example calculation
            test_pass = (outputs == expected);

            if (test_pass) begin
                pass_count = pass_count + 1;
                if (VERBOSE_MODE)
                    $display("  ✓ PASS: outputs=%b, expected=%b", outputs, expected);
            end else begin
                fail_count = fail_count + 1;
                $display("  ✗ FAIL: outputs=%b, expected=%b", outputs, expected);
            end
        end
    endtask

    //====================================================================
    // RESULT REPORTING
    //====================================================================
    task report_results;
        begin
            $display("\n" + "="*60);
            $display("ULTRATHINK TEST RESULTS");
            $display("="*60);
            $display("Total tests:  %0d", test_count);
            $display("Passed:       %0d", pass_count);
            $display("Failed:       %0d", fail_count);
            $display("Success rate: %0.1f%%", (pass_count * 100.0) / test_count);

            if (fail_count == 0) begin
                $display("*** ALL TESTS PASSED ***");
            end else begin
                $display("*** %0d TESTS FAILED ***", fail_count);
            end
            $display("="*60);
        end
    endtask

    //====================================================================
    // WAVEFORM GENERATION
    //====================================================================
    initial begin
        $dumpfile("testbench_template.vcd");
        $dumpvars(0, testbench_template_parameterized);
    end

    //====================================================================
    // SIMULATION TIMEOUT
    //====================================================================
    initial begin
        #SIMULATION_TIME;
        if (test_running) begin
            $display("ERROR: Simulation timeout after %0dns", SIMULATION_TIME);
            $finish;
        end
    end

endmodule

//========================================================================
// USAGE NOTES:
//========================================================================
// 1. Copy this template and rename the module
// 2. Replace DUT instantiation with your actual module
// 3. Modify check_outputs task for your specific verification needs
// 4. Adjust timing parameters as needed for your module
// 5. Add module-specific test patterns
// 6. Configure parameters at the top for different test scenarios
//========================================================================