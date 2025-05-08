#include <stdio.h>
int main() {
    unsigned a = 10;
    // scanf("%u", &a);
    unsigned b = 2;
    unsigned c = a + b;
    unsigned d = a / b;
    unsigned e = a * b;
    
    printf("signed: %d\n",c);
    printf("unsigned: %d\n",d);
    return 0;
}