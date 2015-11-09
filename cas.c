
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

typedef struct q2 {
  int p_int;
} q2;

typedef struct q1 {
  q2 *p;
} q1;

int main() {

  /* Swap value of pointee */
   int *x_0 = malloc(sizeof(int*));
   int *x_1 = malloc(sizeof(int*));
   
   *x_0 = 4;
   *x_1 = 4;
   
   printf("%d\n", *x_0);   
   int *res = __sync_val_compare_and_swap(x_0, *x_0, 2);
   printf("%d\n", *x_0);   
   printf("%d\n", res);   

  /* Swaps out pointer */
  //q1 *q_ = malloc(sizeof(q1*));
  //q1 *q__ = malloc(sizeof(q1*));
  //q_->p =malloc(sizeof(q2*));
  //printf("%d\n",   q_);
  //__sync_val_compare_and_swap(&q_, q_, 2);
  //printf("%d\n",   q_);


  /* Swaps value of pointee */
  // q1 *q_ = malloc(sizeof(q1*));
  // q_->p =malloc(sizeof(q2*));
  // q_->p->p_int = 123;
  // printf("%d\n",   q_->p->p_int);
  // __sync_bool_compare_and_swap(&(q_->p->p_int), q_->p->p_int, 2);
  // printf("%d\n",   q_->p->p_int);


  return 1;
}

  


























