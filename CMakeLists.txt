cmake_minimum_required(VERSION 3.8)
project(olympus_mc)

set(CMAKE_CXX_STANDARD 11)

set(HEADER_FILES include/sparq/SafeQ.h include/sparq/ActiveObject.h include/sparq/Singleton.h include/sparq/PubSub.h include/sparq/Semaphore.h include/sparq/TimeQ.h include/sparq/FSM.h
        include/sparq/ActiveFSM.h include/sparq/Broadcaster.h include/sparq/PODVariant.h include/sparq/IPC/OSSemaphore.h include/sparq/IPC/OSMutex.h
        include/sparq/IPC/OSPushPullBuffer.h include/sparq/IPC/OSSharedMemory.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SOURCE_FILES demo/main.cpp demo/NewFSM.h demo/NewFSM.cpp)

add_executable(olympus_mc ${SOURCE_FILES} ${HEADER_FILES})

add_executable(writer demo/writer.cpp ${HEADER_FILES})

add_library(sparq INTERFACE)

target_include_directories(sparq INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/sparq>
        $<INSTALL_INTERFACE:/include/sparq>  )

install(DIRECTORY include/ DESTINATION include )

