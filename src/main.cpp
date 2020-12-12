#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/syscall.h>
#include <zmq.h>

extern "C" {

#include "defines.h"
#include "logger.h"
#include "parsing.h"
#include "control_node.h"

}

#include "avl.hpp"
#include <iostream>


int main_repl() {
    parse_status status = pse_ok;
    command_u cmd_info;
    void* result;

    cmd_enum cmd;
    while (status == pse_ok && parse_command(&cmd) == pse_ok) {
        get_cmd_info(&cmd, &cmd_info);
        execute_cmd(&cmd, &cmd_info, &result);
        //print_result(result);
    }
    LOG(LL_NOTE, "!!!REPL END!!!");
    kill_childs();
    return true;
}

void __attribute__((constructor)) init() {
    if (!_LOG_INIT())
        exit(-1);
}

void __attribute((destructor)) fini() {
    if (!_LOG_DEINIT())
        exit(-1);
}

int start() {

    main_repl();

    return EXIT_SUCCESS;
}

int test_avl() {
    using namespace std;
    avl_tree tree;
    int key, cmd;
    cout << "1 -- add pid\n"
        << "2 -- print path to pid\n"
        << "3 -- delete subtree with path\n"
        << "4 -- print tree\n"
        << "5 -- delete pid\n";
    while (true) {
        if (!(cin >> cmd)) {
            break;
        }
        if (cmd == 1) {
            cin >> key;
            tree.insert(key);
        }
        else if (cmd == 2) {
            cin >> key;
            vector<int> path;
            path = tree.search(key);
            cout << "Path to " << key << " is:\n";
            for (auto p : path) {
                cout << p << " ";
            }
            cout << "\n";
        }
        else if (cmd == 3) {
            cin >> key;
            tree.delete_sub_tree(key);
        }
        else if (cmd == 4) {
            tree.print();
        }
        else if (cmd == 5) {
            cin >> key;
            tree.remove(key);
        }
        else {
            continue;
        }
    }
    return 0;
}

int main(int argc, char* argv[]) {

    //return start();
    return test_avl();
}