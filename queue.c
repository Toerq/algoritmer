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



typedef struct node {
  int value;
  struct node *next;
} node;


typedef struct queue {
  node *head;
  node *tail;
} queue;


int init(queue *queue) {
  node *init_node = malloc(sizeof(node));
  init_node->value = 0;
  init_node->next = NULL;
  queue->head = queue->tail = init_node;
  return 1;                
}

int enqueue(queue *Q, int val) {
  node *new_node = malloc(sizeof(node));
  new_node->value = val;
  new_node->next = NULL;
  node *tail = Q->tail;
  node *next = tail->next;
    while(1) {
    tail = Q->tail;
    next = tail->next;
    if (tail == Q->tail) {
      if (next == NULL) {
	if (__sync_bool_compare_and_swap(&tail->next, next, new_node)) { 
	  break;
	}
      } else {
	__sync_bool_compare_and_swap(&Q->tail, tail, next); // tail behind
      }
    }
  }
  __sync_bool_compare_and_swap(&Q->tail, tail, new_node);  // tail behind
  return 1;
}

// tail och head två interface, N = A v B
int dequeue(queue *Q, int *pvalue) {
  node *head = Q->head;
  while(1) {
    head = Q->head;
    node *tail = Q->tail;
    node *next = head->next;
    if (head == Q->head) {
      if (head == tail) { // Is queue empty or Tail falling behind?
	if (next == NULL) {
	  printf("3");
	  return 0; //Queue is empty
	}
	__sync_bool_compare_and_swap(&Q->tail, tail, next);
      } else {
	*pvalue = next->value;
	if (__sync_bool_compare_and_swap(&Q->head, head, next)) {
	  break;
	}
      }
    }
  }
  free(head);
  return 1;
}

void print_function(queue *Q) {
  printf("Head=%p, val=%d, next=%p\n", Q->head, (Q->head)->value, (Q->head)->next);
  printf("Tail=%p val=%d, next=%p\n", Q->tail, (Q->tail)->value, (Q->tail)->next);
  node *node_ = Q->head->next;
  do {
    printf("Ptr=%p, Val=%d, next=%p\n",node_, node_->value, node_->next);
    node_ = node_->next;
  } while(node_ != NULL);
}

void *insertion(void *queue_) {
  queue *Q = (queue*) queue_;
  int p = -1;
  for(int i = 0; i < 1000000; i++) {
    if(i % 4 == 0) { dequeue(Q, &p); } else {
      enqueue(Q, i); }
  }
  return NULL;
}


int main() {
  queue *Q = malloc(sizeof(queue));
  init(Q);
  int r = -1;
  enqueue(Q, 123);
  enqueue(Q, 124);
  dequeue(Q,&r);
  dequeue(Q,&r);
  printf("Dequeued: %d\n", r); 
  /*
  enqueue(Q, 44);

  dequeue(Q,&r);
  dequeue(Q,&r);



  pthread_t thread0;
  pthread_t thread1;
  pthread_t thread2;
  pthread_t thread3;
  pthread_t thread4;
  pthread_t thread5;
  
  //  node *root = init();
  // insertion((void*)root);
  //  int i = 1;
  //  while(1) {
  pthread_create(&thread0, NULL, insertion, Q);
  pthread_create(&thread1, NULL, insertion, Q);
  pthread_create(&thread2, NULL, insertion, Q);
  pthread_create(&thread3, NULL, insertion, Q);
  pthread_create(&thread4, NULL, insertion, Q);
  pthread_create(&thread5, NULL, insertion, Q);
  // //
  pthread_join(thread0, NULL);
  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);
  pthread_join(thread3, NULL);  
  pthread_join(thread4, NULL);  
  pthread_join(thread5, NULL);
  */
  //      print_tree(root);
  //  print_function(Q);


  
    //    insertion_4((void*) root);
  //
      //  printf("\nafter\n\n");
  //
       //         print_tree(root);

  return 1;


  
}
