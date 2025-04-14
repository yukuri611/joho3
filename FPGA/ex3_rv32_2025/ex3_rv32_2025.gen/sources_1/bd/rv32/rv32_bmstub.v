// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// -------------------------------------------------------------------------------

`timescale 1 ps / 1 ps

(* BLOCK_STUB = "true" *)
module rv32 (
  sys_clock,
  rst_n,
  G_io_uart_0_rx,
  G_io_uart_0_cts,
  G_io_uart_1_rx,
  G_io_uart_1_cts,
  G_io_gpio_sw_in,
  G_io_intr_p_2,
  G_io_intr_p_3,
  G_io_intr_p_4,
  G_io_intr_p_5,
  G_io_intr_p_6,
  G_io_uart_0_tx,
  G_io_uart_0_rts,
  G_io_uart_1_tx,
  G_io_uart_1_rts,
  G_io_gpio_seg7_dout,
  G_io_gpio_seg7_an,
  G_io_gpio_led_out,
  G_RV32I_SOC_ARCH1_OUT
);

  (* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 CLK.SYS_CLOCK CLK" *)
  (* X_INTERFACE_MODE = "slave CLK.SYS_CLOCK" *)
  (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME CLK.SYS_CLOCK, FREQ_HZ 100000000, FREQ_TOLERANCE_HZ 0, PHASE 0.0, CLK_DOMAIN rv32_sys_clock, INSERT_VIP 0" *)
  input sys_clock;
  (* X_INTERFACE_INFO = "xilinx.com:signal:reset:1.0 RST.RST_N RST" *)
  (* X_INTERFACE_MODE = "slave RST.RST_N" *)
  (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME RST.RST_N, POLARITY ACTIVE_LOW, INSERT_VIP 0" *)
  input rst_n;
  (* X_INTERFACE_IGNORE = "true" *)
  input G_io_uart_0_rx;
  (* X_INTERFACE_IGNORE = "true" *)
  input G_io_uart_0_cts;
  (* X_INTERFACE_IGNORE = "true" *)
  input G_io_uart_1_rx;
  (* X_INTERFACE_IGNORE = "true" *)
  input G_io_uart_1_cts;
  (* X_INTERFACE_IGNORE = "true" *)
  input [15:0]G_io_gpio_sw_in;
  (* X_INTERFACE_IGNORE = "true" *)
  input G_io_intr_p_2;
  (* X_INTERFACE_IGNORE = "true" *)
  input G_io_intr_p_3;
  (* X_INTERFACE_IGNORE = "true" *)
  input G_io_intr_p_4;
  (* X_INTERFACE_IGNORE = "true" *)
  input G_io_intr_p_5;
  (* X_INTERFACE_IGNORE = "true" *)
  input G_io_intr_p_6;
  (* X_INTERFACE_IGNORE = "true" *)
  output G_io_uart_0_tx;
  (* X_INTERFACE_IGNORE = "true" *)
  output G_io_uart_0_rts;
  (* X_INTERFACE_IGNORE = "true" *)
  output G_io_uart_1_tx;
  (* X_INTERFACE_IGNORE = "true" *)
  output G_io_uart_1_rts;
  (* X_INTERFACE_IGNORE = "true" *)
  output [7:0]G_io_gpio_seg7_dout;
  (* X_INTERFACE_IGNORE = "true" *)
  output [7:0]G_io_gpio_seg7_an;
  (* X_INTERFACE_IGNORE = "true" *)
  output [14:0]G_io_gpio_led_out;
  (* X_INTERFACE_IGNORE = "true" *)
  output G_RV32I_SOC_ARCH1_OUT;

  // stub module has no contents

endmodule
