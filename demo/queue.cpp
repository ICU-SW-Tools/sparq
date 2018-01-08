//
// Created by sbasu on 1/3/18.
//

#include <cstring>
#include <iostream>
#include <sparq/IPC/OSMessageQ.h>
#include <unistd.h>

struct msg {
    char txt[256];
};

int main (int arg, char *argv[]) {
    sparq::OSMessageQ<msg,8> buffer;
    if (!buffer.open("/testbuf3")) {
        std::cerr << "Unable to open messageQ\n";
        perror("Error:");
        return 1;
    }
    msg m;
    if (std::string(argv[1]) == "writer") {
        strcpy(m.txt, argv[2]);
        while (1) {
            sleep(1);
            auto flag = buffer.tryPush(m, 10000);
            if (flag) {
                std::cout << "Wrote " << m.txt << "\n";
            } else {
                std::cout << "Failed!\n";
            }
        }
    } else {
        while (1) {
            sleep(1);
            auto t = buffer.pop();
            std::cout << "Read message " << t.txt << "\n";
        }
    }
    return 0;
}