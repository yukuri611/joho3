#include <stdio.h>
#include <stdlib.h>
#include "io.h"




unsigned int counter = 0;
unsigned int N1,N0,NN,HiB_hit,HiB_and,HiB_or;
void timer_handler();
int Hit_and_Blow();
int Hit_and_Blow_2();
void Hit_and_Blow_2_print(unsigned short data[4]);
void Hit_and_Blow_2_timer_handler();