module bot(i, o) ;

    parameter p2 = 16 ;
    input [p2-1:0] i ;
    output [0:p2-1] o ;

    assign o = i+i*i-i ;
    assign o = i+i*i ;

endmodule
