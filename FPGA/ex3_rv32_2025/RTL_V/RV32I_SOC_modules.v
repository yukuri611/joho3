


module reset_gen   (
    /* global inputs */
    clk,    /// input from clock generator
    rst_n,  /// input for clock generator reset
    locked, /// input from clock generator
    /* outputs */
    rst_n_locked
  );

  /*input ports*/
  input wire        clk;
  input wire        rst_n;
  input wire        locked;

  /*output ports*/
  output reg       rst_n_locked       ; /// <0,1> [U1]  ;

  always @ (posedge clk or negedge rst_n)
  if (rst_n == 0)
    rst_n_locked <= 0;
  else
    rst_n_locked <= locked;

endmodule

module uart_switch (
    /// input
    sw, 
    /// inout
    rx_tx,   tx_rx,
    cts_rts, rts_cts,
    /// input
    tx, rts,
    /// output
    rx, cts,
    );

    /// input
    inout wire rx_tx, tx_rx, cts_rts, rts_cts;
    input wire[15:0] sw;
    input wire tx, rts;
    output wire rx, cts;
    
    wire dir = sw[15];

  /*
    (dir == 0):
    rx_tx   ---> rx
    rts_cts ---> cts
    tx_rx   <--- tx
    cts_rts <--- rts

    (dir == 1):
    rx_tx   <--- tx
    rts_cts <--- rts
    tx_rx   ---> rx
    cts_rts ---> cts
  */

  assign rx_tx   = (dir) ? tx   : 1'bz;
  assign cts_rts = (dir) ? rts  : 1'bz;
  assign tx_rx   = (dir) ? 1'bz : tx ;
  assign rts_cts = (dir) ? 1'bz : rts;

  assign rx      = (dir) ? tx_rx   : rx_tx  ;
  assign cts     = (dir) ? rts_cts : cts_rts;

  endmodule
