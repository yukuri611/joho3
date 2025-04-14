-- Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
-- Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
-- --------------------------------------------------------------------------------
-- Tool Version: Vivado v.2024.2 (win64) Build 5239630 Fri Nov 08 22:35:27 MST 2024
-- Date        : Thu Apr  3 05:17:15 2025
-- Host        : daiv running 64-bit major release  (build 9200)
-- Command     : write_vhdl -force -mode synth_stub
--               c:/Users/isshi/Documents/Lectures/ex3/ex3_2025/FPGA/ex3_rv32_2025/ex3_rv32_2025.gen/sources_1/bd/rv32/ip/rv32_RV32I_SOC_ARCH1_0_0/rv32_RV32I_SOC_ARCH1_0_0_stub.vhdl
-- Design      : rv32_RV32I_SOC_ARCH1_0_0
-- Purpose     : Stub declaration of top-level module interface
-- Device      : xc7a100tcsg324-1
-- --------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity rv32_RV32I_SOC_ARCH1_0_0 is
  Port ( 
    clk : in STD_LOGIC;
    rst_n : in STD_LOGIC;
    G_io_uart_0_rx : in STD_LOGIC;
    G_io_uart_0_cts : in STD_LOGIC;
    G_io_uart_1_rx : in STD_LOGIC;
    G_io_uart_1_cts : in STD_LOGIC;
    G_io_gpio_sw_in : in STD_LOGIC_VECTOR ( 15 downto 0 );
    G_io_intr_p_2 : in STD_LOGIC;
    G_io_intr_p_3 : in STD_LOGIC;
    G_io_intr_p_4 : in STD_LOGIC;
    G_io_intr_p_5 : in STD_LOGIC;
    G_io_intr_p_6 : in STD_LOGIC;
    G_io_uart_0_tx : out STD_LOGIC;
    G_io_uart_0_rts : out STD_LOGIC;
    G_io_uart_1_tx : out STD_LOGIC;
    G_io_uart_1_rts : out STD_LOGIC;
    G_io_gpio_seg7_dout : out STD_LOGIC_VECTOR ( 7 downto 0 );
    G_io_gpio_seg7_an : out STD_LOGIC_VECTOR ( 7 downto 0 );
    G_io_gpio_led_out : out STD_LOGIC_VECTOR ( 14 downto 0 );
    G_RV32I_SOC_ARCH1_OUT : out STD_LOGIC
  );

  attribute CHECK_LICENSE_TYPE : string;
  attribute CHECK_LICENSE_TYPE of rv32_RV32I_SOC_ARCH1_0_0 : entity is "rv32_RV32I_SOC_ARCH1_0_0,RV32I_SOC_ARCH1,{}";
  attribute CORE_GENERATION_INFO : string;
  attribute CORE_GENERATION_INFO of rv32_RV32I_SOC_ARCH1_0_0 : entity is "rv32_RV32I_SOC_ARCH1_0_0,RV32I_SOC_ARCH1,{x_ipProduct=Vivado 2024.2,x_ipVendor=xilinx.com,x_ipLibrary=module_ref,x_ipName=RV32I_SOC_ARCH1,x_ipVersion=1.0,x_ipCoreRevision=1,x_ipLanguage=VERILOG,x_ipSimLanguage=MIXED,M_ID=0}";
  attribute DowngradeIPIdentifiedWarnings : string;
  attribute DowngradeIPIdentifiedWarnings of rv32_RV32I_SOC_ARCH1_0_0 : entity is "yes";
  attribute IP_DEFINITION_SOURCE : string;
  attribute IP_DEFINITION_SOURCE of rv32_RV32I_SOC_ARCH1_0_0 : entity is "module_ref";
end rv32_RV32I_SOC_ARCH1_0_0;

architecture stub of rv32_RV32I_SOC_ARCH1_0_0 is
  attribute syn_black_box : boolean;
  attribute black_box_pad_pin : string;
  attribute syn_black_box of stub : architecture is true;
  attribute black_box_pad_pin of stub : architecture is "clk,rst_n,G_io_uart_0_rx,G_io_uart_0_cts,G_io_uart_1_rx,G_io_uart_1_cts,G_io_gpio_sw_in[15:0],G_io_intr_p_2,G_io_intr_p_3,G_io_intr_p_4,G_io_intr_p_5,G_io_intr_p_6,G_io_uart_0_tx,G_io_uart_0_rts,G_io_uart_1_tx,G_io_uart_1_rts,G_io_gpio_seg7_dout[7:0],G_io_gpio_seg7_an[7:0],G_io_gpio_led_out[14:0],G_RV32I_SOC_ARCH1_OUT";
  attribute X_INTERFACE_INFO : string;
  attribute X_INTERFACE_INFO of clk : signal is "xilinx.com:signal:clock:1.0 clk CLK";
  attribute X_INTERFACE_MODE : string;
  attribute X_INTERFACE_MODE of clk : signal is "slave";
  attribute X_INTERFACE_PARAMETER : string;
  attribute X_INTERFACE_PARAMETER of clk : signal is "XIL_INTERFACENAME clk, FREQ_HZ 50000000, FREQ_TOLERANCE_HZ 0, PHASE 0.0, CLK_DOMAIN /clk_wiz_clk_out1, INSERT_VIP 0";
  attribute X_INTERFACE_INFO of rst_n : signal is "xilinx.com:signal:reset:1.0 rst_n RST";
  attribute X_INTERFACE_MODE of rst_n : signal is "slave";
  attribute X_INTERFACE_PARAMETER of rst_n : signal is "XIL_INTERFACENAME rst_n, POLARITY ACTIVE_LOW, INSERT_VIP 0";
  attribute X_CORE_INFO : string;
  attribute X_CORE_INFO of stub : architecture is "RV32I_SOC_ARCH1,Vivado 2024.2";
begin
end;
