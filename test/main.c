#include <stdio.h>
#include <stdint.h>

#include "yabi/yabi.h"

void test1(void);

int main(void) {
   printf("Starting YABI Tests...\n");

   test1();

   return 0;
}

void test1(void) {
   yabi_Error res;

   struct yabi_Config cfg;

   res = yabi_init(&cfg);
}

