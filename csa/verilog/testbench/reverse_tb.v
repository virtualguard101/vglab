/**
 * Self-checking testbench for dut/reverse.v (HDLBits Vectorr).
 *
 * Usage (from csa/verilog):
 *   iverilog -g2012 -Wall -o reverse.vvp dut/reverse.v testbench/reverse_tb.v
 *   vvp reverse.vvp
 */

module tb_reverse;
  reg  [7:0] in;
  wire [7:0] out;
  integer i, errors;

  top_module dut (.in(in), .out(out));

  function automatic [7:0] golden;
    input [7:0] x;
    integer k;
    begin
      for (k = 0; k < 8; k = k + 1)
        golden[k] = x[7 - k];
    end
  endfunction

  initial begin
    errors = 0;
    for (i = 0; i < 256; i = i + 1) begin
      in = i[7:0];
      #1;
      if (out !== golden(in)) begin
        $display("FAIL in=%b out=%b exp=%b", in, out, golden(in));
        errors = errors + 1;
      end
    end
    if (errors == 0)
      $display("ALL TESTS PASSED (256 vectors)");
    else
      $display("%0d TEST(S) FAILED", errors);
    $finish;
  end
endmodule
