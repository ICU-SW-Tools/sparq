//
// Created by Samit Basu on 12/28/17.
//

#ifndef OLYMPUS_MC_NEWFSM_H
#define OLYMPUS_MC_NEWFSM_H

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

        typedef PODVariant<Start, Stop, Quit, Tic> Event;

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


#endif //OLYMPUS_MC_NEWFSM_H
