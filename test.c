//typedef struct node node, pointer pointer;

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

#define CAS __sync_bool_compare_and_swap


typedef struct pair {
  int *first;
  int second;
} pair;

//
  //int state(node *pointer) {
//  int state = 0;
//  int bit0 = ((intptr_t) pointer >> 0) & 1;
//  int bit1 = ((intptr_t) pointer >> 1) & 1;
//  state |= bit0;
//  state |= 2*bit1;
//  return state;
//}
//
 //info *set_state(info *pointer, int mark) {
//  intptr_t temp = (intptr_t) pointer;
//  int bit0 = ((intptr_t) mark >> 0) & 1;
//  int bit1 = ((intptr_t) mark >> 1) & 1;
//  temp ^= (-bit0 ^ temp) & (1 << 0);
//  temp ^= (-bit1 ^ temp) & (1 << 1);
//  pointer = (int*) temp;
//  return pointer;
//}
//


enum { CLEAN = 0, MARKED = 1, IFLAG = 2, DFLAG = 3 };

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
  //pointer = (int*) temp;
  return temp;
}

typedef struct qe2 {
  int p;
} qe2;

typedef struct qe {
  qe2 *p;
} qe;

int main() {
  qe *qe1 = malloc(sizeof(qe));
  qe1->p = malloc(sizeof(qe2));
  intptr_t asd = (intptr_t) qe1;
  (qe1->p)->p = 123;
  printf("before: %d\n%d\n", qe1, (qe1->p)->p);
  asd ^= (-1 ^ asd) & (1 << 0);
  qe1 = (qe*) asd;
  printf("after: %d\n%d\n", qe1, (qe1->p)->p);
  //  int *i = malloc(sizeof(int));
  //*i = 42;
  //int *i_2 = malloc(sizeof(int));
  //*i_2 = *i;

  int *new = malloc(sizeof(int));
  intptr_t temp = (intptr_t) new;
  //  temp |= ((intptr_t) 1);
  new = set_state(new, 3);
  //  new = (int*) temp;
  int result = state(new);
  printf("Result: %d\n", result);
  pair p123; 
  p123.first = 0;
  printf("%d\n", p123.first);
  

  printf("%d, %d, %d, %d\n", sizeof(int*), sizeof(long*), sizeof(int), sizeof(intptr_t));
  int *pointer = malloc(4);
  intptr_t pointer_to = pointer;
  pointer_to |= (intptr_t) 1 << (intptr_t) (sizeof(intptr_t)*8 - 1);
  //pointer_to &= ~((intptr_t) 1 << (intptr_t) (sizeof(intptr_t)*8 - 1));
  pointer = pointer_to;
  //  pointer ^= (-1 ^ pointer) & ( 1 <<  63);
  printf("ja:%d\n", ((intptr_t) pointer >> 63) & 1);
  for (int x = 7; x >= 0; x--) {
    int bit = ((intptr_t) asd >> x) & 1;
       printf("%d: %d\n",x, bit);
  }
  printf("%d\n", 8*sizeof(long));
  
  long *ptr1 = malloc(sizeof(long));
  // Find page start
  printf("%d\n", ptr1);
  ptr1 = (long *) ((uintptr_t) ptr1 | ~(uintptr_t) 0xfff);
  printf("%d\n", ptr1);
  //  pointer ^= 1323;//(-1 ^ (long int) pointer) & (1 << sizeof(intptr_t));
  
  //  ptr1 |= 5;
  uintptr_t ptr3 = ptr1;
  ptr3 |= 5;
  ptr1 = ptr3;
  int *p1 = malloc(sizeof(int));
  int *p2 = malloc(sizeof(int));
  int jo = p1;
  printf("jo: %d\n", jo);
  *p1 = 1;
  *p2 = 1;
  CAS(p1, *p2, 3); //p1 redan en pekar så behöver ej &
  printf("Pointer: %d\n", *p1);
  pair q = {1,2};
  pair p = {1,2};
  //  pair new = {2,3};
  printf("Before: %d, %d\n", q.first, q.second);
  //  __sync_bool_compare_and_swap((long int*) &q, (long int) p, (long int) new);
  printf("After: %d, %d\n", q.first, q.second);
  int *i = malloc(sizeof(int));
  int *i_2 = i;//malloc(sizeof(int));
  int *k = malloc(sizeof(int));
  *i = 5;
  *k = 10;
  printf("Before: %d\n",i);
  //  int result = __sync_bool_compare_and_swap(&i, i_2, k); // CAS på pekare, gör & på pekaren
  printf("Before: %d\n", i);
  return 1;
}

			 /*
inc_cas(void *arg __attribute__((unused)))
{
    int i;
    int local_counter;
     TODO 2: Use the compare and swap primitive to manipulate the shared
     * variable 
    for (i = 0; i < INC_ITERATIONS; i++) {
        local_counter = counter;
        //counter += INCREMENT; // You need to replace this
        while(!(__sync_bool_compare_and_swap(&counter, local_counter, local_counter+INCREMENT))) {
            local_counter = counter; // when CAS fails, update the local_counter
        }
    }

    return NULL;
}

}
			 */
