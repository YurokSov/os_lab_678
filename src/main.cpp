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
    kill_childs();
    return true;
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
        << "3 -- delete subtree with pid\n"
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
            int32_t len = 0;
            int32_t* path = new int32_t[10];
            bool res;
            res = tree.search(key, path, &len);
            cout << "Path to " << key << " is:\n";
            for (int32_t i = 0; i < len; ++i) {
                cout << path[i] << " ";
            }
            cout << "\n";
            delete[] path;
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
        else if (cmd == 6) {
            cin >> key;
            cout << tree.get_parent_pid(key) << "\n";
        }
        else {
            continue;
        }
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (!_LOG_INIT())
        exit(-1);
    if (!init_control_node())
        exit(-2);

    start();
    //return test_avl();

    if (!deinit_control_node())
        exit(-2);
    if (!_LOG_DEINIT())
        exit(-1);
}