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



typedef struct node {
  int value;
  struct node *next;
} node;


typedef struct queue {
  node *head;
  node *tail;
} queue;


int init(queue *queue) {
  node *init_node = malloc(sizeof(node));//{0, NULL};
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
    if (tail == Q->tail) { // True = not falling behind
      if (next == NULL) { // Next should be NULL
	if (__sync_bool_compare_and_swap(&tail->next, next, new_node)) {
	  break;
	}
      } else { // Tail falling behind, move it forward
	__sync_bool_compare_and_swap(&Q->tail, tail, next);
      }
    } 
  }
    __sync_bool_compare_and_swap(&Q->tail, tail, new_node); // Attempt to move tail forward
  return 1;
}

int dequeue(queue *Q, int *pvalue) {
  node *head = Q->head;
  while(1) {
    head = Q->head;
    node *tail = Q->tail;
    node *next = head->next;
    if (head == Q->head) {
      if (head == tail) { // Is queue empty or Tail falling behind?
	if (next == NULL) {
	  return 0; //Queue is empty
	}
	__sync_bool_compare_and_swap(&Q->tail, tail, next); // Tail is falling behind
      } else {
	*pvalue = next->value;
	if (__sync_bool_compare_and_swap(&Q->head, head, next)) { //Move head forward
	  break;
	}
      }
    }
  }
  free(head);
  return 1;
}

void print_function(queue *Q) {
  printf("Head=%d, val=%d, next=%d\n", Q->head, (Q->head)->value, (Q->head)->next);
  printf("Tail=%d val=%d, next=%d\n", Q->tail, (Q->tail)->value, (Q->tail)->next);
  node *node_ = Q->head;
  while(node_->next != NULL) {
    printf("Val=%d, next=%d\n", node_->value, node_->next);
    node_ = node_->next;
  }
  //(Q->tail)->next);
  //  printf(
  // printf(
}

int main() {
  queue *Q = malloc(sizeof(queue));
  init(Q);
  int p = -1;
  dequeue(Q, &p);
  printf("Result from dequeue: %d\n", p);
  int i = 5;
  int q = -1;
  // while (i++ < 20) {
  enqueue(Q, i);
      //      if (i % 2 == 0)
      //	dequeue(Q, p);
      //}
  dequeue(Q, &p);
  printf("Result from dequeue: %d\n", p);
  dequeue(Q, &q);
  printf("Result from dequeue: %d\n", q);
  print_function(Q);
}
