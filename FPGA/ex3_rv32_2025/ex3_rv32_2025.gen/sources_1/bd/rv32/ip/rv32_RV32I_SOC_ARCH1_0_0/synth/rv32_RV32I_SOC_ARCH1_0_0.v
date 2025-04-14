// (c) Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// (c) Copyright 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
// 
// This file contains confidential and proprietary information
// of AMD and is protected under U.S. and international copyright
// and other intellectual property laws.
// 
// DISCLAIMER
// This disclaimer is not a license and does not grant any
// rights to the materials distributed herewith. Except as
// otherwise provided in a valid license issued to you by
// AMD, and to the maximum extent permitted by applicable
// law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
// WITH ALL FAULTS, AND AMD HEREBY DISCLAIMS ALL WARRANTIES
// AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
// BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
// INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
// (2) AMD shall not be liable (whether in contract or tort,
// including negligence, or under any other theory of
// liability) for any loss or damage of any kind or nature
// related to, arising under or in connection with these
// materials, including for any direct, or any indirect,
// special, incidental, or consequential loss or damage
// (including loss of data, profits, goodwill, or any type of
// loss or damage suffered as a result of any action brought
// by a third party) even if such damage or loss was
// reasonably foreseeable or AMD had been advised of the
// possibility of the same.
// 
// CRITICAL APPLICATIONS
// AMD products are not designed or intended to be fail-
// safe, or for use in any application requiring fail-safe
// performance, such as life-support or safety devices or
// systems, Class III medical devices, nuclear facilities,
// applications related to the deployment of airbags, or any
// other applications that could lead to death, personal
// injury, or severe property or environmental damage
// (individually and collectively, "Critical
// Applications"). Customer assumes the sole risk and
// liability of any use of AMD products in Critical
// Applications, subject only to applicable laws and
// regulations governing limitations on product liability.
// 
// THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
// PART OF THIS FILE AT ALL TIMES.
// 
// DO NOT MODIFY THIS FILE.


// IP VLNV: xilinx.com:module_ref:RV32I_SOC_ARCH1:1.0
// IP Revision: 1

(* X_CORE_INFO = "RV32I_SOC_ARCH1,Vivado 2024.2" *)
(* CHECK_LICENSE_TYPE = "rv32_RV32I_SOC_ARCH1_0_0,RV32I_SOC_ARCH1,{}" *)
(* CORE_GENERATION_INFO = "rv32_RV32I_SOC_ARCH1_0_0,RV32I_SOC_ARCH1,{x_ipProduct=Vivado 2024.2,x_ipVendor=xilinx.com,x_ipLibrary=module_ref,x_ipName=RV32I_SOC_ARCH1,x_ipVersion=1.0,x_ipCoreRevision=1,x_ipLanguage=VERILOG,x_ipSimLanguage=MIXED,M_ID=0}" *)
(* IP_DEFINITION_SOURCE = "module_ref" *)
(* DowngradeIPIdentifiedWarnings = "yes" *)
module rv32_RV32I_SOC_ARCH1_0_0 (
  clk,
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

(* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 clk CLK" *)
(* X_INTERFACE_MODE = "slave" *)
(* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME clk, FREQ_HZ 50000000, FREQ_TOLERANCE_HZ 0, PHASE 0.0, CLK_DOMAIN /clk_wiz_clk_out1, INSERT_VIP 0" *)
input wire clk;
(* X_INTERFACE_INFO = "xilinx.com:signal:reset:1.0 rst_n RST" *)
(* X_INTERFACE_MODE = "slave" *)
(* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME rst_n, POLARITY ACTIVE_LOW, INSERT_VIP 0" *)
input wire rst_n;
input wire G_io_uart_0_rx;
input wire G_io_uart_0_cts;
input wire G_io_uart_1_rx;
input wire G_io_uart_1_cts;
input wire [15 : 0] G_io_gpio_sw_in;
input wire G_io_intr_p_2;
input wire G_io_intr_p_3;
input wire G_io_intr_p_4;
input wire G_io_intr_p_5;
input wire G_io_intr_p_6;
output wire G_io_uart_0_tx;
output wire G_io_uart_0_rts;
output wire G_io_uart_1_tx;
output wire G_io_uart_1_rts;
output wire [7 : 0] G_io_gpio_seg7_dout;
output wire [7 : 0] G_io_gpio_seg7_an;
output wire [14 : 0] G_io_gpio_led_out;
output wire G_RV32I_SOC_ARCH1_OUT;

  RV32I_SOC_ARCH1 #(
    .M_ID(0)
  ) inst (
    .clk(clk),
    .rst_n(rst_n),
    .G_io_uart_0_rx(G_io_uart_0_rx),
    .G_io_uart_0_cts(G_io_uart_0_cts),
    .G_io_uart_1_rx(G_io_uart_1_rx),
    .G_io_uart_1_cts(G_io_uart_1_cts),
    .G_io_gpio_sw_in(G_io_gpio_sw_in),
    .G_io_intr_p_2(G_io_intr_p_2),
    .G_io_intr_p_3(G_io_intr_p_3),
    .G_io_intr_p_4(G_io_intr_p_4),
    .G_io_intr_p_5(G_io_intr_p_5),
    .G_io_intr_p_6(G_io_intr_p_6),
    .G_io_uart_0_tx(G_io_uart_0_tx),
    .G_io_uart_0_rts(G_io_uart_0_rts),
    .G_io_uart_1_tx(G_io_uart_1_tx),
    .G_io_uart_1_rts(G_io_uart_1_rts),
    .G_io_gpio_seg7_dout(G_io_gpio_seg7_dout),
    .G_io_gpio_seg7_an(G_io_gpio_seg7_an),
    .G_io_gpio_led_out(G_io_gpio_led_out),
    .G_RV32I_SOC_ARCH1_OUT(G_RV32I_SOC_ARCH1_OUT)
  );
endmodule
