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

enum  {inf2 = INT_MAX, inf1 = INT_MAX - 1};

enum { CLEAN = 0, MARKED = 1, IFLAG = 2, DFLAG = 3 };

enum { INSERT = 0, DELETE = 1 };

typedef struct update {
  info *info;
} update;

typedef struct info {
  int type; //0 = Insert info, 1 = Delete info
  /* Info super */
  node *parent;
  node *leaf;
  /* Insert */
  node *new_internal;
  /* Delete */
  node *grand_parent;
  info *pinfo;
} info;


typedef struct node {
  int is_leaf;
  int key;
  info *info;
  node *left, *right;
} node;
      

typedef struct search_return {
  node *grand_parent;
  node *parent;
  node *leaf;//, *new_internal;
  info *pinfo;
  info *gpinfo;
} search_return;


void printBits(unsigned int num)
{
   for(int bit=0;bit<(sizeof(unsigned int) * 8); bit++)
   {
      printf("%i ", num & 0x01);
      num = num >> 1;
   }
}


int get_state(info *pointer) {
  int state = 0;
  int bit0 = ((intptr_t) pointer >> 0) & 1;
  int bit1 = ((intptr_t) pointer >> 1) & 1;
  state |= bit0;
  state |= 2*bit1;
  return state;
}

info *set_state(info *pointer, int mark) {
  int bit0 = ((intptr_t) mark >> 0) & 1;
  int bit1 = ((intptr_t) mark >> 1) & 1;
  intptr_t temp = (intptr_t) pointer;
  temp ^= (-bit0 ^ temp) & (1 << 0);
  temp ^= (-bit1 ^ temp) & (1 << 1);
  //pointer = (int*) temp;
  return (info*) temp;
}     

info *get_ptr(info *pointer) {
  intptr_t temp = (intptr_t) pointer;
  temp ^= (0 ^ temp) & (1 << 0);
  temp ^= (0 ^ temp) & (1 << 1);
  return (info*) temp;
}


int max(int x, int y) {
  if (x > y) { return x;} else { return y; }
}     
      
      
search_return search(int key, node *root) {
  node *grand_parent, *parent;
  node *leaf = root;
  info *gpinfo, *pinfo;
  while (leaf->is_leaf == 0) {
    grand_parent = parent;
    parent = leaf;
    gpinfo = pinfo;
    pinfo = parent->info;
    if (key < leaf->key) {
      leaf = parent->left;
    } else {
      leaf = parent->right;
    } 
  }   
  search_return ret_struct = {grand_parent, parent, leaf};
  ret_struct.pinfo = pinfo;
  ret_struct.gpinfo = gpinfo;
  return ret_struct;
}     
      
int find(int key, node *root) {
  search_return search_result = search(key, root);
  if ((search_result.leaf)->key == key) {
    return 1;/*search_result.leaf;*/ } else {
    return 0;//NULL;
  }   
}     
      
      
void help_insert(info *op) {
  info *op_ptr = get_ptr(op);
  CAS_child(op_ptr->parent, op_ptr->leaf, op_ptr->new_internal); // ichild CAS
  __sync_bool_compare_and_swap(&(op_ptr->parent->info), set_state(op, IFLAG), set_state(op, CLEAN));   // iunflag CAS
}

int help_delete(info *op) {
  info *op_ptr = get_ptr(op);
  info *result = __sync_val_compare_and_swap(&(op_ptr->parent->info), op_ptr->pinfo, set_state(op, MARKED)); //Mark parent
  if (result == op_ptr->pinfo || result == set_state(op_ptr->parent->info, MARKED)) { //op-parent successfully marked
    help_marked(op); // Complete the deletion
    return 1;
  } else {
    help(result);
    __sync_bool_compare_and_swap(&(op_ptr->grand_parent->info), set_state(op, DFLAG), set_state(op, CLEAN)); //Backtrack
    return 0;
  }
}

