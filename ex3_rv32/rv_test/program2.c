
#include <stdio.h>
#include <stdlib.h>
#include "io.h"

#define BUF_SIZE  9
#define BACK_SPACE_CODE  0x08


void store_int(int*, int*);


int main() {
#if defined(NATIVE_MODE)
    printf("do nothing...\n");
#else
    start_profiler();
    int terminated = 0;
    while(!terminated) {
        int a;
        printf("Number A: \n");
        store_int(&a, &terminated);
        if (terminated) break;
        printf("a: %d\n\n", a);

        int b;
        printf("Number B: \n");
        store_int(&b, &terminated);
        if (terminated) break;
        printf("b: %d\n\n", b);

        if (b != 0) {
            printf("Division result: %f\n", (float)a/b);
            printf("Mod result: %d\n\n", a % b);
        }
        else {
            printf("Error. Cannot divide with 0\n\n");
        }
    }
    printf("End program\n");

    end_profiler();
#endif
    return 0;
}

void store_int(int* store, int* terminated) {
#if defined(NATIVE_MODE)
#else
    char rx_buf[BUF_SIZE];
    int rx_pos = 0;
    while (1) {
        char ch = io_getch(); //teratermに入力された数値(char型)1つを取得
        if (ch == BACK_SPACE_CODE) { //back_spaceが入力されたとき、1文字消去する
            if (rx_pos != 0) {
                rx_buf[rx_pos] = 0;
                rx_pos -= 1;
            }
            continue;
        }

        if (ch == TERMINATE_CODE) {
            rx_buf[rx_pos] = 0;
            printf("%s\nCtrl-Z detected...\n", rx_buf);
            *terminated = 1;
            break;
        }

        rx_buf[rx_pos++] = ch; //bufferに保存

        //改行入力か、bufferの最後（終端文字分を除く）まで到達したとき、while文を抜ける
        if (ch == '\n' || rx_pos == BUF_SIZE - 1) {
            rx_buf[rx_pos] = 0; //終端文字の設定
            if (ch != '\n') { io_putch('\n'); }
            *store = atoi(rx_buf);
            break;
        }
    }
#endif
}

