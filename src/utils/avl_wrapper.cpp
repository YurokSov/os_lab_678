#include "utils/avl_wrapper.hpp"

extern "C" {
#include "server/mm_control_node.h"
}

#define MAX_PATH_LEN 48

static int buffer[48];

bool init_avl(avl_tree** tree) {
    *tree = new avl_tree();
    return true;
}

bool deinit_avl(avl_tree* tree) {
    delete tree;
    return true;
}

bool add_to_tree(avl_tree* tree, int id) {
    int val = tree->insert(id);
    mm_send_relax();
    return val;
}

bool remove_from_tree(avl_tree* tree, int id) {
    int val = tree->remove(id);
    mm_send_relax();
    return val;
}

int get_parent_id(avl_tree* tree, int id) {
    return tree->get_parent_id(id);
}

void print_tree(struct avl_tree* tree) {
    tree->print();
}

int get_root_id(avl_tree* tree) {
    return tree->get_root_id();
}

void delete_subtree(avl_tree* tree, int* ids, int len) {
    tree->destroy_trees(ids, len);
    return;
}