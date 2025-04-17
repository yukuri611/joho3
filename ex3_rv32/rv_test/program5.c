
#include <stdio.h>
#include "io.h"

#if !defined(NATIVE_MODE)



char convert_char_seg7(int ch);

#define GPIO_STR8_new(s) ((((U64)convert_char_seg7(s[0])) << 0x38) | (((U64)convert_char_seg7(s[1])) << 0x30) | \
                            (((U64)convert_char_seg7(s[2])) << 0x28) | (((U64)convert_char_seg7(s[3])) << 0x20) | \
                            (((U64)convert_char_seg7(s[4])) << 0x18) | (((U64)convert_char_seg7(s[5])) << 0x10) | \
                            (((U64)convert_char_seg7(s[6])) << 0x08) | (((U64)convert_char_seg7(s[7])) << 0x00))

void set_gpio_string(U64 val);
unsigned counter = 0;

const char *string_list[] = {"Apple", "Banana", "Cherry", "Date"};
int size = sizeof(string_list) / sizeof(string_list[0]);

void timer_handler() {
    counter ++;
    printf("counting\n");
    const char* s = string_list[counter % size];
    char padded[9] = "        "; // 空白8文字 + null終端
    strncpy(padded, s, 8);
    set_gpio_string(GPIO_STR8_new(padded));
}
#endif

int main() {
#if defined(NATIVE_MODE)
    printf("do nothing...\n");
#else
    set_mtimer_interval(1000/*msec*/); 
    // time_interrupt_hookは、timer_intervalごとに実行される割り込み関数のポインタ。
    // timer_interrupt_hookに、上で作成したtimer_handler関数を割り当てる。
    timer_interrupt_hook = timer_handler; 
    enable_timer_interrupt(); //割り込みハンドラを設定するための関数
    int timer_switch = 1; //timer_switchが1のとき、timerによる割り込みが可能
    printf("timer on\n");
    while (1) {
        if (io_getch()) { //Teratermからの入力を待つ
            timer_switch = !timer_switch;   // timer_switchが切り替わる
            if (timer_switch) {
                asm volatile("csrs mie, %[new]" : : [new] "r" (0x00000080));	///	set meie[7] : machine-external-interrupt-enable
                printf("timer on\n");
            }
            else {
                asm volatile("csrc mie, %[new]" : : [new] "r" (0x00000080));	///	clear meie[7] : machine-external-interrupt-enable
                printf("timer off\n");
                const char* s = "timeroff";
                char padded[9] = "        "; 
                strncpy(padded, s, 8);
                set_gpio_string(GPIO_STR8_new(padded)); //7-SEGにtimeroffと表示
            }
        }
    }
#endif
    return 0;
}
