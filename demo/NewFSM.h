//
// Created by Samit Basu on 12/28/17.
//

#ifndef SPARQ_NEWFSM_H
#define SPARQ_NEWFSM_H

#include <string>
#include "../include/sparq/ActiveFSM.h"
#include "../include/sparq/PODVariant.h"
#include "../include/sparq/PubSub.h"

namespace sparq {

    namespace FooBar {

        struct Start {};

        struct Stop {};

        struct Quit {};

        struct Tic {};

        using Event = EventT<Start, Stop, Quit, Tic>;

        // This is the finite state machine - simple start stop
        defineActiveFSM(FooBarFSM, Event) {
        public:
            static PubSub<std::string> myState;
            defaultEventHandler(Start) {
                std::cout << "Base Object got event:" << typeid(event).name() << "\n";
            }
            defaultEventHandler(Stop) {
                std::cout << "Base Object got event:" << typeid(event).name() << "\n";
            }
            defaultEventHandler(Quit) {
                std::cout << "Shutting down FSM\n";
                signal_quit();
            }
            defaultEventHandler(Tic) {
                std::cout << "Base Object got event:" << typeid(event).name() << "\n";
            }
        };

    }
}


#endif //SPARQ_NEWFSM_H
