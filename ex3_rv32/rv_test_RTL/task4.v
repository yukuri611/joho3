module RV32_DC (ir, T, I, I_rd, I_rs1, I_rs2, imm, OP_type, LD_type, ST_type, SYS_type, MD_type);
input  [31:0] ir;
input  [ 3:0] T;
output [ 9:0] I;
output [ 4:0] I_rd, I_rs1, I_rs2;
output [31:0] imm;
output [13:0] OP_type;
output [ 4:0] LD_type;
output [ 2:0] ST_type;
output [ 7:0] SYS_type, MD_type;

wire [6:0] opcode = ir[6:0];
wire [6:0] funct7 = ir[31:25];
wire [2:0] funct3 = ir[14:12];

function [10:0] DEC_I;
    input [6:0] opcode;
    case(opcode)
        7'b0110011: DEC_I = (10'b1 <<`COMP);　//MULはこれ
        7'b0010011: DEC_I = (10'b1 <<`COMPI);
        7'b0110111: DEC_I = (10'b1 <<`LUI);
        7'b0010111: DEC_I = (10'b1 <<`AUIPC);
        7'b0000011: DEC_I = (10'b1 <<`LD);
        7'b0100011: DEC_I = (10'b1 <<`ST);
        7'b1101111: DEC_I = (10'b1 <<`JAL);
        7'b1100111: DEC_I = (10'b1 <<`JALR);
        7'b1100011: DEC_I = (10'b1 <<`BR);
        7'b1110011: DEC_I = (10'b1 <<`SYS);
        default:    DEC_I = 10'b0;
    endcase
endfunction

    assign I = DEC_I(opcode); //I はワンホット
    wire R_type = I[`COMP];  //MULの時、I ビットベクタで I[COMP] が 1 になる。
    wire I_type = I[`COMPI] | I[`LD] | I[`JALR] | I[`SYS];
    wire U_type = I[`LUI]   | I[`AUIPC];
    wire S_type = I[`ST];
    wire J_type = I[`JAL];
    wire B_type = I[`BR];

    assign I_rd  = (R_type | I_type | U_type | J_type) ? ir[11: 7] : 5'b00000;
    assign I_rs1 = (R_type | I_type | S_type | B_type) ? ir[19:15] : 5'b00000;
    assign I_rs2 = (R_type | S_type | B_type)          ? ir[24:20] : 5'b00000;

    wire [31:0] imm_i = (I_type) ? {{20{ir[31]}}, ir[31:20]} : 32'b0;
    wire [31:0] imm_s = (S_type) ? {{20{ir[31]}}, ir[31:25], ir[11:7]} : 32'b0;
    wire [31:0] imm_b = (B_type) ? {{20{ir[31]}}, ir[7], ir[30:25], ir[11:8], 1'b0} : 32'b0;
    wire [31:0] imm_j = (J_type) ? {{12{ir[31]}}, ir[19:12], ir[20], ir[30:21], 1'b0} : 32'b0;
    wire [31:0] imm_u = (U_type) ? {ir[31:12], 12'b0} : 32'b0;
    assign      imm = (T[0]) ? 32'd4 : (imm_i | imm_s | imm_b | imm_j | imm_u);
    wire        md_flag = (funct7 == 7'b0000001) & I[`COMP]; //MULの時1になる

function [13:0] DEC_COMP; //これはALUのみ。MULには関係ない
    input I_COMPI;
    input [6:0] funct7;
    input [2:0] funct3;
    reg f7_0, f7_32;
    begin
    f7_0  = (funct7 == 7'b0000000);
    f7_32 = (funct7 == 7'b0100000);
    case(funct3)
        3'b000:     DEC_COMP = (I_COMPI | f7_0) ? (14'b1 << `ADD) : (f7_32) ? (14'b1 << `SUB) : 14'b0;
        3'b010:     DEC_COMP = (14'b1 << `LT);
        3'b011:     DEC_COMP = (14'b1 << `LTU);
        3'b111:     DEC_COMP = (14'b1 << `AND);
        3'b110:     DEC_COMP = (14'b1 << `OR);
        3'b100:     DEC_COMP = (14'b1 << `XOR);
        3'b001:     DEC_COMP = (14'b1 << `SLL);
        default:    DEC_COMP = (f7_0) ? (14'b1 << `SRL) : (f7_32) ? (14'b1 << `SRA) : 14'b0;
    endcase
    end
endfunction

function [13:0] DEC_BR;
    input [2:0] funct3;
    begin
    case(funct3)
        3'b000:     DEC_BR = (14'b1 << `EQ);
        3'b001:     DEC_BR = (14'b1 << `NE);
        3'b100:     DEC_BR = (14'b1 << `LT);
        3'b110:     DEC_BR = (14'b1 << `LTU);
        3'b101:     DEC_BR = (14'b1 << `GE);
        3'b111:     DEC_BR = (14'b1 << `GEU);
        default:    DEC_BR = 14'b0;
    endcase
    end
endfunction

function [4:0] DEC_LD;
input [2:0] funct3;
    case(funct3)
        3'b000:     DEC_LD = (5'b1 << `LB);
        3'b100:     DEC_LD = (5'b1 << `LBU);
        3'b001:     DEC_LD = (5'b1 << `LH);
        3'b101:     DEC_LD = (5'b1 << `LHU);
        3'b010:     DEC_LD = (5'b1 << `LW);
        default:    DEC_LD = 5'b0;
    endcase
