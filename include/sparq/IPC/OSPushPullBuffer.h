//
// Created by Samit Basu on 12/31/17.
//

#ifndef SPARQ_OSPUSHPULLBUFFER_H
#define SPARQ_OSPUSHPULLBUFFER_H

#include <iostream>
#include "OSMutex.h"
#include "OSSharedMemory.h"

namespace sparq {

    /**
      * This is an IPC push pull buffer, not a thread push pull buffer.
      * It is not meant to be used by multiple threads in a single application
      * but by multiple applications communicating via shared memory.
      *
      * @tparam T
      * @tparam element_count
      */
    template <class T, int element_count>
    class OSPushPullBuffer {
        // FIXME - is there an issue with zero filling the shared memory segment on creation?
        static_assert(std::is_trivially_copyable<T>::value, "only trivially copyable types are supported in shared memory");
        struct BufferData {
            T data[element_count];
            int writer_index;
            int reader_index;
        };
        OSSharedMemory<BufferData> memory;
        OSSemaphore freecount;
        OSSemaphore usedcount;
        OSMutex writermutex;
        OSMutex readermutex;

    public:
        bool open(const std::string &name) {
            if (!freecount.open(name+".freecount",element_count)) return false;
            if (!usedcount.open(name+".usedcount",0)) return false;
            if (!writermutex.open(name+".writerlock")) return false;
            if (!readermutex.open(name+".readerlock")) return false;
            if (!memory.open(name+".buffer")) return false;
            return true;
        }
        void push(const T& elem) {
            std::cout << "Waiting for free block\n";
            freecount.wait();
            std::cout << "Getting writer mutex\n";
            std::lock_guard<OSMutex> guard(writermutex);
            auto ptr = memory.get();
            ptr->data[ptr->writer_index] = elem;
            ptr->writer_index = (ptr->writer_index + 1) % element_count;
            std::cout << "Signaling used block\n";
            usedcount.notify();
        }
        T pop() {
            std::cout << "Waiting for used block\n";
            usedcount.wait();
            std::cout << "Getting reader mutex\n";
            std::lock_guard<OSMutex> guard(readermutex);
            auto ptr = memory.get();
            T ret = ptr->data[ptr->reader_index];
            ptr->reader_index = (ptr->reader_index + 1) % element_count;
            std::cout << "Signalling free block\n";
            freecount.notify();
            return ret;
        }
    };

}

#endif //SPARQ_OSPUSHPULLBUFFER_H
