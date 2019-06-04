module top #(parameter p1 = 8, parameter p2 = 16)
            (input [0:p1-1] in1, input [0:p2-1] in2,
             output [0:p1-1] out1, output [0:p2-1] out2);

	bot #(.p2(p1)) i1 (in1, out1) ;
    bot #(.p2(p2)) i2 (in2, out2) ;

endmodule

module bot(i, o) ;

    parameter p2 = 16 ;
    input [p2-1:0] i ;
    output [0:p2-1] o ;

    assign o = ~i ;

endmodule