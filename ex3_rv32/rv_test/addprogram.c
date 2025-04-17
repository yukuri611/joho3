
#include <stdio.h>
#include <stdlib.h>
#include "io.h"

#define BUF_SIZE  9

void store_int(int*);


int main() {
#if defined(NATIVE_MODE)
    printf("do nothing...\n");
#else
    while(1) {
        int a;
        printf("Number A: ");
        store_int(&a);
        printf("a: %d\n", a);

        int b;
        printf("Number B: ");
        store_int(&b);
        printf("b: %d\n", b);

        if (b != 0) {
            printf("result / : %d\n", a + b);
            printf("mod result : %d\n", a - b);
        }
        else {
            printf("Error. Cannot divide with 0\n");
        }
    }

    

#endif
    return 0;
}

void store_int(int* store) {
#if defined(NATIVE_MODE)
#else
    char rx_buf[BUF_SIZE];
    int rx_pos = 0;
    while (1) {
        char ch = io_getch(); //teratermに入力された数値(char型)1つを取得
        
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

