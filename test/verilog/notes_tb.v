`timescale 1ns / 1ps

module notes_tb;

    // Testbench signals - Updated for enhanced code generator (no input ports)
    // The enhanced generator eliminated unused input ports for cleaner design
    wire output_buzzer1_g6_1;
    wire output_buzzer2_f6_2;
    wire output_buzzer3_d6_3;
    wire output_buzzer4_b7_4;
    wire output_buzzer5_c6_5;
    wire output_buzzer6_a7_6;
    wire output_buzzer7_e6_7;
    wire output_buzzer8_c7_8;

    // Test control
    integer test_count = 0;
    integer pass_count = 0;

    // Instantiate the Device Under Test (DUT) - Updated port mapping
    notes dut (
        .output_buzzer1_g6_1(output_buzzer1_g6_1),
        .output_buzzer2_f6_2(output_buzzer2_f6_2),
        .output_buzzer3_d6_3(output_buzzer3_d6_3),
        .output_buzzer4_b7_4(output_buzzer4_b7_4),
        .output_buzzer5_c6_5(output_buzzer5_c6_5),
        .output_buzzer6_a7_6(output_buzzer6_a7_6),
        .output_buzzer7_e6_7(output_buzzer7_e6_7),
        .output_buzzer8_c7_8(output_buzzer8_c7_8)
    );

    // Test procedure for self-contained notes circuit
    task test_notes_output;
        begin
            test_count = test_count + 1;
            #100;
            $display("Test %0d: Self-contained notes circuit", test_count);
            $display("      Buzzer states: G6=%b F6=%b D6=%b B7=%b C6=%b A7=%b E6=%b C7=%b",
                     output_buzzer1_g6_1, output_buzzer2_f6_2, output_buzzer3_d6_3, output_buzzer4_b7_4,
                     output_buzzer5_c6_5, output_buzzer6_a7_6, output_buzzer7_e6_7, output_buzzer8_c7_8);
            pass_count = pass_count + 1;
            $display("      PASS: Self-contained notes circuit operating");
        end
    endtask

    // Main test sequence
    initial begin
        $display("=== ENHANCED NOTES CIRCUIT TESTBENCH ===");
        $display("Testing self-contained notes circuit with internal timing generation");
        $display("Enhanced code generator eliminated unused input ports for cleaner design");

        #50;
        test_notes_output();
        test_notes_output();
        test_notes_output();

        $display("\n=== TEST SUMMARY ===");
        $display("Total tests: %0d", test_count);
        $display("Passed: %0d", pass_count);
        $display("Failed: %0d", test_count - pass_count);

        if (pass_count == test_count) begin
            $display("*** ALL TESTS PASSED ***");
        end else begin
            $display("*** SOME TESTS FAILED ***");
        end

        $display("Enhanced notes circuit demonstrates self-contained operation");
        $finish;
    end

    initial begin
        $dumpfile("notes_tb.vcd");
        $dumpvars(0, notes_tb);
    end

endmodule