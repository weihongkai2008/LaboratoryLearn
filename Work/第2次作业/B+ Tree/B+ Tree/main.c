//
//  main.c
//  B+ Tree
//
//  Created by 魏宏凯 on 15/8/5.
//  Copyright © 2015年 魏宏凯. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define ORDERS 15

typedef struct node {
    void **node_ptrs;
    char **keys;
    struct node *parent;
    int num_keys;
    bool is_leaf;
    struct node *next;
} node;

typedef struct datarecord {
    int value;
} datarecord;

int size = 3;


void print_tree(node *root);
datarecord *find(node *root, char *key);
node *find_leaf(node *root, char *key);

datarecord *create_datarecord(int value);
node *create_node();
node *create_leaf();
node *create_tree(char *key, datarecord *rec);
node *create_root(node *left, node *right, char *key);
node *insert(node *root, char *key, int value);
node *insert_into_parent(node *root, node *left, node *right, char *key);
void insert_node(node *nd, node *right, int index, char *key);
node *insert_node_splitting(node *root, node *nd, node *right,
                                       int index, char *key);
node *insert_leaf_splitting(node *root, node *leaf, int index,
                                       char *key, datarecord *rec);
void insert_leaf(node *leaf, int index, char *key, datarecord *rec);

void free_node(node *nd);
node *thorw_path(node *nd, int index);
node *delete(node *root, char *key);
node *delete_path(node *root, node *nd, int index);
node *identity_root(node *root);
int get_node_index(node *nd);
node *merge_nodes(node *root, node *nd, node *neighbor, int nd_index);
void split_nodes(node *nd, node *neighbor, int nd_index);

void print_tree(node *root)
{
    node *p, *p_down;
    int i;
    if (root == NULL){
        printf("Empty tree!\n");
        return;
    }
    p = root;
    p_down = root;
    while (!p->is_leaf){
        for (i = 0; i < p->num_keys; i++)
            printf("%s ", p->keys[i]);
        printf("\t");
        p = p->next;
        if (!p){
            p_down = p_down->node_ptrs[0];
            p = p_down;
            printf("\n");
        }
    }
    
    while (p){
        for (i = 0; i < p->num_keys; i++)
            printf("%s ", p->keys[i]);
        printf("\t");
        p = p->node_ptrs[size-1];
    }
    printf("\n");
}

datarecord *find(node *root, char *key)
{
    node *leaf;
    int i;
    leaf = find_leaf(root, key);
    if (leaf == NULL)
        return NULL;
    for (i = 0; i < leaf->num_keys && strcmp(leaf->keys[i], key) != 0; i++)
        ;
    if (i == leaf->num_keys)
        return NULL;
    return (datarecord *)leaf->node_ptrs[i];
}

node *find_leaf(node *root, char *key)
{
    node *nd;
    int i;
    if (root == NULL)
        return root;
    nd = root;
    while (!nd->is_leaf){
        for (i = 0; i < nd->num_keys && strcmp(nd->keys[i], key) <= 0; i++)
            ;
        nd = (node *)nd->node_ptrs[i];
    }
    return nd;
}

datarecord *create_datarecord(int value)
{
    datarecord *rec;
    rec = (datarecord *)malloc(sizeof(datarecord));
    rec->value = value;
    return rec;
}

node *create_node()
{
    node *nd;
    nd = (node *)malloc(sizeof(node));
    nd->node_ptrs = malloc(size * sizeof(void *));
    nd->keys = malloc((size - 1) * sizeof(char *));
    nd->parent = NULL;
    nd->num_keys = 0;
    nd->is_leaf = false;
    nd->next = NULL;
    return nd;
}

node *create_leaf()
{
    node *leaf;
    leaf = create_node();
    leaf->is_leaf = true;
    return leaf;
}

node *create_tree(char *key, datarecord *rec)
{
    node *root;
    root = create_leaf();
    root->node_ptrs[0] = rec;
    root->keys[0] = malloc(ORDERS);
    strcpy(root->keys[0], key);
    root->node_ptrs[size-1] = NULL;
    root->num_keys++;
    return root;
}

node *create_root(node *left, node *right, char *key)
{
    node *root;
    root = create_node();
    root->node_ptrs[0] = left;
    root->node_ptrs[1] = right;
    root->keys[0] = malloc(ORDERS);
    strcpy(root->keys[0], key);
    root->num_keys++;
    left->parent = root;
    right->parent = root;
    return root;
}

