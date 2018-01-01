cmake_minimum_required(VERSION 3.8)
project(olympus_mc)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES main.cpp SafeQ.h ActiveObject.h Singleton.h PubSub.h Semaphore.h TimeQ.h FSM.h NewFSM.h Event.h ActiveFSM.h NewFSM.cpp Broadcaster.h PODVariant.h IPC/OSSemaphore.h IPC/OSMutex.h IPC/OSPushPullBuffer.h IPC/OSSharedMemory.h)
add_executable(olympus_mc ${SOURCE_FILES})

add_executable(writer writer.cpp)
