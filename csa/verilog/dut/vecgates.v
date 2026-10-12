module top_module( 
    input [2:0] a,
    input [2:0] b,
    output [2:0] out_or_bitwise,
    output out_or_logical,
    output [5:0] out_not
);

    wire [2:0] bus1 = a[2:0];
    wire [2:0] bus2 = b[2:0];
    
    // assign out_or_bitwise = a | b;
    assign out_or_bitwise[2] = bus1[2] | bus2[2];
    assign out_or_bitwise[1] = bus1[1] | bus2[1];
    assign out_or_bitwise[0] = bus1[0] | bus2[0];
    
    assign out_or_logical = bus1 || bus2;
    
    // assign out_not = {~bus2, ~bus1};
    assign out_not[5] = ~bus2[2];
    assign out_not[4] = ~bus2[1];
    assign out_not[3] = ~bus2[0];
    assign out_not[2] = ~bus1[2];
    assign out_not[1] = ~bus1[1];
    assign out_not[0] = ~bus1[0];

endmodule
