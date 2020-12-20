#include "utils/avl.hpp"

extern "C" {
#include "server/mm_control_node.h"
#include "utils/logger.h"
}

#include <unistd.h>

void check_and_send_rebind(cptr node, cptr new_parent) {
    if (!node.use_count()) {
        LOG(LL_FATAL, "ASSERTION FAILED!!");
        return;
    }
    if (!new_parent.use_count()) {
        mm_send_rebind(node->id, -1);
    }
    else {
        mm_send_rebind(node->id, new_parent->id);
    }
}

void check_and_send_rebind(cptr node, pptr new_parent) {
    if (!node.use_count()) {
        LOG(LL_FATAL, "ASSERTION FAILED!!");
        return;
    }
    if (!new_parent.use_count()) {
        mm_send_rebind(node->id, -1);
    }
    else {
        mm_send_rebind(node->id, new_parent.lock()->id);
    }
}

int32_t avl_tree::get_root_id() {
    if (_root == nullptr) {
        return -1;
    }
    return _root->id;
}

int32_t avl_tree::get_parent_id(int32_t id) {
    cptr node = _find(id, _root);
    if (node == nullptr || node == _root) {
        return -1;
    }
    return node->parent.lock()->id;
}

void avl_tree::_left_rotate(cptr node) {
    int32_t node_b = node->balance;
    cptr right_son = node->right;
    int32_t rs_b = right_son->balance;
    if (right_son == nullptr) {
        return;
    }
    node->right = right_son->left;
    if (right_son->left != nullptr) {
        check_and_send_rebind(right_son->left, node);
        right_son->left->parent = node;
    }
    check_and_send_rebind(right_son, node->parent);
    right_son->parent = node->parent;
    if (node == _root) {
        _root = right_son;
    }
    else if (node == node->parent.lock()->left) {
        node->parent.lock()->left = right_son;
    }
    else {
        node->parent.lock()->right = right_son;
    }
    right_son->left = node;
    check_and_send_rebind(node, right_son);
    node->parent = right_son;

    node->balance++;
    right_son->balance++;

    if (node_b == -2 && rs_b == -1) {
        node->balance = 0;
        right_son->balance = 0;
    }
    else if (node_b == -2 && rs_b == 0) {
        node->balance = -1;
        right_son->balance = 1;
    }
}

void avl_tree::_right_rotate(cptr node) {
    int32_t node_b = node->balance;
    cptr left_son = node->left;
    int32_t ls_b = left_son->balance;
    if (left_son == nullptr) {
        return;
    }
    node->left = left_son->right;
    if (left_son->right != nullptr) {
        check_and_send_rebind(left_son->right, node);
        left_son->right->parent = node;
    }
    check_and_send_rebind(left_son, node->parent);
    left_son->parent = node->parent;
    if (node == _root) {
        _root = left_son;
    }
    else if (node == node->parent.lock()->right) {
        node->parent.lock()->right = left_son;
    }
    else {
        node->parent.lock()->left = left_son;
    }
    left_son->right = node;
    check_and_send_rebind(node, left_son);
    node->parent = left_son;

    node->balance--;
    left_son->balance--;

    if (node_b == 2 && ls_b == 1) {
        node->balance = 0;
        left_son->balance = 0;
    }
    else if (node_b == 2 && ls_b == 0) {
        node->balance = 1;
        left_son->balance = -1;
    }
}

cptr avl_tree::_rebalance(cptr node) {
    if (node->balance == -2) {
        if (node->right->balance > 0) {
            _right_rotate(node->right);
        }
        _left_rotate(node);
    }
    else {
        if (node->left->balance < 0)
            _left_rotate(node->left);
        _right_rotate(node);
    }
    return node->parent.lock();
}

bool avl_tree::insert(int32_t id) {
    if (_root == nullptr) {
        _root = cptr(new tree_node(id));
        return true;
    }
    else {
        int ret = _insert(id, _root);
        return ret;
    }
}

bool avl_tree::_insert(int32_t id, cptr node) {
    if (id == node->id) {
        return false;
    }
    else if (id < node->id) {
        if (node->left == nullptr) {
            node->left = cptr(new tree_node(id));
            node->left->parent = node;
            _go_up_insert(node, node->left);
            return true;
        }
        else {
            return _insert(id, node->left);
        }
    }
    else {
        if (node->right == nullptr) {
            node->right = cptr(new tree_node(id));
            node->right->parent = node;
            _go_up_insert(node, node->right);
            return true;
        }
        else {
            return _insert(id, node->right);
        }
    }
}

void avl_tree::_go_up_insert(cptr node, cptr prev) {
    if (prev == node->left) {
        node->balance++;
    }
    else {
        node->balance--;
    }
    if (node->balance == 0) {
        return;
    }
    else if (std::abs(node->balance) == 1) {
        if (node == _root) {
            return;
        }
        _go_up_insert(node->parent.lock(), node);
    }
    else {
        node = _rebalance(node);
        if (node->balance == 0 || node == _root) {
            return;
        }
        _go_up_insert(node->parent.lock(), node);
    }
}

void avl_tree::print() {
    _print("", _root, false, 1);
}

void avl_tree::_print(const std::string& prefix, cptr node, bool is_left, int32_t height) {
    if (node != nullptr) {
        std::string new_prefix = "";
        for (int32_t i = 0; i < height; ++i) {
            new_prefix += "    ";
        }
        _print(new_prefix, node->left, true, height + 1);
        std::cout << prefix;
        if (height == 1) {
            std::cout << "───";
        }
        else {
            std::cout << (is_left ? "┌──" : "└──");
        }
        std::cout << node->id << "\n";
        _print(new_prefix, node->right, false, height + 1);
    }
}

