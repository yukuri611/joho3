#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "io.h"


//Mancala
#define PITS            14
#define BUF_SIZE        40
#define BACK_SPACE_CODE 0x08

#define ANSI_PLAYER1       "\x1b[38;5;208m"
#define ANSI_PLAYER2         "\x1b[36m"
#define ANSI_POCKET      "\x1b[32m"
#define ANSI_BOLD      "\x1b[1m"
#define ANSI_UNBOLD    "\x1b[22m"
#define ANSI_RESET        "\x1b[0m"

extern void init_board(int board[]);
extern void print_board(int board[], int perspective, int current_player);
extern int is_game_over(int board[]);
extern void store_input(char* store, int* terminated);
extern int Mancala(void);


#define DICE 5
#define ROUNDS 5
#define TOTAL_ROLES 13

//Yacht
void roll_dice(int dice[DICE]);
int yacht_score(int dice[DICE], const char *role);
void uart_puts_id(int id, const char *s);
void uart_putint_id(int id, int n);
void uart_newline_id(int id);
void recv_choice(int id, char *buf, int maxlen);
int Yacht();

//Speed
extern int card_to_point(char card);
extern void dual_uart_puts(const char *s);
void dual_uart_putch(char c);
int uart_has_input(int pid);
char uart_receive(int pid);
void uart_clear_buf(int pid);
int Speed();

//Nine_Mens_Morris
int Nine_Mens_Morris();


//Hit_and_Blow
unsigned int counter = 0;
unsigned int N1,N0,NN,HiB_hit,HiB_and,HiB_or;
void timer_handler();
int Hit_and_Blow();
int Hit_and_Blow_2();
void Hit_and_Blow_2_print(unsigned short data[4]);
void Hit_and_Blow_2_timer_handler();