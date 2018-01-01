//
// Created by Samit Basu on 12/17/17.
//

#ifndef OLYMPUS_MC_ACTIVEOBJECT_H
#define OLYMPUS_MC_ACTIVEOBJECT_H

#include <thread>
#include "SafeQ.h"
#include "TimeQ.h"

namespace CDSF {
    // An active object is a thread that owns it's resources.
    // Communication can only occur through the message queue.
    // This pattern allows us to write single-threaded code inside
    // a multi-threaded environment.

    template <class MsgType>
    class ActiveObject {
    public:
        ActiveObject() : cmdQueue() {
            t1 = new std::thread(&ActiveObject::run, this);
        }

        void join() {
            t1->join();
        }

        void push(const MsgType & msg) {
            this->cmdQueue.push(msg);
        }

    protected:
        SafeQ<MsgType> cmdQueue;
        TimeQ timers;
        std::thread *t1;

        virtual void initialize() {}
        virtual bool process(const MsgType &t) = 0;

        // No lock is required here - only the MsgHandler class can manipulate
        // timers, and it runs in the same thread as the AO
        int64_t addTimer(uint64_t micros, const callback_t &callback) {
            return this->timers.add(mkTimeStamp(micros, callback));
        }

        // This version allows you to post a command at the given time
        int64_t addTimer(uint64_t micros, const MsgType &msg) {
            auto ret =  this->timers.add(mkTimeStamp(micros, [=]() {
                this->push(msg);
            }));
            std::cout << "Add timer " << ret << "\n";
            return ret;
        }

        bool cancelTimer(int64_t id) {
            std::cout << "Cancel timer " << id << "\n";
            return this->timers.cancel(id);
        }

        void run() {
            initialize();
            bool quit = false;
            while (!quit) {
                if (timers.empty()) {
                    std::cout << "Thread blocked forever\n";
                    quit = !this->process(cmdQueue.pop());
                } else {
                    MsgType obj;
                    std::cout << "Thread waiting on msg/timers\n";
                    bool msg = cmdQueue.tryPopUntil(&obj,timers.next());
                    std::cout << "Processing timers\n";
                    timers.update();
                    if (msg)
                        quit = !this->process(obj);
                }
            }
            std::cout << "Quit encountered...\n";
        }
    };
}


#endif //OLYMPUS_MC_ACTIVEOBJECT_H
