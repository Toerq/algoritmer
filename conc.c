//typedef struct node node, pointer pointer;

#include <sys/types.h>
#include "tree.h"
#include <pthread.h>
#include <stdio.h> /* I/O functions: printf() ... */
#include <stdlib.h> /* rand(), srand() */
#include <unistd.h> /* read(), write() calls */
#include <assert.h> /* assert() */
#include <time.h>   /* time() */
#include <signal.h> /* kill(), raise() and SIG???? */
#include <string.h>
#include <stdint.h>
#include <limits.h>

//
#include <sys/types.h> /* pid */
#include <sys/wait.h> /* waitpid() */

typedef struct pair {
  int x;
  int y;
  int result_x;
  int result_y;
} pair;



void *f1(void *p_) {
  pair *p = (pair*) p_;

  int x = p->x;
  p->x = 1;
  int y = p->y;

  if (x == 0 && y == 0) { p->result_x = 1; }
  return NULL;
}

void *f2(void *p_) {
  pair *p = (pair*) p_;

  int y = p->y;
  p->y = 1;
  int x = p->x;

  if (x == 0 && y == 0) { p->result_y = 1; }
  return NULL;
}

int main() {
  pair *p = malloc(sizeof(pair));
  int count = 0;
  int iterations = 0;
  int count_x = 0;
  int count_y = 0;
  while(1) {
    pthread_t thread0;
    pthread_t thread1;


    p->x = 0;
    p->y = 0;
    p->result_x = 0;
    p->result_y = 0;

    pthread_create(&thread0, NULL, f1, p);
    pthread_create(&thread1, NULL, f2, p);

    pthread_join(thread0, NULL);
    pthread_join(thread1, NULL);

    
    if (p->result_x && p->result_y) {
      //      printf("^");
      printf("Reodering count: %d after %d iterations\n", count, iterations); 
      printf("x only count: %d\n", count_x);
      printf("y only count: %d\n", count_y);
            
      count++;
      count_x = 0;
      count_y = 0;
    } else if (p->result_x) {
      count_x++;
    } else if (p->result_y) {
      count_y++;
    }
    iterations++;
  }
  return 1;
}

/*
  gcc -lpthread -pthread -Wall -std=c11 conc.c -o conc
*/
