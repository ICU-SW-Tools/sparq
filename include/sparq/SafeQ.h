//
// Created by Samit Basu on 12/17/17.
//

#ifndef SPARQ_SAFEQ_H
#define SPARQ_SAFEQ_H

// A thread safe templated Q

#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <pthread.h>

namespace sparq {
    // SafeQ uses the POSIX pthread API so that it can wait based on a
    // monotonic clock (and thus support wait times that do not change
    // based on system time changes). The current implementation of
    // std::condition_variable does not support this use case (see
    // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=41861). This is less
    // portable than the C++ standard library (i.e. no Windows support),
    // but avoids race conditions that could cause a timeout to be delayed
    // by an arbitrary amount (or time out too soon).
    template <class T>
    class SafeQ {
    public:
        SafeQ() : queue() {
            pthread_mutex_init(&mutex, NULL);
            pthread_condattr_init(&condattr);
            pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC);
            pthread_cond_init(&cond, &condattr);
        }

        void push(T t) {
            pthread_mutex_lock(&mutex);
            queue.push(t);
            pthread_mutex_unlock(&mutex);
            pthread_cond_signal(&cond);
        }

        T pop() {
            pthread_mutex_lock(&mutex);
            while (queue.empty()) pthread_cond_wait(&cond, &mutex);
            T val = queue.front();
            queue.pop();
            pthread_mutex_unlock(&mutex);
            return val;
        }

        bool tryPop(T* p) {
            pthread_mutex_lock(&mutex);
            if (!queue.empty()) {
                p[0] = queue.front();
                queue.pop();
                pthread_mutex_unlock(&mutex);
                return true;
            }
            pthread_cond_wait(&cond, &mutex);
            if (queue.empty()) {
                pthread_mutex_unlock(&mutex);
                return false;
            }
            p[0] = queue.front();
            queue.pop();
            pthread_mutex_unlock(&mutex);
            return true;
        }

        template <class S>
        bool tryPopUntil(T* p, const S& when) {
            pthread_mutex_lock(&mutex);
            if (!queue.empty()) {
                p[0] = queue.front();
                queue.pop();
                pthread_mutex_unlock(&mutex);
                return true;
            }

            const auto secs = std::chrono::time_point_cast<std::chrono::seconds>(when);
            const auto nsecs = std::chrono::duration_cast<std::chrono::nanoseconds>(when - secs);
            const struct timespec ts = {
                static_cast<std::time_t>(secs.time_since_epoch().count()),
                static_cast<long>(nsecs.count())
            };

            pthread_cond_timedwait(&cond, &mutex, &ts);

            if (queue.empty()) {
                pthread_mutex_unlock(&mutex);
                return false;
            }
            p[0] = queue.front();
            queue.pop();
            pthread_mutex_unlock(&mutex);
            return true;
        }

        void signal() {
            pthread_cond_signal(&cond);
        }
    private:
        std::queue<T> queue;
        pthread_mutex_t mutex;
        pthread_cond_t cond;
        pthread_condattr_t condattr;
    };
}


#endif //SPARQ_SAFEQ_H
