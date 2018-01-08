//
// Created by Samit Basu on 12/31/17.
//

#ifndef SPARQ_OSPUSHPULLBUFFER_H
#define SPARQ_OSPUSHPULLBUFFER_H

#include <iostream>
#include <mutex>
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
    template <class T, int element_count, bool lossy = false>
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
            std::cerr << "Starting Buffer open " << name << "\n";
            if (!lossy && !freecount.open(name+".freecount",element_count)) return false;
            if (!usedcount.open(name+".usedcount",0)) return false;
            if (!writermutex.open(name+".writerlock")) return false;
            if (!readermutex.open(name+".readerlock")) return false;
            if (!memory.open(name+".buffer")) return false;
            std::cerr << "Buffer open successful\n";
            return true;
        }
        void push(const T& elem) {
            if (!lossy) freecount.wait();
            std::lock_guard<OSMutex> guard(writermutex);
            auto ptr = memory.get();
            ptr->data[ptr->writer_index] = elem;
            ptr->writer_index = (ptr->writer_index + 1) % element_count;
            usedcount.notify();
        }
        T pop() {
            std::cerr << "Pop issued\n";
            usedcount.wait();
            std::cerr << "Used count is valid\n";
            std::lock_guard<OSMutex> guard(readermutex);
            std::cerr << "Reader mutex locked\n";
            auto ptr = memory.get();
            std::cerr << "Reader_index = " << ptr->reader_index << "\n";
            T ret = ptr->data[ptr->reader_index];
            ptr->reader_index = (ptr->reader_index + 1) % element_count;
            if (!lossy) freecount.notify();
            return ret;
        }
    };

}

#endif //SPARQ_OSPUSHPULLBUFFER_H
