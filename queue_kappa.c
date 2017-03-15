#include <pthread.h>
#include <stdio.h> /* I/O functions: printf() ... */
#include <stdlib.h> /* rand(), srand() */


typedef struct node {
  int value;
  struct node *next;
} node;

typedef struct node_val {
  int value;
} node_val;


typedef struct stym_node {
  int value;
  struct node *next;
} stym_node;

typedef struct link_node {
  struct node *node;
} link_node;

typedef struct queue {
  node *head;
  node *tail;
} queue;

typedef struct node_has_int {
  int value;
} node_has_int;



#define CAT(a,b,c) (__sync_bool_compare_and_swap(&a,(node*) b, (node*) c) ? c = NULL, 1 : 0)
#define CAT_unlink(a,b,c, barred) (__sync_bool_compare_and_swap(&a,(node*) b, (node*) c) ? barred->value = c->value, c = NULL, 1 : 0)

void print_function_simple(queue *Q) {
  printf("-------------------------\nQ=%p\n", Q);
  printf("Head=%p, val=%d, next=%p\n", Q->head, (Q->head)->value, (Q->head)->next);
  printf("Tail=%p val=%d, next=%p\n", Q->tail, (Q->tail)->value, (Q->tail)->next);
}


void print_function(queue *Q) {
  printf("-------------------------\nQ=%p\n", Q);
  printf("Head=%p, val=%d, next=%p\n", Q->head, (Q->head)->value, (Q->head)->next);
  printf("Tail=%p val=%d, next=%p\n", Q->tail, (Q->tail)->value, (Q->tail)->next);
  node *node_ = Q->head;
  do {
    printf("Ptr=%p, Val=%d, next=%p\n",node_, node_->value, node_->next);
    node_ = node_->next;
  } while(node_ != NULL);
}

stym_node *speculate(node *n) {
  return (stym_node*) n;
}


stym_node *get_next(stym_node *n) {
  return (stym_node*) n->next;
}

queue *init() {
  queue *Q = malloc(sizeof(queue));
  node *init_node = malloc(sizeof(node));
  init_node->value = 1;
  init_node->next = NULL;
  Q->head = Q->tail = init_node;
  return Q;                
}

int enqueue(queue *Q, int val) {
  node *new_node = malloc(sizeof(node));
  new_node->value = val;
  new_node->next = NULL;
  stym_node *tail = speculate(Q->tail);
  stym_node *next = get_next(tail);

  while(1) {
    tail = speculate(Q->tail);
    next = get_next(tail);
    if (tail == speculate(Q->tail)) {
      if (next == NULL) {
	if (CAT(tail->next, next, new_node)) {
	  break;
	}
      } else {
	CAT(Q->tail, tail, next); // tail behind
      }
    }
  }
  node *tail_next = tail->next;
  if (tail_next == NULL)
    {
      printf("Hello!!!!!!!!!1%p\n", tail_next);
      
    }
  CAT(Q->tail, tail, tail_next);
  return 1;
}

int dequeue(queue *Q) {
  stym_node *head = speculate(Q->head);
  stym_node *tail = speculate(Q->tail);
  stym_node *next = speculate(head->next);
  node_has_int *barred_value = malloc(sizeof(node_has_int));
  int return_value = 0;

  while(1) {
    head = speculate(Q->head);
    tail = speculate(Q->tail);
    next = speculate(head->next);
    if (head == speculate(Q->head)) {
      if (head == tail) { // Is queue empty or Tail falling behind?
	if (next == NULL) {
	  return 0; //Queue is empty
	}
	CAT(Q->tail, tail, next);
      } else {
	if(CAT_unlink(Q->head, head, next, barred_value)) {
	  return_value = barred_value->value;
	  //free(head);
	  //free(barred_value);
	  return return_value;
	}
      }
    }
  }
  //free(head);
  return 1;
}

void *insertion(void *queue_) {
  queue *Q = (queue*) queue_;
  //  int p = -1;
  for(int i = 0; i < 1000000003; i++) {
    if(i % 2 == 0) { enqueue(Q, i); } else {
      dequeue(Q); }
  }
  return NULL;
}


int main() {
  int thread_count = 8;
  pthread_t threads[thread_count];
  queue *Q = init();
  printf("-------- after init ------\n");
  print_function_simple(Q);
  printf("------- after --------\n");
  print_function(Q);
  

  for(int i = 0; i < thread_count; ++i) {
    pthread_create(&threads[i], NULL, insertion, Q);
  }

  for(int i = 0;i < thread_count; ++i) {
    pthread_join(threads[i], NULL);
  }

  // print_function(Q);

  enqueue(Q, 42);
  enqueue(Q, 43);
  int ret1 = dequeue(Q);
  int ret2 = dequeue(Q);
  printf("ret1: %d, ret2: %d\n", ret1, ret2);

  

  return 1;


          
}
