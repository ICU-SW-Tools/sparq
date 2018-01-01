//
// Created by Samit Basu on 12/17/17.
//

#ifndef OLYMPUS_MC_PUBSUB_H_H
#define OLYMPUS_MC_PUBSUB_H_H

#include <set>
#include <thread>
#include <map>
#include <iostream>
#include "SafeQ.h"

namespace CDSF {
    // A broadcasting thread safe pub-sub mechanism
    template <class T, bool keepLast = false>
    class PubSub {
    public:
        using callback = std::function<void(const T& msg)>;
        PubSub() : msgQ(), lock(), listeners(), cv() {
            std::thread t1(&PubSub::run, this);
            t1.detach();
        }

        // Executes in the thread of the caller - will not block
        void publish(const T& msg) {
            this->msgQ.push(msg);
        }

        void subscribe(const std::string& name, callback c) {
            std::lock_guard<std::mutex> guard(lock);
            this->listeners[name] = c;
        }

        void unsubscribe(const std::string &name) {
            std::lock_guard<std::mutex> guard(lock);
            this->listeners.erase(name);
        }

        void stop() {
            std::unique_lock<std::mutex> guard(lock);
            this->quit = true;
            this->msgQ.signal();
            cv.wait(guard);
        }

        T last() {
            static_assert(keepLast, "PubSub was not created with support for last element retrieval");
            std::lock_guard<std::mutex> guard(lock);
            return lastMsg;
        }

    private:
        SafeQ<T> msgQ;
        std::mutex lock;
        std::map<std::string,callback> listeners;
        std::condition_variable cv;
        T lastMsg;
        bool quit = false;
        void run() {
            while (!quit) {
                T obj;
                auto msgAvail = this->msgQ.tryPop(&obj);
                std::cout << "Msg Avail: " << obj << " flag: " << msgAvail << "\n";
                if (msgAvail) {
                    for (auto t : this->listeners) {
                        std::cout << "Sending msg " << obj << " to listener " << t.first << "\n";
                        t.second(obj);
                    }
                    if (keepLast) {
                        this->lastMsg = obj;
                    }
                }
            }
            std::cout << "Shutting down pubsub";
            listeners.clear();
            cv.notify_all();
        }
    };
}

#endif //OLYMPUS_MC_PUBSUB_H_H
