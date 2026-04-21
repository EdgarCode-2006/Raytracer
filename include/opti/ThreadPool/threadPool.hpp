#ifndef THREAD_POOL_HPP_
#define THREAD_POOL_HPP_
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <raylib.h>
#include <atomic>
#include "utils/rayTracer.hpp"

namespace optimization {
    class ThreadPool {
        public:
            ~ThreadPool();
            ThreadPool(size_t numThreads);
            void enqueue(std::function<void()> task);
            void wait();
            
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
