#include "io.h"
#include <stdlib.h>
#include<stdio.h>


extern int card_to_point(char card);
extern void dual_uart_puts(const char *s);
void dual_uart_putch(char c);
int uart_has_input(int pid);
char uart_receive(int pid);
void uart_clear_buf(int pid);
int Speed();
