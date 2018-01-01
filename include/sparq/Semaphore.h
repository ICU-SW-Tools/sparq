//
// Created by Samit Basu on 12/17/17.
//

#ifndef OLYMPUS_MC_SEMAPHORE_H
#define OLYMPUS_MC_SEMAPHORE_H

#include <mutex>

namespace sparq {
    class Semaphore {
    public:
//        using duration = std::chrono::time_point<std::steady_clock,

        Semaphore(unsigned int count = 0): m_count(count) {}

        void notify() {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_count++;
            m_cv.notify_one();
        }

        void wait() {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this]() {return m_count > 0;});
            m_count--;
        }

        template <class Clock, class Duration>
        bool waitUntil(const std::chrono::time_point<Clock, Duration>& point) {
            std::unique_lock<std::mutex> lock(m_mutex);
            if (!m_cv.wait_until(lock, point, [this]() { return m_count > 0; }))
                return false;
            m_count--;
            return true;
        }

    private:
        std::mutex m_mutex;
        std::condition_variable m_cv;
        unsigned int m_count;
    };
}

#endif //OLYMPUS_MC_SEMAPHORE_H
