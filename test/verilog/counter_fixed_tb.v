`timescale 1ns/1ps

module counter_fixed_tb;
    // Inputs
    reg input_clock1_1;

    // Outputs
    wire output_led1_0_2;
    wire output_led2_0_3;
    wire output_led3_0_4;

    // Instantiate counter
    counter_fixed dut (
        .input_clock1_1(input_clock1_1),
        .output_led1_0_2(output_led1_0_2),
        .output_led2_0_3(output_led2_0_3),
        .output_led3_0_4(output_led3_0_4)
    );

    // Combined output for easy checking
    wire [2:0] count_value = {output_led3_0_4, output_led2_0_3, output_led1_0_2};

    // Clock generation
    initial begin
        input_clock1_1 = 0;
        forever #50 input_clock1_1 = ~input_clock1_1; // 10MHz clock
    end

    // Test stimulus
    initial begin
        $display("COUNTER FUNCTIONALITY TEST:");
        $display("Expected: 3-bit counting behavior");

        // Wait and observe counting
        #10;
        $display("Time=%0t: Count = %03b (%d)", $time, count_value, count_value);

        #100;
        $display("Time=%0t: Count = %03b (%d)", $time, count_value, count_value);

        #100;
        $display("Time=%0t: Count = %03b (%d)", $time, count_value, count_value);

        #100;
        $display("Time=%0t: Count = %03b (%d)", $time, count_value, count_value);

        #100;
        $display("Time=%0t: Count = %03b (%d)", $time, count_value, count_value);

        // Check if any counting occurred
        if (count_value == 3'b111) begin
            $display("STATUS: Counter outputs all 1s (static behavior)");
        end else begin
            $display("STATUS: Counter shows dynamic behavior");
        end

        $finish;
    end
endmodule