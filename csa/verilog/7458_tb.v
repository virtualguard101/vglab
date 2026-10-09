/**
 * Self-checking testbench for HDLBits 7458 (top_module in 7458.v).
 *
 * Usage:
 * iverilog -g2012 -Wall -o 7458.vvp 7458.v 7458_tb.v
 * vvp 7458.vvp
 */

module tb_7458;
  reg  p1a, p1b, p1c, p1d, p1e, p1f;
  reg  p2a, p2b, p2c, p2d;
  wire p1y, p2y;

  integer errors;

  top_module dut (
    .p1a(p1a), .p1b(p1b), .p1c(p1c),
    .p1d(p1d), .p1e(p1e), .p1f(p1f),
    .p1y(p1y),
    .p2a(p2a), .p2b(p2b), .p2c(p2c), .p2d(p2d),
    .p2y(p2y)
  );

  // Golden model: same boolean equations as the chip.
  function automatic expected_p1y;
    input a, b, c, d, e, f;
    begin
      expected_p1y = (a & b & c) | (d & e & f);
    end
  endfunction

  function automatic expected_p2y;
    input a, b, c, d;
    begin
      expected_p2y = (a & b) | (c & d);
    end
  endfunction

  task automatic check;
    input [5:0] p1;
    input [3:0] p2;
    reg exp1, exp2;
    begin
      {p1a, p1b, p1c, p1d, p1e, p1f} = p1;
      {p2a, p2b, p2c, p2d}           = p2;
      #1; // allow combinational settle
      exp1 = expected_p1y(p1a, p1b, p1c, p1d, p1e, p1f);
      exp2 = expected_p2y(p2a, p2b, p2c, p2d);
      if (p1y !== exp1 || p2y !== exp2) begin
        $display("FAIL t=%0t p1=%b p2=%b -> p1y=%b (exp %b) p2y=%b (exp %b)",
                 $time, p1, p2, p1y, exp1, p2y, exp2);
        errors = errors + 1;
      end else begin
        $display("PASS t=%0t p1=%b p2=%b -> p1y=%b p2y=%b",
                 $time, p1, p2, p1y, p2y);
      end
    end
  endtask

  initial begin
    $dumpfile("7458.vcd");
    $dumpvars(0, tb_7458);
  end

  initial begin
    errors = 0;

    // Spot checks covering AND/OR paths.
    check(6'b000000, 4'b0000);
    check(6'b111000, 4'b0000); // p1 left AND
    check(6'b000111, 4'b0000); // p1 right AND
    check(6'b111111, 4'b0000); // p1 both OR
    check(6'b000000, 4'b1100); // p2 left AND
    check(6'b000000, 4'b0011); // p2 right AND
    check(6'b000000, 4'b1111); // p2 both OR
    check(6'b101010, 4'b1010); // mixed zeros
    check(6'b111000, 4'b1100); // both sides active

    if (errors == 0)
      $display("ALL TESTS PASSED");
    else
      $display("%0d TEST(S) FAILED", errors);

    $finish;
  end
endmodule
