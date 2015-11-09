#include <stdio.h> /* I/O functions: printf() ... */
#include <stdlib.h> /* rand(), srand() */
#include <unistd.h> /* read(), write() calls */
#include <assert.h> /* assert() */
#include <time.h>   /* time() */
#include <signal.h> /* kill(), raise() and SIG???? */
#include <string.h>

#include <sys/types.h> /* pid */
#include <sys/wait.h> /* waitpid() */
#include <stdint.h>


enum { CLEAN = 0, MARKED = 1, IFLAG = 2, DFLAG = 3 };

void printBits(unsigned int num)
{
   for(int bit=0;bit<(sizeof(unsigned int) * 8); bit++)
   {
      printf("%i ", num & 0x01);
      num = num >> 1;
   }
}


int state(int *pointer) {
  int state = 0;
  int bit0 = ((intptr_t) pointer >> 0) & 1;
  int bit1 = ((intptr_t) pointer >> 1) & 1;
  state |= bit0;
  state |= 2*bit1;
  return state;
}

int *set_state(int *pointer, int mark) {
  int bit0 = ((intptr_t) mark >> 0) & 1;
  int bit1 = ((intptr_t) mark >> 1) & 1;
  intptr_t temp = (intptr_t) pointer;
  temp ^= (-bit0 ^ temp) & (1 << 0);
  temp ^= (-bit1 ^ temp) & (1 << 1);
  pointer = (int*) temp;
  return pointer;
}

int *get_ptr(int *pointer) {
  intptr_t temp = (intptr_t) pointer;
  temp ^= (0 ^ temp) & (1 << 0);
  temp ^= (0 ^ temp) & (1 << 1);
  return temp;
}

int main() {
  long *p = malloc(sizeof(long*));
  printBits(p);
  printf("\n%p\n", p);
  p = set_state((int*) p, IFLAG);
  printBits(p);
  int *nobits = get_ptr(p);
  printf("\n");
  printBits(nobits);
  return 1;
}
