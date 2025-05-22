#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "io.h"
#include "mancala.h"

int Yacht(){return 0;}
int Nine_Mens_Morris(){return 0;}
int hit_and_blow(){return 0;}
int hit_and_blow_2(){return 0;}
// int Mancala(){return 0;}


#define DICE 5
#define ROUNDS 5
#define TOTAL_ROLES 13


unsigned int N1,N0,NN;
int int_to_gpio[10] = {0b00111111,0b00000110,0b01011011,0b01001111,0b01100110,0b01101101,0b01111101,0b00000111,0b01111111,0b01101111};


void timer_handler();
void print_segment_int(int M);


int main(){
    char ch = 0;
    int game,result;


    #if defined(NATIVE_MODE)
    printf("do nothing...\n");
    #else
    gpio->data = 0;
    while(1){
        printf("\n1  Manvala\n2  Yacht\n3  Nine Men's Morris\n4  Hit and Blow \n5  Hit and Blow 2\nwhat do you play? >>");
        ch = io_getch();
        if('1' <= ch && ch <= '5') {
            game = ch - '0';
            printf("\n");
            break;
        }
    }


    if(game == 1)       result = Mancala();
    else if(game == 2)  result = Yacht();
    else if(game == 3)  result = Nine_Mens_Morris();
    else if(game == 4)  result = hit_and_blow();
    else if(game == 5)  result = hit_and_blow_2();


    if(game == 2){
        print_segment_int(result);
        gpio->dout7SEG[1] = N1;
        gpio->dout7SEG[0] = N0;
    }
    else if(game == 5){
        print_segment_int(result);
        gpio->dout7SEG[1] = 0b01000110000111000101000001010100; // "turn"
        gpio->dout7SEG[0] = N0;
    }
    else{
        if(result == 1){
            printf("Winner is 1.\n");
            N1 = 0b00000000000000000000011000000000;
            N0 = 0b10111110001100000001000001010100;
        }
        else if(result == 2){
            printf("Winner is 1.\n");
            N1 = 0b00000000000000000101101100000000;
            N0 = 0b10111110001100000001000001010100;
        }
        else if(result == 0){
            printf("draw.");
            N1 = 0b00000000000000000000000001011110;
            N0 = 0b01010000010111001001110000010000;
        }
        else{
            printf("error");
            N1 = 0b00000000000000000000000001111001;
            N0 = 0b01010000010100000101110001010000;
        }
        set_mtimer_interval(500/*msec*/);
        timer_interrupt_hook = timer_handler;
        enable_timer_interrupt(); //割り込みハンドラを設定するための関数
    }
    printf("\npress any key to end your program...\n");
    io_getch();
    printf("\nThank you for playing!\n");
    asm volatile("csrc mie, %[new]" : : [new] "r" (0x00000080));    /// clear meie[7] : machine-external-interrupt-enable
    #endif
    return 0;
}






#if !defined(NATIVE_MODE)
void timer_handler(){
    NN = (N1 << 8) | (N0 >> 24);
    N0 = (N0 << 8) | (N1 >> 24);
    N1 = NN;
    gpio->dout7SEG[1] = N1;
    gpio->dout7SEG[0] = N0;
}
#endif




void print_segment_int(int M){
    int j, k;
    N0 = N1 = 0;
    if (M == 0) N1 = int_to_gpio[0];
    else {
        k = 1;
        while(k <= M) k *= 10;
        while(k != 1){
            k /= 10;
            j = M / k;
            if(k >= 10000)  N0 = (N0 << 8) & int_to_gpio[j];
            else            N1 = (N1 << 8) & int_to_gpio[j];
            M -= k * j;
        }
    }
}