node *insert(node *root, char *key, int value)
{
    datarecord *rec;
    node *leaf;
    int index, cond = 0;
    leaf = find_leaf(root, key);
    if (!leaf){
        rec = create_datarecord(value);
        return create_tree(key, rec);
    }
    for (index = 0; index < leaf->num_keys && (cond = strcmp(leaf->keys[index], key)) < 0; index++)
        ;
    if (cond == 0)
        return root;
    rec = create_datarecord(value);
    if (leaf->num_keys < size - 1){
        insert_leaf(leaf, index, key, rec);
        return root;
    }
    return insert_leaf_splitting(root, leaf, index, key, rec);
}

node *insert_into_parent(node *root, node *left, node *right, char *key)
{
    node *parent;
    int index;
    parent = left->parent;
    
    if (parent == NULL){
        return create_root(left, right, key);
    }
    
    for (index = 0; index < parent->num_keys && parent->node_ptrs[index] != left; index++);
    ;
    if (parent->num_keys < size - 1){
        insert_node(parent, right, index, key);
        return root;
    }
    return insert_node_splitting(root, parent, right, index, key);
}

void insert_node(node *nd, node *right, int index, char *key)
{
    int i;
    for (i = nd->num_keys; i > index; i--){
        nd->keys[i] = nd->keys[i-1];
        nd->node_ptrs[i+1] = nd->node_ptrs[i];
    }
    nd->keys[index] = malloc(ORDERS);
    strcpy(nd->keys[index], key);
    nd->node_ptrs[index+1] = right;
    nd->num_keys++;
}

node *insert_node_splitting(node *root, node *nd, node *right, int index, char *key)
{
    int i, split;
    node **temp_ps, *new_nd, *child;
    char **temp_ks, *new_key;
    temp_ps = malloc((size + 1) * sizeof(node *));
    temp_ks = malloc(size * sizeof(char *));
    
    for (i = 0; i < size + 1; i++){
        if (i == index + 1)
            temp_ps[i] = right;
        else if (i < index + 1)
            temp_ps[i] = nd->node_ptrs[i];
        else
            temp_ps[i] = nd->node_ptrs[i-1];
    }
    for (i = 0; i < size; i++){
        if (i == index){
            temp_ks[i] = malloc(ORDERS);
            strcpy(temp_ks[i], key);
        }
        else if (i < index)
            temp_ks[i] = nd->keys[i];
        else
            temp_ks[i] = nd->keys[i-1];
    }
    
    
    split = size % 2 ? size / 2 + 1 : size / 2;
    nd->num_keys = split - 1;
    for (i = 0; i < split - 1; i++){
        nd->node_ptrs[i] = temp_ps[i];
        nd->keys[i] = temp_ks[i];
    }
    nd->node_ptrs[i] = temp_ps[i];
    new_key = temp_ks[split - 1];
    
    new_nd = create_node();
    new_nd->num_keys = size - split;
    for (++i; i < size; i++){
        new_nd->node_ptrs[i - split] = temp_ps[i];
        new_nd->keys[i - split] = temp_ks[i];
    }
    new_nd->node_ptrs[i - split] = temp_ps[i];
    new_nd->parent = nd->parent;
    for (i = 0; i <= new_nd->num_keys; i++){
        child = (node *)(new_nd->node_ptrs[i]);
        child->parent = new_nd;
    }
    new_nd->next = nd->next;
    nd->next = new_nd;
    
    free(temp_ps);
    free(temp_ks);
    return insert_into_parent(root, nd, new_nd, new_key);
}

void insert_leaf(node *leaf, int index, char *key, datarecord *rec)
{
    int i;
    for (i = leaf->num_keys; i > index; i--){
        leaf->keys[i] = leaf->keys[i-1];
        leaf->node_ptrs[i] = leaf->node_ptrs[i-1];
    }
    leaf->keys[index] = malloc(ORDERS);
    strcpy(leaf->keys[index], key);
    leaf->node_ptrs[index] = rec;
    leaf->num_keys++;
}

