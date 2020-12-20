#ifndef INCLUDE_AVL_HPP
#define INCLUDE_AVL_HPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <memory>
#include <cstdint>

struct tree_node;

using pptr = std::weak_ptr<tree_node>;
using cptr = std::shared_ptr<tree_node>;

struct tree_node {
    int32_t id;
    int32_t balance;
    pptr parent;
    cptr left;
    cptr right;

    tree_node() : id(-1), balance(0), left(nullptr), right(nullptr) {
        parent.lock() = nullptr;
    }
    tree_node(int32_t id) : id(id), balance(0), left(nullptr), right(nullptr) {
        parent.lock() = nullptr;
    }
    ~tree_node() = default;
};

class avl_tree {
private:
    cptr _root;

    cptr _find(int32_t id, cptr node);
    bool _insert(int32_t id, cptr node);
    void _go_up_insert(cptr node, cptr prev);

    void _remove(cptr node);
    void _go_up_remove(cptr node, cptr prev);

    void _left_rotate(cptr node);
    void _right_rotate(cptr node);
    cptr _rebalance(cptr node);

    void _delete_sub_tree(cptr node);
    void _rec_reconstruct(cptr& new_root, cptr node);
    void _reconstruct();

    void _print(const std::string& prefix, cptr node, bool is_left, int32_t height);

public:
    avl_tree() : _root(nullptr) {};

    bool insert(int32_t id);
    void print();
    bool destroy_trees(int32_t* pids, int32_t len);
    bool remove(int32_t id);
    int32_t get_parent_id(int32_t id);
    int32_t get_root_id();
    ~avl_tree() = default;
};

#endif