void help_marked(info *op) {
  // Precond: op points to a DIinfo record
  node *other;
  info *op_ptr = get_ptr(op);
  if (op_ptr->parent->right == op_ptr->leaf) { other = op_ptr->parent->left; }
  else { other = op_ptr->parent->right; }
  CAS_child(op_ptr->grand_parent, op_ptr->parent, other);
  __sync_bool_compare_and_swap(&(op_ptr->grand_parent->info), set_state(op, DFLAG), set_state(op, CLEAN));
}
// Set other to point to the sibling of the node to which op->leaf points
  

void help(info *op) {
  int flag = get_state(op);
  if (flag == IFLAG) {  /*printf("insertion.\n");*/ help_insert(op); }
  if (flag == MARKED) { help_marked(op); }
  if (flag == DFLAG) { help_delete(op); }
}
      

int delete(int key, node *root) {
  node *parent, *new_internal;
  node *leaf, *new_sibling;
  info *result, *op;
      
  while(1) {
    //free(op);

    search_return search_result = search(key, root);
    node *leaf = search_result.leaf;
    node *parent = search_result.parent;
    node *grand_parent = search_result.grand_parent;
    info *pinfo = search_result.pinfo;
    info *gpinfo = search_result.gpinfo;
    
    if (leaf->key != key) { return 0; }
    if (get_state(gpinfo) != CLEAN) { help(gpinfo); }
    else if (get_state(pinfo) != CLEAN) { help(pinfo); }
    else {
      info *op = malloc(sizeof(info));
      op->parent = parent;
      op->grand_parent = grand_parent;
      op->leaf = leaf;
      op->pinfo = pinfo;
      info *result = __sync_val_compare_and_swap(&(grand_parent->info), gpinfo, set_state(op, DFLAG));
      if (result == gpinfo) {
	if (help_delete(op)) { return 1; }
	else { 		//	printf("Delete collision!\n");
	  help(result); }
      }
    }
  }
}

int insert(int key, node *root) {
  node *parent, *new_internal;
  node *leaf, *new_sibling;
  node *new = malloc(sizeof(node));
  new->key = key;
  new->is_leaf = 1;
  info *result, *op;
      
  while(1) {
    //    free(new_sibling);
    //    free(new_internal);
    //    free(op);
      
    search_return search_result = search(key, root);
    node *leaf = search_result.leaf;
    node *parent = search_result.parent;
    info *pinfo = search_result.pinfo;
      
    if (leaf->key == key) { return 0; }
    int mark = get_state(pinfo);
    if (mark != CLEAN) { help(pinfo); } else {
      
      new_sibling = malloc(sizeof(node));
      new_sibling->is_leaf = 1;
      new_sibling->key = leaf->key;
      
      new_internal = malloc(sizeof(node));
      new_internal->is_leaf = 0;
      new_internal->key = max(key, leaf->key);
      (new_internal->info) = malloc(sizeof(info));
      (new_internal->info) = set_state((new_internal->info), CLEAN);
      
      if (new->key < new_sibling->key) {
      	new_internal->left = new; new_internal->right = new_sibling; } else {
      	new_internal->left = new_sibling; new_internal->right = new; }
      
      info *op = malloc(sizeof(info));
      op->type = INSERT;
      op->parent = parent;
      op->leaf = leaf;
      op->new_internal = new_internal; // New "parent" replacing a leaf
      
      result = __sync_val_compare_and_swap(&(parent->info), pinfo, set_state(op, IFLAG));
      if (result == pinfo) {
	
	help_insert(op);
	return 1; }
      else {
	//	printf("Insert collision!\n");
	int flag = get_state((unsigned int) parent->info);
	if(flag != 0) {
	  //  printf("Flag in insert collision: %d", flag);
	}
	help(result); }
    } 
  }   
}     
      	