node *insert_leaf_splitting(node *root, node *leaf, int index, char *key, datarecord *rec)
{
    node *new_leaf;
    datarecord **temp_ps;
    char **temp_ks, *new_key;
    int i, split;
    
    temp_ps = malloc(size * sizeof(datarecord *));
    temp_ks = malloc(size * sizeof(char *));
    for (i = 0; i < size; i++){
        if (i == index){
            temp_ps[i] = rec;
            temp_ks[i] = malloc(ORDERS);
            strcpy(temp_ks[i], key);
        }
        else if (i < index){
            temp_ps[i] = leaf->node_ptrs[i];
            temp_ks[i] = leaf->keys[i];
        }
        else{
            temp_ps[i] = leaf->node_ptrs[i-1];
            temp_ks[i] = leaf->keys[i-1];
        }
    }
    
    split = size / 2;
    leaf->num_keys = split;
    for (i = 0; i < split; i++){
        leaf->node_ptrs[i] = temp_ps[i];
        leaf->keys[i] = temp_ks[i];
    }
    
    new_leaf = create_leaf();
    new_leaf->num_keys = size - split;
    for (; i < size; i++){
        new_leaf->node_ptrs[i - split] = temp_ps[i];
        new_leaf->keys[i - split] = temp_ks[i];
    }
    
    new_leaf->parent = leaf->parent;
    new_leaf->node_ptrs[size - 1] = leaf->node_ptrs[size - 1];
    leaf->node_ptrs[size - 1] = new_leaf;
    free(temp_ps);
    free(temp_ks);
    new_key = new_leaf->keys[0];
    return insert_into_parent(root, leaf, new_leaf, new_key);
}

node *delete(node *root, char *key)
{
    node *leaf;
    datarecord *rec;
    int i;
    leaf = find_leaf(root, key);
    if (leaf == NULL)
        return root;
    for (i = 0; i < leaf->num_keys && strcmp(leaf->keys[i], key) != 0; i++)
        ;
    if (i == leaf->num_keys)
        return root;
    rec = (datarecord *)leaf->node_ptrs[i];
    root = delete_path(root, leaf, i);
    return root;
}

node *delete_path(node *root, node *nd, int index)
{
    int min_keys, cap, nd_index;
    node *neighbor;
    
    thorw_path(nd, index);
    if (nd == root)
        return identity_root(nd);
    min_keys = nd->is_leaf ? size / 2 : (size - 1) / 2;
    if (nd->num_keys >= min_keys) {
        return root;
    }
    
    nd_index = get_node_index(nd);
    if (nd_index == 0)
        neighbor = nd->parent->node_ptrs[1];
    else
        neighbor = nd->parent->node_ptrs[nd_index - 1];
    
    cap = nd->is_leaf ? size - 1 : size - 2;
    if (neighbor->num_keys + nd->num_keys <= cap)
        return merge_nodes(root, nd, neighbor, nd_index);
    
    split_nodes(nd, neighbor, nd_index);
    return root;
}

void split_nodes(node *nd, node *neighbor, int nd_index)
{
    int i;
    node *tmp;
    if (nd_index != 0) {
        if (!nd->is_leaf)
            nd->node_ptrs[nd->num_keys + 1] = nd->node_ptrs[nd->num_keys];
        for (i = nd->num_keys; i > 0; i--){
            nd->keys[i] = nd->keys[i - 1];
            nd->node_ptrs[i] = nd->node_ptrs[i - 1];
        }
        if (!nd->is_leaf){
            nd->keys[0] = nd->parent->keys[nd_index - 1];
            
            nd->node_ptrs[0] = neighbor->node_ptrs[neighbor->num_keys];
            tmp = (node *)nd->node_ptrs[0];
            tmp->parent = nd;
            neighbor->node_ptrs[neighbor->num_keys] = NULL;
            
            nd->parent->keys[nd_index - 1] = neighbor->keys[neighbor->num_keys - 1];
            neighbor->keys[neighbor->num_keys - 1] = NULL;
        }
        else {
            nd->keys[0] = neighbor->keys[neighbor->num_keys - 1];
            neighbor->keys[neighbor->num_keys - 1] = NULL;
            
            nd->node_ptrs[0] = neighbor->node_ptrs[neighbor->num_keys - 1];
            neighbor->node_ptrs[neighbor->num_keys - 1] = NULL;
            strcpy(nd->parent->keys[nd_index - 1], nd->keys[0]);
        }
    }
    else {
        if (!nd->is_leaf){
            nd->keys[nd->num_keys] = nd->parent->keys[0];
            nd->node_ptrs[nd->num_keys + 1] = neighbor->node_ptrs[0];
            tmp = (node *)nd->node_ptrs[nd->num_keys + 1];
            tmp->parent = nd;
            nd->parent->keys[0] = neighbor->keys[0];
        }
        else {
            nd->keys[nd->num_keys] = neighbor->keys[0];
            nd->node_ptrs[nd->num_keys] = neighbor->node_ptrs[0];
            strcpy(nd->parent->keys[0], neighbor->keys[1]);
        }
        for (i = 0; i < neighbor->num_keys - 1; i++){
            neighbor->keys[i] = neighbor->keys[i + 1];
            neighbor->node_ptrs[i] = neighbor->node_ptrs[i + 1];
        }
        neighbor->keys[i] = NULL;
        if (!nd->is_leaf)
            neighbor->node_ptrs[i] = neighbor->node_ptrs[i + 1];
        else
            neighbor->node_ptrs[i] = NULL;
    }
    
    neighbor->num_keys--;
    nd->num_keys++;
    
}

