//
// Created by Samit Basu on 12/31/17.
//

#ifndef OLYMPUS_MC_OSMUTEX_H
#define OLYMPUS_MC_OSMUTEX_H

#include <string>
#include "OSSemaphore.h"

namespace SparQ {
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

#endif //OLYMPUS_MC_OSMUTEX_H
