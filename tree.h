typedef struct info info;
typedef struct node node;
int state(info *pointer);
void CAS_child(node *parent, node *old, node *new);
void help_marked(info *op);

