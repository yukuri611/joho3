
#include <stdio.h>
#include "io.h"
#include <stdlib.h>

#define BUF_SIZE   40
char rx_buf[BUF_SIZE];
int main() {
#if defined(NATIVE_MODE)
    printf("do nothing...\n");
#else
    gpio->data = 0;
    int terminated = 0;
    unsigned gpio_data = 0;
    int a, b;
    printf("Number A: ");
    int rx_pos = 0;
    while (1) {
        char ch = io_getch();
        gpio_data = (gpio_data << 8) | ch;
        gpio->data = gpio_data;
        if (ch == TERMINATE_CODE) {
            rx_buf[rx_pos] = 0;
            printf("%s\nCtrl-Z detected...\n", rx_buf);
            terminated = 1;
            break;
        }
        rx_buf[rx_pos++] = ch;
        if (ch == '\n' || rx_pos == BUF_SIZE - 1) {
            rx_buf[rx_pos] = 0;
            if (ch != '\n') { io_putch('\n'); }
            a = atoi(rx_buf);
            break;
        }
    }
    printf("a: %d\n", a);
    printf("Number B: ");
    rx_pos = 0;
    while (1) {
        char ch = io_getch();
        gpio_data = (gpio_data << 8) | ch;
        gpio->data = gpio_data;
        if (ch == TERMINATE_CODE) {
            rx_buf[rx_pos] = 0;
            printf("%s\nCtrl-Z detected...\n", rx_buf);
            terminated = 1;
            break;
        }
        rx_buf[rx_pos++] = ch;
        if (ch == '\n' || rx_pos == BUF_SIZE - 1) {
            rx_buf[rx_pos] = 0;
            if (ch != '\n') { io_putch('\n'); }
            b = atoi(rx_buf);
            printf("b: %d\n", b);
            
            break;
        }
    }
    printf("result: %d", a / b);
#endif
    return 0;
}

