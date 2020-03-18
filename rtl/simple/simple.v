module simple
(
    clk,
    rst,
    a,
    b,
    cin,
    cout,
    y
);

    input clk;
    input rst;
    input a;
    input b;
    input cin;
    output cout;
    output y;

    assign cout = (a & b) | (a & cin) | (b & cin);
    assign y = a ^ b ^ cin;

endmodule
