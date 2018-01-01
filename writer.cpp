//
// Created by Samit Basu on 12/31/17.
//

#include <cstring>
#include <iostream>
#include "IPC/OSPushPullBuffer.h"
#include "PODVariant.h"
#include <type_traits>

struct msg {
    char txt[256];
};

int main (int arg, char *argv[]) {
    using Event = CDSF::PODVariant<msg>;
    std::cout << "is_pod: " << std::is_pod<Event>::value << "\n";
    std::cout << "is_trivially_constructable: " << std::is_trivially_constructible<Event>::value << "\n";
    std::cout << "has_trivial_copy: " << std::is_trivially_copyable<Event>::value << "\n";
    std::cout << "has_trivial_copy: " << std::is_trivially_copyable<std::string>::value << "\n";
    std::cout << "is_standard_layout: " << std::is_standard_layout<Event>::value << "\n";
    SparQ::OSPushPullBuffer<Event, 10> buffer;
    if (!buffer.open("/testbuf2")) return 1;
    Event t;
    msg m;
    if (std::string(argv[1]) == "writer") {
        strcpy(m.txt, argv[2]);
        while (1) {
            sleep(1);
            buffer.push(Event(m));
            std::cout << "Wrote " << m.txt << "\n";
        }
    } else {
        while (1) {
            sleep(1);
            t = buffer.pop();
            std::cout << "Read message\n";// << t.txt << "\n";
        }
    }
    return 0;
}