bool avl_tree::destroy_trees(int32_t* pids, int32_t len) {

    for (int32_t i = 0; i < len; ++i) {
        int32_t id = pids[i];
        cptr to_delete = _find(id, _root);
        if (to_delete == nullptr) {
            LOG(LL_DEBUG, "assert!");
            continue;
        }
        if (to_delete == _root) {
            _root = nullptr;
        }
        _delete_sub_tree(to_delete);
    }
    if (_root != nullptr)
        _reconstruct();
    return true;
}

cptr avl_tree::_find(int32_t id, cptr node) {
    if (node == nullptr) {
        return nullptr;
    }
    else if (id == node->id) {
        return node;
    }
    else {
        cptr to = id < node->id ? node->left : node->right;
        return _find(id, to);
    }
}

void avl_tree::_delete_sub_tree(cptr node) {
    if (node == nullptr) {
        return;
    }
    else {
        if (!node->parent.lock().use_count()) {
            LOG(LL_WARNING, "ASSERT: %d", node->id);
        }
        else {
            if (node->parent.lock()->left == node) {
                node->parent.lock()->left = nullptr;
            }
            else {
                node->parent.lock()->right = nullptr;
            }
            node->parent.lock() = nullptr;
        }
    }
}

void avl_tree::_reconstruct() {
    cptr old_root = _root;
    _root = cptr(new tree_node(old_root->id));
    _rec_reconstruct(_root, old_root->left);
    _rec_reconstruct(_root, old_root->right);
}

void avl_tree::_rec_reconstruct(cptr& new_root, cptr node) {
    if (node == nullptr) {
        return;
    }
    _insert(node->id, new_root);
    _rec_reconstruct(new_root, node->left);
    _rec_reconstruct(new_root, node->right);
}

bool avl_tree::remove(int32_t id) {
    cptr node = _root;
    while ((node != nullptr) && (id != node->id)) {
        cptr to = (id < node->id) ? node->left : node->right;
        node = to;
    }
    if (node == nullptr) {
        return false;
    }
    _remove(node);
    return true;
}

void avl_tree::_remove(cptr node) {
    cptr to_delete = node;
    int32_t to_delete_balance = to_delete->balance;
    cptr to_replace;
    cptr to_replace_parent;
    if (node->left == nullptr) {
        to_replace = node->right;
        if (to_replace != nullptr) {
            check_and_send_rebind(to_replace, node->parent);
            to_replace->parent = node->parent;
            to_replace_parent = node->parent.lock();
        }
        else {
            to_replace_parent = node->parent.lock();
            if (node == _root) {
                to_replace_parent = nullptr;
                _root = nullptr;
            }
        }
        if (_root != nullptr) {
            if (node != _root) {
                if (node->parent.lock()->left == node) {
                    node->parent.lock()->left = to_replace;
                }
                else {
                    node->parent.lock()->right = to_replace;
                }
            }
            else {
                _root = to_replace;
            }
        }
    }
    else if (node->right == nullptr) {
        to_replace = node->left;
        check_and_send_rebind(to_replace, node->parent);
        to_replace->parent = node->parent;
        to_replace_parent = node->parent.lock();
        if (node != _root) {
            if (node->parent.lock()->left == node) {
                node->parent.lock()->left = to_replace;
            }
            else {
                node->parent.lock()->right = to_replace;
            }
        }
        else {
            _root = to_replace;
        }
    }
    else {
        cptr min_in_right = node->right;
        while (min_in_right->left != nullptr) {
            min_in_right = min_in_right->left;
        }
        to_delete = min_in_right;
        to_delete_balance = to_delete->balance;
        to_replace = to_delete->right;
        if (to_delete->parent.lock() == node) {
            if (to_replace != nullptr) {
                check_and_send_rebind(to_replace, to_delete);
                to_replace->parent = to_delete;
            }
            to_replace_parent = to_delete;
        }
        else {
            to_delete->parent.lock()->left = to_replace;
            if (to_replace != nullptr) {
                check_and_send_rebind(to_replace, to_delete->parent);
                to_replace->parent = to_delete->parent;
            }
            to_replace_parent = to_delete->parent.lock();
            to_delete->right = node->right;
            check_and_send_rebind(to_delete->right, to_delete);
            to_delete->right->parent = to_delete;
        }
        if (node != _root) {
            if (node->parent.lock()->left == node) {
                node->parent.lock()->left = to_delete;
            }
            else {
                node->parent.lock()->right = to_delete;
            }
        }
        else {
            _root = to_delete;
        }
        check_and_send_rebind(to_delete, node->parent);
        to_delete->parent = node->parent;
        to_delete->left = node->left;

        check_and_send_rebind(to_delete->left, to_delete);
        to_delete->left->parent = to_delete;
        to_delete->balance = node->balance;
    }
    if (to_replace_parent != nullptr) {
        _go_up_remove(to_replace_parent, to_replace);
    }
}

void avl_tree::_go_up_remove(cptr node, cptr prev) {
    if (node->left == nullptr && node->right == nullptr) {
        node->balance = 0;
    }
    else {
        if (prev == node->left) {
            node->balance--;
        }
        else {
            node->balance++;
        }
    }
    if (std::abs(node->balance) == 1) {
        return;
    }
    else if (node->balance == 0) {
        if (node == _root) {
            return;
        }
        _go_up_remove(node->parent.lock(), node);
    }
    else {
        node = _rebalance(node);
        if (node->balance == 0 || node == _root) {
            return;
        }
        _go_up_remove(node->parent.lock(), node);
    }
}