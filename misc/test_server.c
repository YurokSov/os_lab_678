// server
#include <stdio.h>
#include <unistd.h>
#include <zmq.h>
#include <assert.h>

int main() {
    void *context = zmq_ctx_new();
    void *resp = zmq_socket(context, ZMQ_REP);
    int rc = zmq_bind(resp,"tcp://*:5555");
    assert(rc == 0); 
    while (1) {
        char buf[10];
        zmq_recv(resp, buf, 10, 0);
        printf("Recieved Hello\n");
        sleep(1);
        zmq_send(resp, "World", 5, 0);
    }
    return 0;
}