//Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
//Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
//--------------------------------------------------------------------------------
//Tool Version: Vivado v.2024.2 (win64) Build 5239630 Fri Nov 08 22:35:27 MST 2024
//Date        : Thu Apr  3 05:12:29 2025
//Host        : daiv running 64-bit major release  (build 9200)
//Command     : generate_target rv32_wrapper.bd
//Design      : rv32_wrapper
//Purpose     : IP block netlist
//--------------------------------------------------------------------------------
`timescale 1 ps / 1 ps

module rv32_wrapper
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
  input rst_n;
  input sys_clock;

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
  wire rst_n;
  wire sys_clock;

  rv32 rv32_i
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
        .rst_n(rst_n),
        .sys_clock(sys_clock));
endmodule
