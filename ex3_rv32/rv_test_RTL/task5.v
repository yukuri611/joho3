wire [ 7:0] MD_type;   /// { MUL, MULH, MULHSU, MULHU, DIV, DIVU, REM, REMU }



`define MUL    0
`define MULH   1
`define MULHSU 2
`define MULHU  3
`define DIV    4
`define DIVU   5
`define REM    6
`define REMU   7

module MD (
    input              clk,
    input              rst_n,
    input       [31:0] x,
    input       [31:0] y,
    input       [7:0]  MD_type,
    output reg  [31:0] out,
    output             pending
);
    reg        stt;         // 0: idle／load, 1: processing
    reg [31:0] a, b, c;

    // For DIVU: 組み込みの符号なし除算・剰余
    wire [31:0] divu_q = a / b;
    wire [31:0] divu_r = a % b;

    // 部分和演算（MUL）と同様に、DIVU も stt==0 でロード、stt==1 で計算完了に戻す
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            stt <= 1'b0;
            a   <= 32'b0;
            b   <= 32'b0;
            c   <= 32'b0;
            out <= 32'b0;
        end else begin
            // 乗算
            if (MD_type[`MUL]) begin
                if (!stt) begin
                    // load x,y, clear c
                    stt <= 1'b1;
                    a   <= x;
                    b   <= y;
                    c   <= 32'b0;
                end else begin
                    // shift-add の一ステップ
                    if (b[0]) c <= c + a;
                    a <= a << 1;
                    b <= b >> 1;
                    // 終了判定
                    if ((b >> 1) == 32'b0) stt <= 1'b0;
                end
                out <= (stt ? c + (b[0] ? a : 32'b0) : 32'b0);
            end
            // 符号なし除算
            else if (MD_type[`DIVU]) begin
                if (!stt) begin
                    // load operands
                    stt <= 1'b1;
                    a   <= x;
                    b   <= y;
                end else begin
                    // 1 サイクル後に quotient を出力して終了
                    out <= divu_q;
                    stt <= 1'b0;
                end
            end
            // それ以外（未実装命令）は idle のまま
            else begin
                stt <= 1'b0;
                out <= 32'b0;
            end
        end
    end

    // pending: MUL の場合は b をシフトし切るまで、DIVU の場合は stt が 1 の間
    assign pending =
        (MD_type[`MUL]  & (stt | ~stt))        // MUL は内部で b==0 まで running
     || (MD_type[`DIVU] & stt);

endmodule

