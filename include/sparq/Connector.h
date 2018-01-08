//
// Created by sbasu on 1/5/18.
//

#ifndef SPARQ_CONNECTOR_H
#define SPARQ_CONNECTOR_H

#include <thread>

namespace sparq {
    template<class T>
    class Connector {
    public:
        using reader = std::function<T(void)>;

        using writer = std::function<void(const T &)>;

        Connector(const reader &r, const writer &w) :rd(r), wr(w), quitflag(false) {
            t1 = new std::thread(&Connector::run, this);
        }

        void join() {
            t1->join();
        }

        void quit() {
            this->quitflag = true;
        }

    protected:
        void run() {
            while (!this->quitflag) {
                T obj = rd();
                wr(obj);
            }
        }

        std::thread *t1;
        reader rd;
        writer wr;
        bool quitflag;
    };
}

#endif //SPARQ_CONNECTOR_H
