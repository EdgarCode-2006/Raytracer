#ifndef THREAD_POOL_HPP_
#define THREAD_POOL_HPP_
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <raylib.h>
#include <atomic>
#include "utils/rayTracer.hpp"

namespace optimization {
    class ThreadPool {
        public:
            ~ThreadPool();
            ThreadPool(size_t numThreads);
            void enqueue(std::function<void()> task);
            // all at once under one lock: queuing a frame's tiles one by one makes the caller
            // fight the workers for the lock on every tile, for as long as the frame renders
            void enqueueBatch(std::vector<std::function<void()>> tasks);
            void wait();
            bool busy() {return _activeTasks > 0;};
            
        private:
            std::atomic<size_t> _activeTasks{0};
            size_t _numThreads;
            std::vector<std::thread> _threads;
            std::queue<std::function<void()>> _tasks;
            std::mutex _queueMutex;
            std::condition_variable _cv;
            bool _stop = false;
    };
}
#endif
