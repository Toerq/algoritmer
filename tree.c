//typedef struct node node, pointer pointer;

#include "tree.h"
#include <stdio.h> /* I/O functions: printf() ... */
#include <stdlib.h> /* rand(), srand() */
#include <unistd.h> /* read(), write() calls */
#include <assert.h> /* assert() */
#include <time.h>   /* time() */
#include <signal.h> /* kill(), raise() and SIG???? */
#include <string.h>
#include <stdint.h>
 //
 #include <sys/types.h> /* pid */
 #include <sys/wait.h> /* waitpid() */

enum  {inf2 = 10000001, inf1 = 10000000};

enum { CLEAN = 0, MARKED = 1, IFLAG = 2, DFLAG = 3 };

enum { INSERT = 0, DELETE = 1 };

typedef struct update {
  info *info;
} update;

typedef struct info {
  int type; //0 = Insert info, 1 = Delete info
  /* Info super */
  node *internal, *parent;
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
      
      
//typedef struct info {
  //  int type; //0 = Insert info, 1 = Delete info
  //  node *parent, *new_node, *grand_parent, *new_internal;
  //  node *leaf;
  //  info pinfo;
  //} info;
   
typedef struct search_return {
  node *grand_parent;
  node *parent;
  node *leaf;//, *new_internal;
  info *pinfo;
  info *gpinfo;
} search_return;

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

int *get_ptr(info *pointer) {
  intptr_t temp = (intptr_t) pointer;
  temp ^= (0 ^ temp) & (1 << 0);
  temp ^= (0 ^ temp) & (1 << 1);
  return (int*) temp;
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
      
node *find(int key, node *root) {
  search_return search_result = search(key, root);
  if ((search_result.leaf)->key == key) {
    return search_result.leaf; } else {
    return NULL;
  }   
}     
      
      
void help_insert(info *op) {
  CAS_child(op->parent, op->leaf, op->new_internal); // ichild CAS
  __sync_bool_compare_and_swap(&(op->parent->info), set_state(op, IFLAG), set_state(op, CLEAN));   // iunflag CAS
}

int help_delete(info *op) {
  // precond: op points to a DInfo record
  //  info *result;
  info *result = __sync_val_compare_and_swap(&(op->parent->info), op->pinfo, set_state(op, MARKED));
  if (result == op->pinfo || op->parent->info == set_state(op->parent->info, MARKED)) { //op-parent successfully marked
    help_marked(op); // Complete the deletion
    return 1;
  } else {
    help(result);
    __sync_bool_compare_and_swap(&(op->parent->info), set_state(op, DFLAG), set_state(op, CLEAN)); //Backtrack
    return 0;
  }
}

void help_marked(info *op) {
  // Precond: op points to a DIinfo record
  node *other;
  if (op->parent->right == op->leaf) { other = op->parent->left; }
  else { other = op->parent->right; }
  CAS_child(op->grand_parent, op->parent, other);
  __sync_bool_compare_and_swap(&(op->grand_parent->info), set_state(op, DFLAG), set_state(op, DFLAG));
}
  // Set other to point to the sibling of the node to which op->leaf points
  

void help(info *op) {
int flag = get_state(op);
if (flag == IFLAG) { help_insert(op); }
  if (flag == MARKED) { help_marked(op); }
  if (flag == DFLAG) { help_delete(op); }
}
      

int insert(int key, node *root) {
  node *parent, *new_internal;
  node *leaf, *new_sibling;
  node *new = malloc(sizeof(node));
  new->key = key;
  new->is_leaf = 1;
  info *result;
  //info *op;
      
  while(1) {
    search_return search_result = search(key, root);
    node *leaf = search_result.leaf;
    node *parent = search_result.parent;
    info *pinfo = search_result.pinfo;
      
    if (leaf->key == key) { return 0; }
    int mark = get_state(pinfo);
    if (mark != CLEAN) 
      { help(pinfo); } else {
      
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
      else { help(result); }
    } 
  }   
}     
      	
void CAS_child(node *parent, node *old, node* new) {
  //Precond: parent points to an internal node and new points to a Node.
  //This function tries to change one of the child fields of the node that parents points to from old to new
  if (new->key < parent->key) {
    __sync_bool_compare_and_swap(&(parent->left), old, new); } else {
    __sync_bool_compare_and_swap(&(parent->right), old, new); }
}

node *init() {
  node *root = malloc(sizeof(node));
  (root->info).info = malloc(sizeof(info));
  root->left = malloc(sizeof(node));
  root->right = malloc(sizeof(node));
  (root->left->info).info = malloc(sizeof(info));
  (root->right->info).info = malloc(sizeof(info));

  root->key = inf2; // infinity_2
  (root->right)->key = inf2;
  (root->left)->key = inf1;
  (root->right)->is_leaf = 1;
  (root->left)->is_leaf = 1;
  return root;
}


int main() {
  node *root = init();
  insert(5, root);
  return 1;
}

