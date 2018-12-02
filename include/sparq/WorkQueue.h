#ifndef SPARQ_WORKQUEUE_H
#define SPARQ_WORKQUEUE_H

#include <chrono>
#include <mutex>
#include <thread>
#include "SafeQ.h"

namespace sparq {
    // A work queue is a thread that reads messages from an input queue
    // and processes them.
    template <class MsgType>
    class WorkQueue {
    public:
        WorkQueue() : msgQ() {
            t1 = new std::thread(&WorkQueue::run, this);
        }

        ~WorkQueue() {
            if (t1 && t1->joinable()) {
                shutdown();
            }
        }

        void push(const MsgType & msg) {
            this->msgQ.push(msg);
        }

        void shutdown() {
            guard.lock();
            quit = true;
            guard.unlock();
            msgQ.signal();
            t1->join();
            t1 = nullptr;
        }

    protected:
        sparq::SafeQ<MsgType> msgQ;
        std::thread *t1;

        virtual void initialize() {}
        virtual void process(const MsgType &t) = 0;

        void run() {
            initialize();
            while (1) {
                MsgType msg{};
                bool didPop = false;
                // If we are shutting down, then add a timeout so that we can
                // stop on the first timeout (i.e. when the input queue is empty).
                if (doQuit()) {
                    const auto when = std::chrono::steady_clock::now() + std::chrono::milliseconds(100);
                    didPop = msgQ.tryPopUntil(&msg, when);
                } else {
                    didPop = msgQ.tryPop(&msg);
                }
                if (doQuit() && !didPop) { return; }
                if (didPop) {
                    process(msg);
                } 
            }
        }

    private:
        std::mutex guard;
        bool quit{false};
        bool doQuit() {
            std::lock_guard<std::mutex> lk(guard);
            return quit;
        }
    };
}

#endif //SPARQ_WORKQUEUE_H
