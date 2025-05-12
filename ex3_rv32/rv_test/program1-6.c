#include <stdio.h>
#include "io.h"


#define LINE_BUF_SIZE 128


int main() {
#if defined(NATIVE_MODE)
    printf("NATIVE_MODE: do nothing...\n");
#else
    enable_UART_interrupt();
    gpio->data = 0;


    int terminated = 0;
    char line[LINE_BUF_SIZE];
    int pos;


    // 初期プロンプト表示 (UART0 → PC)
    set_uart_ID(0);
    const char* prompt = "UART0 (PC側) >> ";
    for (const char* p = prompt; *p; ++p) {
        _UARTBuf_putch(*p);
    }


    while (!terminated) {
        // (1) UART0 (PC側)から文字列を割込みベースで受信
        UARTBuf* ub_pc_rx = &UART_BUF_RX[0];
        if (ub_pc_rx->num_new_lines || ub_pc_rx->terminate_code || ub_pc_rx->full || ub_pc_rx->not_empty) {
            set_uart_ID(0);  // PC側
            pos = 0;
            while (1) {
                char ch = _UARTBuf_getch();


                if (ch == TERMINATE_CODE) {
                    terminated = 1;
                    break;
                }


                if (pos < LINE_BUF_SIZE - 1) {
                    line[pos++] = ch;
                }


                if (ch == '\n') {
                    break;
                }
            }
            line[pos] = '\0';


            // (2) 受信文字列をUART1経由で相手FPGAへ送信 (割込みベース)
            set_uart_ID(1);
            for (int i = 0; line[i]; ++i) {
                _UARTBuf_putch(line[i]);
            }


            if (terminated) {
                _UARTBuf_putch(TERMINATE_CODE);  // 相手に終了通知
                terminated= 1;
                break;
            }


            // プロンプト再表示 (UART0 → PC)
            set_uart_ID(0);
            for (const char* p = prompt; *p; ++p) {
                _UARTBuf_putch(*p);
            }


            // 改行消費
            if (ub_pc_rx->num_new_lines) {
                UARTBuf_consume_new_line(ub_pc_rx);
            }
        }


        // (3) UART1 (相手FPGA側)から文字列受信→PC側(UART0)へ表示
        UARTBuf* ub_pmod_rx = &UART_BUF_RX[1];
        if (ub_pmod_rx->num_new_lines || ub_pmod_rx->terminate_code || ub_pmod_rx->full || ub_pmod_rx->not_empty) {
            set_uart_ID(1);  // 相手FPGA側
            pos = 0;
            while (1) {
                char ch = _UARTBuf_getch();


                if (ch == TERMINATE_CODE) {
                    terminated = 1;
                    break;
                }


                if (pos < LINE_BUF_SIZE - 1) {
                    line[pos++] = ch;
                }


                if (ch == '\n') {
                    break;
                }
            }
            line[pos] = '\0';


            // 受信文字列をPC(UART0)に表示（"receive: "付き）
            set_uart_ID(0);
            const char* recv_str = "\nreceive: ";
            for (const char* p = recv_str; *p; ++p) {
                _UARTBuf_putch(*p);
            }
            for (int i = 0; line[i]; ++i) {
                _UARTBuf_putch(line[i]);
            }


            if (terminated) {
                const char* endmsg = "\nCtrl-Z detected from remote FPGA. exiting...\n";
                for (const char* p = endmsg; *p; ++p) {
                    _UARTBuf_putch(*p);
                }
                break;
            }


            // プロンプト再表示 (UART0 → PC)
            for (const char* p = prompt; *p; ++p) {
                _UARTBuf_putch(*p);
            }


            // 改行消費
            if (ub_pmod_rx->num_new_lines) {
                UARTBuf_consume_new_line(ub_pmod_rx);
            }
        }
    }


    // UARTをフラッシュして終了処理
    flush_UART();
    set_uart_ID(0);
#endif
    return 0;
}


