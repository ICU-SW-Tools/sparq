//
// Created by Samit Basu on 12/17/17.
//

#ifndef SPARQ_SAFEQ_H
#define SPARQ_SAFEQ_H

// A thread safe templated Q

#include <queue>
#include <mutex>
#include <condition_variable>

namespace sparq {
    template <class T>
    class SafeQ {
    public:
        SafeQ() : queue(), mutex(), cv() {}

        void push(T t) {
            std::lock_guard<std::mutex> guard(mutex);
            queue.push(t);
            cv.notify_one();
        }

        T pop() {
            std::unique_lock<std::mutex> lock(mutex);
            while (queue.empty())  cv.wait(lock);
            T val = queue.front();
            queue.pop();
            return val;
        }

        bool tryPop(T* p) {
            std::unique_lock<std::mutex> lock(mutex);
            if (!queue.empty()) {
                p[0] = queue.front();
                queue.pop();
                return true;
            }
            cv.wait(lock);
            if (queue.empty()) return false;
            p[0] = queue.front();
            queue.pop();
            return true;
        }

        template <class S>
        bool tryPopUntil(T* p, const S& when) {
            std::unique_lock<std::mutex> lock(mutex);
            if (!queue.empty()) {
                p[0] = queue.front();
                queue.pop();
                return true;
            }
            cv.wait_until(lock,when);
            if (queue.empty()) return false;
            p[0] = queue.front();
            queue.pop();
            return true;
        }

        void signal() {
            std::lock_guard<std::mutex> guard(mutex);
            cv.notify_one();
        }
    private:
        std::queue<T> queue;
        std::mutex mutex;
        std::condition_variable cv;
    };
}


#endif //SPARQ_SAFEQ_H
