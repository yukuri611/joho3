//UARTバッファではなく、スタック上のバッファを使って情報をやり取りするプログラム。

#include <stdio.h>
#include "io.h"

#define BUF_SIZE  40
#define BACK_SPACE_CODE  0x08


void store_input(char* store, int* terminated);


int main() {
#if defined(NATIVE_MODE)
    printf("do nothing...\n");
#else
    enable_UART_interrupt();
    gpio->data = 0;
    int terminated = 0;
    unsigned gpio_data = 0;
    int i;

    int flag = 0;
    char buf[BUF_SIZE] = {'\0'};
    
    while (!terminated) {
        set_uart_ID(flag);
        if(buf[0] == '\0') {
            printf("From TeraTerm%d: \n", !flag);
        }
        else {
            printf("From TeraTerm%d: %s", !flag, buf);
        }
        printf("TeraTerm-%d  >> ", flag);
        //UART0の処理
        for (int i = 0; i < BUF_SIZE; i++) {
            buf[i] = '\0';
        }
        store_input(buf, &terminated);
        flag = !flag;
    }
    set_uart_ID(0);
#endif
    return 0;
}

void store_input(char* store, int* terminated) {
#if defined(NATIVE_MODE)
#else
    int buf_pos = 0;
    while (1) {
        char ch = io_getch(); //teratermに入力された数値(char型)1つを取得
        if (ch == BACK_SPACE_CODE) { //back_spaceが入力されたとき、1文字消去する
            if (buf_pos != 0) {
                store[buf_pos] = 0;
                buf_pos -= 1;
            }
            continue;
        }

        if (ch == TERMINATE_CODE) {
            store[buf_pos] = 0;
            printf("%s\nCtrl-Z detected...\n", store);
            *terminated = 1;
            break;
        }

        store[buf_pos++] = ch; //bufferに保存

        //改行入力か、bufferの最後（終端文字分を除く）まで到達したとき、while文を抜ける
        if (ch == '\n' || buf_pos == BUF_SIZE - 1) {
            store[buf_pos] = 0; //終端文字の設定
            if (ch != '\n') { io_putch('\n'); }
            break;
        }
    }
#endif
}