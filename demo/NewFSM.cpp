//
// Created by Samit Basu on 12/29/17.
//

#include "NewFSM.h"

namespace sparq {
    namespace FooBar {

        predefineState(FooBarFSM, Running);

        defineState(FooBarFSM, Idle) {
            int64_t id;

            onEntry() {
                myState.publish("idle");
                std::cout << "+Idle\n";
                id = addTimer(10*TimeQ::Seconds, Quit());
            }

            onEvent(Start) {
                transit<Running>();
            }

            onExit() {
                std::cout << "-Idle\n";
                cancelTimer(id);
            }
        };

        defineState(FooBarFSM, Running) {
            int64_t id;

            onEntry() {
                myState.publish("run");
                std::cout << "+Run\n";
                id = addTimer(5000000, Tic());
                std::cout << "Added timer, ID = " << id << "\n";
            }

            onEvent(Stop) {
                transit<Idle>();
            }

            onEvent(Tic) {
                std::cout << "Tic\n";
                id = addTimer(100000, Stop());
            }

            onExit() {
                auto res = cancelTimer(id);
                std::cout << "Cancel timer " << id << " res = " << res << "\n";
                std::cout << "-Run\n";
            }
        };
    }

    PubSub<std::string> FooBar::FooBarFSM::myState;

    AFSM_INITIAL_STATE(FooBar::FooBarFSM, FooBar::Event, FooBar::Idle);
}