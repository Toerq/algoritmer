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


typedef struct node {
  int key;
  struct node *next;
} node;


typedef struct list {
  node *head;
  node *tail;
} list;

node *search(list *list, int search_key, node **left_node);

int is_marked(node *node) {
  int ret = ((intptr_t) node >> (sizeof(intptr_t)*8 -1)) & 1;
  return ret;
}

node *get_marked_reference(node *node) {
  intptr_t temp = (intptr_t) node;
  temp |= (intptr_t) 1 << (intptr_t) (sizeof(intptr_t)*8 - 1);
  //  node = temp;
  return temp;
}
    
node *get_unmarked_reference(node *node) {
  intptr_t temp = (intptr_t) node;
  temp &= ~((intptr_t) 1 << (intptr_t) (sizeof(intptr_t)*8 - 1));
  //  node = temp;
  return temp;
}

list init() {
  node *head = malloc(sizeof(node));//{0, NULL};
  node *tail = malloc(sizeof(node));//{0, NULL};
  head->next = tail;
  list new_list = {head, tail};
  head->key = -1;
  tail->key = -2;
  return new_list;
}

int insert(list *list, int key) {
  node *new_node = malloc(sizeof(node));
  new_node->key = key;
  node *right_node = list->head;
  node **left_node_ref;
  node *left_node;
  do {
    right_node = search(list, key, &left_node_ref);
    left_node = (node*) left_node_ref;
    if ((right_node != list->tail) && (right_node->key == key)) { //Only unique values
      free(new_node);
      return 0;
    }
    new_node->next = right_node;
  } while (__sync_bool_compare_and_swap(&left_node->next, right_node, new_node)); 
    return 1;
}

int delete(list *list, int search_key) {
  node *right_node, *right_node_next, *left_node, **left_node_ref;

  do {
    right_node = search(list, search_key, &left_node_ref);
    left_node = (node*) left_node_ref;
    if ((right_node == list->tail) || (right_node->key != search_key)) {
      //right_node is a dummy node or doesn't exist in the list
      return 0;
    }
    right_node_next = right_node->next;
    if (!is_marked(right_node_next)) {
      //Logical deletion
      if (__sync_bool_compare_and_swap(&(right_node->next),
				       right_node_next, get_marked_reference(right_node_next))) {
	break;
      }
    }
  } while (1);

  //Physical deletion
  if (!(__sync_bool_compare_and_swap(&(left_node->next), right_node, right_node_next))) {
    right_node = search(list, right_node->key, left_node);
  }
  return 1;
}

int find(list *list, int search_key) {
  node *right_node, *left_node, **left_node_ref;
  
  right_node = search(list, search_key, &left_node_ref);
  left_node = (node*) left_node_ref;
  if ((right_node == list->tail) || (right_node->key != search_key)) {
    return 0;
  } else {
    return 1;
  }
}

node *search(list *list, int search_key, node **left_node) {
  node *left_node_next, *right_node;
  
 search_again:
  while (1) {
    node *t = list->head;
    node *t_next = (list->head)->next;
    
    // 1: Find left_node and right_node
    do {
      if (!is_marked(t_next)) { // If t is not marked
	*left_node = t;
	left_node_next = t_next;
      }
      t = get_unmarked_reference(t_next);       // Move t one node forward
      if (t == list->tail) break;               // Exit loop if t is the last node
      t_next = t->next;                         // Update t_next
    } while(is_marked(t_next) || (t->key < search_key)); 
    // Exit loop if t->key >= search_key or if t is marked

    right_node = t;

    // 2: Check nodes are adjacent
    if (left_node_next == right_node) {
      if ((right_node != list->tail) && is_marked(right_node->next)) {
	// Search again if right_node is marked and is not the last (dummy) node.
	goto search_again;
      } else {
	return right_node;
      }
    }
    // 3: Remove one or more marked nodes
    if (__sync_bool_compare_and_swap(&(*left_node)->next, left_node_next, right_node)) {
      if ((right_node != list->tail) && is_marked(right_node->next)) {
	// Search again if right_node is marked and is not the tail node.
	goto search_again;
      } else {
	return right_node;
      }
    }
  }
}

void print_function(list *list) {
  node *node = list->head;
  int i = 0;
  printf("head: %d, tail: %d\n", list->head, list->tail);
  while(node->next != NULL) {
    printf("adress: %d next: %d, key: %d\n",node, node->next,node->key);
    i++;
    node = node->next;
  }
  printf("adress: %d next: %d, key: %d\n",node, node->next,node->key);
}

int main() {
  list list = init();
  for(int i = 0; i < 10; i++) {
    insert(&list, i);
  }
  insert(&list, 11);
  insert(&list, 10);
  delete(&list, 5);
  print_function(&list);
  int q = 0;
  q = find(&list, -1);
  printf("%d\n", q);
  
  //  insert(&list, 4);
  //  insert(&list, 5);
  
  return 1;
}
