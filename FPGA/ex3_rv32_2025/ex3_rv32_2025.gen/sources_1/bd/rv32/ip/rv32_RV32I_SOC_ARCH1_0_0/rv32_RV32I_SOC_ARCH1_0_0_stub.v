// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2024.2 (win64) Build 5239630 Fri Nov 08 22:35:27 MST 2024
// Date        : Thu Apr  3 05:17:15 2025
// Host        : daiv running 64-bit major release  (build 9200)
// Command     : write_verilog -force -mode synth_stub
//               c:/Users/isshi/Documents/Lectures/ex3/ex3_2025/FPGA/ex3_rv32_2025/ex3_rv32_2025.gen/sources_1/bd/rv32/ip/rv32_RV32I_SOC_ARCH1_0_0/rv32_RV32I_SOC_ARCH1_0_0_stub.v
// Design      : rv32_RV32I_SOC_ARCH1_0_0
// Purpose     : Stub declaration of top-level module interface
// Device      : xc7a100tcsg324-1
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
(* CHECK_LICENSE_TYPE = "rv32_RV32I_SOC_ARCH1_0_0,RV32I_SOC_ARCH1,{}" *) (* CORE_GENERATION_INFO = "rv32_RV32I_SOC_ARCH1_0_0,RV32I_SOC_ARCH1,{x_ipProduct=Vivado 2024.2,x_ipVendor=xilinx.com,x_ipLibrary=module_ref,x_ipName=RV32I_SOC_ARCH1,x_ipVersion=1.0,x_ipCoreRevision=1,x_ipLanguage=VERILOG,x_ipSimLanguage=MIXED,M_ID=0}" *) (* DowngradeIPIdentifiedWarnings = "yes" *) 
(* IP_DEFINITION_SOURCE = "module_ref" *) (* X_CORE_INFO = "RV32I_SOC_ARCH1,Vivado 2024.2" *) 
module rv32_RV32I_SOC_ARCH1_0_0(clk, rst_n, G_io_uart_0_rx, G_io_uart_0_cts, 
  G_io_uart_1_rx, G_io_uart_1_cts, G_io_gpio_sw_in, G_io_intr_p_2, G_io_intr_p_3, 
  G_io_intr_p_4, G_io_intr_p_5, G_io_intr_p_6, G_io_uart_0_tx, G_io_uart_0_rts, 
  G_io_uart_1_tx, G_io_uart_1_rts, G_io_gpio_seg7_dout, G_io_gpio_seg7_an, 
  G_io_gpio_led_out, G_RV32I_SOC_ARCH1_OUT)
/* synthesis syn_black_box black_box_pad_pin="rst_n,G_io_uart_0_rx,G_io_uart_0_cts,G_io_uart_1_rx,G_io_uart_1_cts,G_io_gpio_sw_in[15:0],G_io_intr_p_2,G_io_intr_p_3,G_io_intr_p_4,G_io_intr_p_5,G_io_intr_p_6,G_io_uart_0_tx,G_io_uart_0_rts,G_io_uart_1_tx,G_io_uart_1_rts,G_io_gpio_seg7_dout[7:0],G_io_gpio_seg7_an[7:0],G_io_gpio_led_out[14:0],G_RV32I_SOC_ARCH1_OUT" */
/* synthesis syn_force_seq_prim="clk" */;
  (* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 clk CLK" *) (* X_INTERFACE_MODE = "slave" *) (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME clk, FREQ_HZ 50000000, FREQ_TOLERANCE_HZ 0, PHASE 0.0, CLK_DOMAIN /clk_wiz_clk_out1, INSERT_VIP 0" *) input clk /* synthesis syn_isclock = 1 */;
  (* X_INTERFACE_INFO = "xilinx.com:signal:reset:1.0 rst_n RST" *) (* X_INTERFACE_MODE = "slave" *) (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME rst_n, POLARITY ACTIVE_LOW, INSERT_VIP 0" *) input rst_n;
  input G_io_uart_0_rx;
  input G_io_uart_0_cts;
  input G_io_uart_1_rx;
  input G_io_uart_1_cts;
  input [15:0]G_io_gpio_sw_in;
  input G_io_intr_p_2;
  input G_io_intr_p_3;
  input G_io_intr_p_4;
  input G_io_intr_p_5;
  input G_io_intr_p_6;
  output G_io_uart_0_tx;
  output G_io_uart_0_rts;
  output G_io_uart_1_tx;
  output G_io_uart_1_rts;
  output [7:0]G_io_gpio_seg7_dout;
  output [7:0]G_io_gpio_seg7_an;
  output [14:0]G_io_gpio_led_out;
  output G_RV32I_SOC_ARCH1_OUT;
endmodule
