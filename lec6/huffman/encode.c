#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include "encode.h"

#define NSYMBOLS 257

static int count[NSYMBOLS];

struct node
{
  int symbol;
  int count;
  struct node *left;
  struct node *right;
};

typedef struct node Node;

void count_symbols(const char *filename)
{
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) {
    fprintf(stderr, "error: cannot open %s\n", filename);
    exit(1);
  }
  
  int i;
  for (i = 0; i < NSYMBOLS; i++) {
    count[i] = rand() % 100;
  }

  count[NSYMBOLS-1]++; // End of File

  fclose(fp);
}

Node* pop_min(int *n, Node *nodes[])
{
  // find a node with the smallest count
  int i, j = 0;
  for (i = 0; i < *n; i++) {
    if (nodes[i]->count < nodes[j]->count) {
      j = i;
    }
  }

  Node *node_min = nodes[j];

  // remove the node from nodes[]
  for (i = j; i < (*n) - 1; i++) {
    nodes[i] = nodes[i + 1];
  }
  (*n)--;

  return node_min;
}

Node* build_tree()
{
  int i, n = 0;
  Node *nodes[NSYMBOLS];

  // create leaf nodes
  for (i = 0; i < NSYMBOLS; i++) {
    if (count[i] == 0) continue;
    nodes[n] = (Node*)malloc(sizeof(Node));
    nodes[n]->symbol = i;
    nodes[n]->count  = count[i];
    nodes[n]->left   = NULL;
    nodes[n]->right  = NULL;
    n++;
  }

  while (n >= 2) {
    Node *node1 = pop_min(&n, nodes);
    Node *node2 = pop_min(&n, nodes);

    // create a new node
    
  }

  return nodes[0];
}

// perform depth-first traversal of the tree
void traverse_tree(const int depth, const Node *np)
{
  assert(depth < NSYMBOLS);

  if (np->left == NULL) {
    return;
  }

  traverse_tree(depth + 1, np->left);
  traverse_tree(depth + 1, np->right);
}

int encode(const char *filename)
{
  count_symbols(filename);
  Node *root = build_tree();
  traverse_tree(0, root);

  return 1;
}