void CAS_child(node *parent, node *leaf, node* new_internal) {
  //Precond: parent points to an internal node and new_internal points to a Node.
  //This function tries to change one of the child fields of the node that parents points to from leaf to new_internal
  if (new_internal->key < parent->key) {
    __sync_bool_compare_and_swap(&(parent->left), leaf, new_internal); } else {
    __sync_bool_compare_and_swap(&(parent->right), leaf, new_internal); }
}

node *init() {
  node *root = malloc(sizeof(node));
  root->info = malloc(sizeof(info));
  root->left = malloc(sizeof(node));
  root->right = malloc(sizeof(node));
  root->left->info = malloc(sizeof(info));
  root->right->info = malloc(sizeof(info));

  root->key = inf2; // infinity_2
  (root->right)->key = inf2;
  (root->left)->key = inf1;
  (root->right)->is_leaf = 1;
  (root->left)->is_leaf = 1;
  root->is_leaf = 0;
  return root;
}

void print_tree_(node *root, int depth) {
  printf("Depth: %d, Key: %d, mark: %d\n", depth, root->key, get_state(root->info));
  if (root->is_leaf == 0) {
    printf("Left: ");
    print_tree_(root->left, depth + 1);
    printf("Right: ");
    print_tree_(root->right, depth + 1);
  }
}
void print_tree(node *root) { print_tree_(root, 0); }

void *insertion_1(void *root_) {
  node *root = (node*) root_;
  pthread_t Q = pthread_self();
  int P = (int) Q;
  int prev = 0;
  for(int i = 0; i < 4000; i++) {
    int r = rand()/100000;
    int val = r;
    if (rand() % 2 == 1) {
      int result = insert(val, root); 
      if (result == 1) {
	result = find(val, root);
	if(result != 1) {
	  	  printf("Insertion succeeded but not in tree\n");
	}
      }
    }  else {
      int exists_before = find(val, root);
      int result = delete(val, root);
      //     if (result == 0) printf("delete returned 0");
      int exists_after = find(val, root);
      if(exists_before == exists_after && exists_before == 1) {
	printf("deletion failed: %d\n", result);
      }
    }
    int prev = val;
  }
  return NULL;
}

void *insertion_2(void *root_) {
  node *root = (node*) root_;
  for(int i = 0; i < 1000; i++) {
    insert(i, root);
  }
  return NULL;
}

void *insertion_3(void *root_) {
  node *root = (node*) root_;
  for(int i = 0; i < 1000; i++) {
    delete(i, root);
  }
  return NULL;
}

void *insertion_4(void *root_) {
  node *root = (node*) root_;
  for(int i = 0; i < 10000; i++) {
    insert(i, root);
    delete(i-1, root);
  }
  return NULL;
}


int main() {

  pthread_t thread0;
  pthread_t thread1;
  pthread_t thread2;
  pthread_t thread3;
  pthread_t thread4;
  pthread_t thread5;
  
  node *root = init();
  // insertion((void*)root);
  int i = 1;
  while(1) {
  pthread_create(&thread0, NULL, insertion_1, root);
  pthread_create(&thread1, NULL, insertion_1, root);
  pthread_create(&thread2, NULL, insertion_1, root);
  pthread_create(&thread3, NULL, insertion_1, root);
  pthread_create(&thread4, NULL, insertion_1, root);
  pthread_create(&thread5, NULL, insertion_1, root);
  // //
  pthread_join(thread0, NULL);
  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);
  pthread_join(thread3, NULL);  
  pthread_join(thread4, NULL);  
  pthread_join(thread5, NULL);
  //      print_tree(root);
  if (i++ % 100 == 0) {
    printf("%d\n",i); }
  }
      insert(5,root);
      delete(4,root);
      insert(6,root);
      delete(5,root);
      insert(7,root);
      delete(6,root);

  
    //    insertion_4((void*) root);
  //
      //  printf("\nafter\n\n");
  //
       //         print_tree(root);

  return 1;
}


    //    insertion_4((void*) root);