node *merge_nodes(node *root, node *nd, node *neighbor, int nd_index)
{
    int i, j, start;
    node *tmp, *parent;
    
    if (nd_index == 0) {
        tmp = nd;
        nd = neighbor;
        neighbor = tmp;
        nd_index = 1;
    }
    parent = nd->parent;
    
    start = neighbor->num_keys;
    if (nd->is_leaf){
        for (i = start, j = 0; j < nd->num_keys; i++, j++){
            neighbor->keys[i] = nd->keys[j];
            neighbor->node_ptrs[i] = nd->node_ptrs[j];
            nd->keys[j] = NULL;
            nd->node_ptrs[j] = NULL;
        }
        neighbor->num_keys += nd->num_keys;
        neighbor->node_ptrs[size - 1] = nd->node_ptrs[size - 1];
    }
    else {
        neighbor->keys[start] = malloc(ORDERS);
        strcpy(neighbor->keys[start], parent->keys[nd_index - 1]);
        for (i = start + 1, j = 0; j < nd->num_keys; i++, j++){
            neighbor->keys[i] = nd->keys[j];
            neighbor->node_ptrs[i] = nd->node_ptrs[j];
        }
        neighbor->node_ptrs[i] = nd->node_ptrs[j];
        neighbor->num_keys += nd->num_keys + 1;
        neighbor->next = nd->next;
        
        for (i = 0; i <= neighbor->num_keys; i++){
            tmp = (node *)neighbor->node_ptrs[i];
            tmp->parent = neighbor;
        }
    }
    free_node(nd);
    return delete_path(root, parent, nd_index);
}

int get_node_index(node *nd)
{
    node *parent;
    int i;
    parent = nd->parent;
    for (i = 0; i < parent->num_keys && parent->node_ptrs[i] != nd; i++)
        ;
    return i;
}

void free_node(node *nd)
{
    free(nd->keys);
    free(nd->node_ptrs);
    free(nd);
}

node *identity_root(node *root)
{
    node *new_root;
    if (root->num_keys > 0)
        return root;
    if (!root->is_leaf){
        new_root = root->node_ptrs[0];
        new_root->parent = NULL;
    }
    else
        new_root = NULL;
    free_node(root);
    return new_root;
}

node *thorw_path(node *nd, int index)
{
    int i, index_k;
    
    if (nd->is_leaf){
        free(nd->keys[index]);
        free(nd->node_ptrs[index]);
        for (i = index; i < nd->num_keys - 1; i++){
            nd->keys[i] = nd->keys[i + 1];
            nd->node_ptrs[i] = nd->node_ptrs[i + 1];
        }
        nd->keys[i] = NULL;
        nd->node_ptrs[i] = NULL;
    }
    else{
        index_k = index - 1;
        free(nd->keys[index_k]);
        for (i = index_k; i < nd->num_keys - 1; i++){
            nd->keys[i] = nd->keys[i + 1];
            nd->node_ptrs[i + 1] = nd->node_ptrs[i + 2];
        }
        nd->keys[i] = NULL;
        nd->node_ptrs[i + 1] = NULL;
    }
    nd->num_keys--;
    return nd;
}


int main()
{
    node *root = NULL;
    char *terms[] = {"a", "b", "c", "d", "e", "f", "g",
        "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r",
        "s", "t", "u", "v", "w", "x", "y", "z" };
    int i;
    size = 4;
    for (i = 0; i < 26; i++){
        root = insert(root, terms[i], i + 1000);
    }
    print_tree(root);
    return 0;
}