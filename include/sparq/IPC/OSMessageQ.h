//
// Created by sbasu on 1/3/18.
//

#ifndef SPARQ_OSMESSAGEQ_H
#define SPARQ_OSMESSAGEQ_H

#include <string>
#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace sparq {
    template <class T, int element_count>
    class OSMessageQ {
        static_assert(std::is_trivially_copyable<T>::value, "only trivially copyable types are supported");
        mqd_t fd;
        const int64_t nsec_per_sec = 1000000000;
    public:
        bool open(const std::string &name) {
            mq_attr attr;
            attr.mq_maxmsg = element_count;
            attr.mq_msgsize = sizeof(T);
            fd = mq_open(name.c_str(),O_RDWR | O_CREAT, 0666, &attr);
            return (fd > 0);
        }
        void push(const T& val, unsigned int priority = 0) {
            mq_send(fd, (const char *)(&val), sizeof(val), priority);
        }
        T pop() {
            T ret;
            unsigned int prio;
            mq_receive(fd, (char*)( &ret), sizeof(T), &prio);
            return ret;
        }
        bool tryPop(T* msg, uint64_t microseconds) {
            struct timespec tm;
            clock_gettime(CLOCK_REALTIME, &tm);
            const uint64_t current = tm.tv_sec * nsec_per_sec + tm.tv_nsec;
            const uint64_t future = current + microseconds * 1000;
            tm.tv_sec = future / nsec_per_sec;
            tm.tv_nsec = future % nsec_per_sec;
            unsigned int prio;
            int ret = mq_timedreceive(fd, (char*)(msg), sizeof(T), &prio, &tm);
            return (ret > 0);
        }
        bool tryPush(const T&val, uint64_t microseconds, unsigned int priority = 0) {
            struct timespec tm;
            clock_gettime(CLOCK_REALTIME, &tm);
            const uint64_t current = tm.tv_sec * nsec_per_sec + tm.tv_nsec;
            const uint64_t future = current + microseconds * 1000;
            tm.tv_sec = future / nsec_per_sec;
            tm.tv_nsec = future % nsec_per_sec;
            int ret = mq_timedsend(fd, (const char*)(&val), sizeof(T), priority, &tm);
            return (ret == 0);
        }
    };

}


#endif //SPARQ_OSMESSAGEQ_H
