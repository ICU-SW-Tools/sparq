//
// Created by Samit Basu on 12/26/17.
//

#ifndef SPARQ_TIMEQ_H
#define SPARQ_TIMEQ_H

#include <functional>
#include <chrono>
#include <queue>
#include <iostream>

namespace sparq {
    // Provides a priority Q for time stamps.
    using callback_t = std::function<void()>;
    using Clock = std::chrono::high_resolution_clock;
    using timepoint_t = Clock::time_point;

    template <typename V, typename R>
    std::ostream& operator<<(std::ostream&s, const std::chrono::duration<V,R>& d) {
        s << "[" << d.count() << " of " << R::num << "/" << R::den << "]";
        return s;
    };

    typedef uint64_t timerid_t;

    struct TimeStamp {
        timerid_t id; // Provided by the TimeQ when the TimeStamp is added
        callback_t callback;
        timepoint_t when;

        bool operator>(const TimeStamp &other) const {
            return this->when > other.when;
        }
        TimeStamp(timerid_t _id, callback_t _callback, timepoint_t _when) :
                id(_id), callback(std::move(_callback)), when(_when) {}
    };

    inline TimeStamp mkTimeStamp(uint64_t micros, const callback_t &callback) {
        timepoint_t when = Clock::now() + std::chrono::microseconds(micros);
        return TimeStamp(0, callback, when);
    }

    inline std::ostream& operator<<(std::ostream& o, const TimeStamp &ts) {
        auto micros = std::chrono::duration_cast<std::chrono::microseconds>(ts.when - Clock::now()).count();
        o << "TimeStamp {id: " << ts.id << " callback?" << bool(ts.callback) << " when " << micros << "us}";
        return o;
    }

    using baseQ = std::priority_queue<TimeStamp, std::vector<TimeStamp>, std::greater<TimeStamp>>;


    class TimeQ : private baseQ{
    public:
        static const uint64_t MilliSeconds = 1000;
        static const uint64_t Seconds = 1000*1000;
        timerid_t add(const TimeStamp &ts) {
            TimeStamp t(ts);
            t.id = this->counter++;
            this->push(t);
            return t.id;
        }
        bool empty() const {
            return baseQ::empty();
        }
        const TimeStamp& top() const {
            return baseQ::top();
        }
        void pop() {
            baseQ::pop();
        }
        timepoint_t next() const {
            auto micros = std::chrono::duration_cast<std::chrono::microseconds>(baseQ::top().when - Clock::now()).count();
            //std::cout << "Next timer expires " << micros << " from now\n";
            return baseQ::top().when;
        }
        bool cancel(timerid_t id) {
            //std::cout << "cancel: timers in list: " << this->c.size() << "\n";
            for (auto &t : this->c) {
                if (t.id == id) {
                    t.id = 0;
                    return true;
                }
            }
            return false;
        }
        void drain() {
            while (!this->empty()) this->pop();
        }
        void update() {
            while ((!this->empty()) && (this->next() <= Clock::now())) {
                if (this->top().id && this->top().callback) {
//                    std::cout << "Processing callback of timer " << this->top().id << "\n";
                    this->top().callback();
                }
                this->pop();
            }
        }
    protected:
        timerid_t counter = 1;
    };

    inline uint64_t operator "" _sec(unsigned long long t) {return t*1000*1000;}
    inline uint64_t operator "" _msec(unsigned long long t) {return t*1000;}
    inline uint64_t operator "" _usec(unsigned long long t) {return t;}
}

#endif //SPARQ_TIMEQ_H
