
#include <stdio.h>
#include <stdlib.h>
#include "io.h"

#define BUF_SIZE   40

void store_int(int*);


int main() {
#if defined(NATIVE_MODE)
    printf("do nothing...\n");
#else
    gpio->data = 0;
    int terminated = 0;
    unsigned gpio_data = 0;
    int a;
    printf("Number A: ");
    store_int(&a);
    printf("a: %d\n", a);

    int b;
    printf("Number B: ");
    store_int(&b);
    printf("b: %d\n", b);

    printf("result + : %d\n", a + b);
    printf("result - : %d\n", a - b);
    printf("result * : %d\n", a * b);
    printf("result / : %d\n", a / b);
    printf("result %% : %d\n", a % b);

#endif
    return 0;
}

void store_int(int* store) {
#if defined(NATIVE_MODE)
    printf("hello...\n");
#else
    char rx_buf[BUF_SIZE];
    int rx_pos = 0;
    while (1) {
        char ch = io_getch();
        
        rx_buf[rx_pos++] = ch;
        if (ch == '\n' || rx_pos == BUF_SIZE - 1) {
            rx_buf[rx_pos] = 0;
            if (ch != '\n') { io_putch('\n'); }
            *store = atoi(rx_buf);
            break;
        }
    }
#endif
}

