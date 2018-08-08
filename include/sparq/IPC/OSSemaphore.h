//
// Created by Samit Basu on 12/31/17.
//

#ifndef SPARQ_OSSEMAPHORE_H
#define SPARQ_OSSEMAPHORE_H

#include <semaphore.h>
#include <iostream>
#include <string>
#include <fcntl.h>

namespace sparq {
   /**
    * Provides an abstraction for an OS semaphore (i.e., one used for IPC, rather than
    * for inter-thread-communication).
    */
    class OSSemaphore {
    public:
        bool open(const std::string &name, int initial_value = 0) {
            sem = sem_open(name.c_str(), O_CREAT, 0666, initial_value);
            std::cerr << "Open of " << name << " condition: " << (sem != SEM_FAILED) << "\n";
            return (sem != SEM_FAILED);
        }

        void notify() {
            sem_post(sem);
        }

        void wait() {
            sem_wait(sem);
        }

        ~OSSemaphore() {
            sem_close(sem);
        }

    private:
        sem_t *sem;
    };
}
#endif //SPARQ_OSSEMAPHORE_H
