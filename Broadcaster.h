//
// Created by Samit Basu on 12/29/17.
//

#ifndef OLYMPUS_MC_BROADCASTER_H
#define OLYMPUS_MC_BROADCASTER_H

#include <thread>
#include <map>

namespace CDSF {
    template <class T, class Func>
    class Broadcaster {
    public:
        using callback = std::function<void(const T& msg)>;
        Broadcaster() {
            t1 = new std::thread(&Broadcaster::run, this);
        }

        void join() {
            t1->join();
        }

        void quit() {
            // TODO - address the issue that the thread can be blocked
            // forever, waiting on a message to arrive, while we signal a quit.
            this->quitflag = true;
        }

        void subscribe(const std::string &name, callback c) {
            std::lock_guard<std::mutex> guard(lock);
            this->listeners[name] = c;
        }

        void unsubscribe(const std::string &name) {
            std::lock_guard<std::mutex> guard(lock);
            this->listeners.erase(name);
        }

    protected:
        void run() {
            while (!this->quitflag) {
                T obj = Func();
                {
                    std::lock_guard<std::mutex> guard(lock);
                    for (const auto &p : listeners) {
                        if (p.second) {
                            p.second(obj);
                        }
                    }
                }
            }
        }

    private:
        std::mutex lock;
        std::map<std::string,callback> listeners;
        std::thread *t1 = nullptr;
        bool quitflag = false;
    };
}

#endif //OLYMPUS_MC_BROADCASTER_H
