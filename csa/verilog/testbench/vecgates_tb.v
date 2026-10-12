/**
 * Self-checking testbench for HDLBits vecgates (top_module in dut/vecgates.v).
 *
 * Usage:
 *    iverilog -g2012 -Wall -o vecgates.vvp dut/vecgates.v testbench/vecgates_tb.v
 *    vvp vecgates.vvp
 */

module tb_vecgates;
  reg  [2:0] a, b;
  wire [2:0] out_or_bitwise;
  wire       out_or_logical;
  wire [5:0] out_not;

  integer errors;
  integer i, j;

  top_module dut (
    .a(a),
    .b(b),
    .out_or_bitwise(out_or_bitwise),
    .out_or_logical(out_or_logical),
    .out_not(out_not)
  );

  function automatic [2:0] golden_bitwise;
    input [2:0] x, y;
    begin
      golden_bitwise = x | y;
    end
  endfunction

  function automatic golden_logical;
    input [2:0] x, y;
    begin
      golden_logical = (|x) || (|y);
    end
  endfunction

  function automatic [5:0] golden_not;
    input [2:0] x, y;
    begin
      golden_not = {~y, ~x};
    end
  endfunction

  task automatic check;
    input [2:0] av, bv;
    reg [2:0] exp_bw;
    reg       exp_l;
    reg [5:0] exp_n;
    begin
      a = av;
      b = bv;
      #1;
      exp_bw = golden_bitwise(a, b);
      exp_l  = golden_logical(a, b);
      exp_n  = golden_not(a, b);
      if (out_or_bitwise !== exp_bw ||
          out_or_logical !== exp_l ||
          out_not !== exp_n) begin
        $display("FAIL t=%0t a=%b b=%b -> bw=%b (exp %b) lg=%b (exp %b) not=%b (exp %b)",
                 $time, a, b,
                 out_or_bitwise, exp_bw,
                 out_or_logical, exp_l,
                 out_not, exp_n);
        errors = errors + 1;
      end
    end
  endtask

  initial begin
    $dumpfile("vecgates.vcd");
    $dumpvars(0, tb_vecgates);
  end

  initial begin
    errors = 0;

    for (i = 0; i < 8; i = i + 1)
      for (j = 0; j < 8; j = j + 1)
        check(i[2:0], j[2:0]);

    if (errors == 0)
      $display("ALL TESTS PASSED (64 vectors)");
    else
      $display("%0d TEST(S) FAILED", errors);

    $finish;
  end
endmodule
