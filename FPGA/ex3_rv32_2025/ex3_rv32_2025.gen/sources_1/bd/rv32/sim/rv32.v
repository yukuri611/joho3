//Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
//Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
//--------------------------------------------------------------------------------
//Tool Version: Vivado v.2024.2 (win64) Build 5239630 Fri Nov 08 22:35:27 MST 2024
//Date        : Thu Apr  3 05:12:29 2025
//Host        : daiv running 64-bit major release  (build 9200)
//Command     : generate_target rv32.bd
//Design      : rv32
//Purpose     : IP block netlist
//--------------------------------------------------------------------------------
`timescale 1 ps / 1 ps

(* CORE_GENERATION_INFO = "rv32,IP_Integrator,{x_ipVendor=xilinx.com,x_ipLibrary=BlockDiagram,x_ipName=rv32,x_ipVersion=1.00.a,x_ipLanguage=VERILOG,numBlks=4,numReposBlks=4,numNonXlnxBlks=0,numHierBlks=0,maxHierDepth=0,numSysgenBlks=0,numHlsBlks=0,numHdlrefBlks=2,numPkgbdBlks=0,bdsource=USER,da_board_cnt=2,da_clkrst_cnt=1,synth_mode=Hierarchical}" *) (* HW_HANDOFF = "rv32.hwdef" *) 
module rv32
   (G_RV32I_SOC_ARCH1_OUT,
    G_io_gpio_led_out,
    G_io_gpio_seg7_an,
    G_io_gpio_seg7_dout,
    G_io_gpio_sw_in,
    G_io_intr_p_2,
    G_io_intr_p_3,
    G_io_intr_p_4,
    G_io_intr_p_5,
    G_io_intr_p_6,
    G_io_uart_0_cts,
    G_io_uart_0_rts,
    G_io_uart_0_rx,
    G_io_uart_0_tx,
    G_io_uart_1_cts,
    G_io_uart_1_rts,
    G_io_uart_1_rx,
    G_io_uart_1_tx,
    rst_n,
    sys_clock);
  output G_RV32I_SOC_ARCH1_OUT;
  output [14:0]G_io_gpio_led_out;
  output [7:0]G_io_gpio_seg7_an;
  output [7:0]G_io_gpio_seg7_dout;
  input [15:0]G_io_gpio_sw_in;
  input G_io_intr_p_2;
  input G_io_intr_p_3;
  input G_io_intr_p_4;
  input G_io_intr_p_5;
  input G_io_intr_p_6;
  input G_io_uart_0_cts;
  output G_io_uart_0_rts;
  input G_io_uart_0_rx;
  output G_io_uart_0_tx;
  input G_io_uart_1_cts;
  output G_io_uart_1_rts;
  input G_io_uart_1_rx;
  output G_io_uart_1_tx;
  (* X_INTERFACE_INFO = "xilinx.com:signal:reset:1.0 RST.RST_N RST" *) (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME RST.RST_N, INSERT_VIP 0, POLARITY ACTIVE_LOW" *) input rst_n;
  (* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 CLK.SYS_CLOCK CLK" *) (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME CLK.SYS_CLOCK, CLK_DOMAIN rv32_sys_clock, FREQ_HZ 100000000, FREQ_TOLERANCE_HZ 0, INSERT_VIP 0, PHASE 0.0" *) input sys_clock;

  wire G_RV32I_SOC_ARCH1_OUT;
  wire [14:0]G_io_gpio_led_out;
  wire [7:0]G_io_gpio_seg7_an;
  wire [7:0]G_io_gpio_seg7_dout;
  wire [15:0]G_io_gpio_sw_in;
  wire G_io_intr_p_2;
  wire G_io_intr_p_3;
  wire G_io_intr_p_4;
  wire G_io_intr_p_5;
  wire G_io_intr_p_6;
  wire G_io_uart_0_cts;
  wire G_io_uart_0_rts;
  wire G_io_uart_0_rx;
  wire G_io_uart_0_tx;
  wire G_io_uart_1_cts;
  wire G_io_uart_1_rts;
  wire G_io_uart_1_rx;
  wire G_io_uart_1_tx;
  wire clk_wiz_clk_out1;
  wire clk_wiz_locked;
  wire reset_gen_0_rst_n_locked;
  wire [0:0]reset_inv_0_Res;
  wire rst_n;
  wire sys_clock;

  rv32_RV32I_SOC_ARCH1_0_0 RV32I_SOC_ARCH1_0
       (.G_RV32I_SOC_ARCH1_OUT(G_RV32I_SOC_ARCH1_OUT),
        .G_io_gpio_led_out(G_io_gpio_led_out),
        .G_io_gpio_seg7_an(G_io_gpio_seg7_an),
        .G_io_gpio_seg7_dout(G_io_gpio_seg7_dout),
        .G_io_gpio_sw_in(G_io_gpio_sw_in),
        .G_io_intr_p_2(G_io_intr_p_2),
        .G_io_intr_p_3(G_io_intr_p_3),
        .G_io_intr_p_4(G_io_intr_p_4),
        .G_io_intr_p_5(G_io_intr_p_5),
        .G_io_intr_p_6(G_io_intr_p_6),
        .G_io_uart_0_cts(G_io_uart_0_cts),
        .G_io_uart_0_rts(G_io_uart_0_rts),
        .G_io_uart_0_rx(G_io_uart_0_rx),
        .G_io_uart_0_tx(G_io_uart_0_tx),
        .G_io_uart_1_cts(G_io_uart_1_cts),
        .G_io_uart_1_rts(G_io_uart_1_rts),
        .G_io_uart_1_rx(G_io_uart_1_rx),
        .G_io_uart_1_tx(G_io_uart_1_tx),
        .clk(clk_wiz_clk_out1),
        .rst_n(reset_gen_0_rst_n_locked));
  rv32_clk_wiz_0 clk_wiz
       (.clk_in1(sys_clock),
        .clk_out1(clk_wiz_clk_out1),
        .locked(clk_wiz_locked),
        .reset(reset_inv_0_Res));
  rv32_reset_gen_0_0 reset_gen_0
       (.clk(clk_wiz_clk_out1),
        .locked(clk_wiz_locked),
        .rst_n(rst_n),
        .rst_n_locked(reset_gen_0_rst_n_locked));
  rv32_reset_inv_0_0 reset_inv_0
       (.Op1(rst_n),
        .Res(reset_inv_0_Res));
endmodule
