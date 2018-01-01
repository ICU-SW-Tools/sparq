//
// Created by Samit Basu on 12/31/17.
//

#ifndef OLYMPUS_MC_OSSHAREDMEMORY_H
#define OLYMPUS_MC_OSSHAREDMEMORY_H

#include <iostream>
#include <string>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace SparQ {
    /**
     * Manages an object as a shared memory implementation.
     */
    template <class T>
    class OSSharedMemory {
    public:
        static_assert(std::is_trivially_copyable<T>::value, "non-trivially copyable types not supported in shared memory");
        bool open(const std::string &name) {
            fd_shm = shm_open(name.c_str(), O_RDWR | O_CREAT , 0x660);
            std::cout << "Open of " << name << " status " << (fd_shm != -1) << "\n";
            if (fd_shm == -1) {
                perror("open");
                return false;
            }
            ftruncate(fd_shm, sizeof(T));
            std::cout << "Requesting: " << sizeof(T) << " bytes in shared mem fd = " << fd_shm << "\n";
            ptr = static_cast<T*>(mmap(NULL, sizeof(T), PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0));
            if (ptr == MAP_FAILED) {
                std::cout << "Memory map failed\n";
                perror("mmap");
                close(fd_shm);
                return false;
            }
            return true;
        }
        T* get() {
            return ptr;
        }
        ~OSSharedMemory() {
            munmap(ptr, sizeof(T));
            close(fd_shm);
        }
    private:
        int fd_shm;
        T* ptr;
    };


}
#endif //OLYMPUS_MC_OSSHAREDMEMORY_H
