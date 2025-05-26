#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "io.h"


#define DICE 5
#define ROUNDS 5
#define TOTAL_ROLES 13


void roll_dice(char dice[DICE]);
void uart_puts(const char *s);
void uart_putint(int n);
void print_dice(const char dice[DICE]);
void send_str(const char *s) ;
void recv_str(char *buf, char max_len);
void send_int(int val);
int recv_int();
int yacht_score(const char dice[DICE], const char *chosen_role);
int Yacht();