endfunction

function [2:0] DEC_ST;
input [2:0] funct3;
    case(funct3)
        3'b000:     DEC_ST = (3'b1 << `SB);
        3'b001:     DEC_ST = (3'b1 << `SH);
        3'b010:     DEC_ST = (3'b1 << `SW);
        default:    DEC_ST = 3'b0;
    endcase
endfunction

function [7:0] DEC_SYS;
input [11:0] imm12;
input [2:0] funct3;
input zero_19_7;
    case(funct3)
        3'b001: DEC_SYS = (8'b1 << `CSRRW);
        3'b010: DEC_SYS = (8'b1 << `CSRRS);
        3'b011: DEC_SYS = (8'b1 << `CSRRC);
        3'b101: DEC_SYS = (8'b1 << `CSRRWI);
        3'b110: DEC_SYS = (8'b1 << `CSRRSI);
        3'b111: DEC_SYS = (8'b1 << `CSRRCI);
        default:
            DEC_SYS = (zero_19_7 & (imm12 == 12'h302)) ? (8'b1 << `MRET) :
                           (zero_19_7 & (imm12 == 12'h105)) ? (8'b1 << `WFI) : 8'b0;
    endcase
endfunction

function [7:0] DEC_MD;
input [2:0] funct3;
    case(funct3)
        3'b000:  DEC_MD = (8'b1 << `MUL);
        3'b001:  DEC_MD = (8'b1 << `MULH);
        3'b010:  DEC_MD = (8'b1 << `MULHSU);
        3'b011:  DEC_MD = (8'b1 << `MULHU);
        3'b100:  DEC_MD = (8'b1 << `DIV);
        3'b101:  DEC_MD = (8'b1 << `DIVU);
        3'b110:  DEC_MD = (8'b1 << `REM);
        default: DEC_MD = (8'b1 << `REMU);
    endcase
endfunction

    wire [13:0] COMP_type = (T[2] & (~md_flag) & (I[`COMP] | I[`COMPI])) ? DEC_COMP(I[`COMPI], funct7, funct3) : 14'b0;
    wire [13:0] BR_type = (T[2] & I[`BR]) ? DEC_BR(funct3) : 14'b0;
    wire add_cond = T[0] | (T[2] & (I[`LUI] | I[`AUIPC] | I[`LD] | I[`ST] | I[`JAL] | I[`JALR])) | (T[3] & I[`BR]);
    assign OP_type = (add_cond) ? (14'b1 << `ADD) : (COMP_type | BR_type);
    assign LD_type = (T[2] & I[`LD]) ? DEC_LD(funct3) : (T[0]) ? (5'b1 << `LW) : 5'b0;
    assign ST_type = (T[2] & I[`ST]) ? DEC_ST(funct3) : 3'b0;
    assign SYS_type = (T[2] & I[`SYS]) ? DEC_SYS(ir[31:20], funct3, (ir[19:7] == 13'b0)) : 8'b0;
    assign MD_type = (T[2] & md_flag & I[`COMP]) ? DEC_MD(funct3) : 8'b0;　//MULのとき、md_flagが1なので実行、T[2]は演算実行タイミングを意味している。

endmodule   /// RV32I_DC


MD M3 (clk, rst_n, bus1, bus2, MD_type, MD_out, MD_pending);

module MD (clk, rst_n, x, y, MD_type, out, pending); /// MD: Multiply/Divide module
input clk, rst_n;
input [31:0] x, y;
input [7:0] MD_type;
output [31:0] out;
output pending;

    reg stt;
    reg [31:0] a, b, c;

    wire [31:0] a1 = (~stt) ? x : a << 1;
    wire [31:0] b1 = (~stt) ? y : b >> 1;
    wire [31:0] c1 = (~stt) ? 32'b0 : (b & 1) ? c + a : c;

    always @ (posedge clk or negedge rst_n)
    if (rst_n == 1'b0) begin
        stt <= 1'b0;
        a <= 32'b0;
        b <= 32'b0;
        c <= 32'b0;
    end
    else if (MD_type[`MUL]) begin /// currently, only MUL instruction is implemented
        if (~stt)
            stt <= 1'b1;
        else if (b1 == 32'b0)
            stt <= 1'b0;
        a <= a1;
        b <= b1;
        c <= c1;
    end
    
    assign pending = MD_type[`MUL] & ((~stt) | (stt & (b1 != 32'b0)));
    assign out = (stt) ? c1 : 32'b0;
endmodule