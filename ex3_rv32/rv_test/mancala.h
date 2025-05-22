#include <stdio.h>
#include <stdlib.h>
#include "io.h"

#define PITS            14
#define BUF_SIZE        40
#define BACK_SPACE_CODE 0x08

// ANSI カラーコード
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