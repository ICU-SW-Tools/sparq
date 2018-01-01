//
// Created by Samit Basu on 12/31/17.
//

#ifndef SPARQ_OSMUTEX_H
#define SPARQ_OSMUTEX_H

#include <string>
#include "OSSemaphore.h"

namespace sparq {
    /**
      *  For semantic clarity, we also define an OSMutex as a simple
      *  semaphore with different names.  Makes it compatible with
      *  std::lock_guard.
      */
    class OSMutex : public OSSemaphore {
    public:
        bool open(const std::string &name) {
            return OSSemaphore::open(name, 1);
        }
        void lock() {
            this->wait();
        }
        void unlock() {
            this->notify();
        }
    };

}

#endif //SPARQ_OSMUTEX_H
