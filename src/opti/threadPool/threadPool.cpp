#include "opti/ThreadPool/threadPool.hpp"

namespace optimization {
    ThreadPool::ThreadPool(size_t numThreads) {
        this->_numThreads = numThreads;

        for (size_t i = 0; i < _numThreads; i++) {
            _threads.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(
                            this->_queueMutex
                        );

                        _cv.wait(lock, [this] {
                            return !_tasks.empty() || _stop;
                        });

                        if  (this->_stop && this->_tasks.empty()) return;

                        task = std::move(this->_tasks.front());
                        this->_tasks.pop();
                    }
                    task(); // excute
                    {
                        std::unique_lock<std::mutex> lock(this->_queueMutex);
                        this->_activeTasks--;
                        if (this->_activeTasks == 0 && this->_tasks.empty()) {
                            _cv.notify_all();
                        }
                    }
                }
            });
        }
    }

    ThreadPool::~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(this->_queueMutex);
            _stop = true;
        }

        _cv.notify_all();

        for (auto& thread : _threads) {
            thread.join();
        }
    }

    void ThreadPool::enqueue(std::function<void()> task)
    {
        {
            std::unique_lock<std::mutex> lock(this->_queueMutex);
            this->_activeTasks++;
            _tasks.emplace(std::move(task));
        }
        _cv.notify_one();
    }

    void ThreadPool::enqueueBatch(std::vector<std::function<void()>> tasks)
    {
        {
            std::unique_lock<std::mutex> lock(this->_queueMutex);
            this->_activeTasks += tasks.size();
            for (auto &task : tasks)
                _tasks.emplace(std::move(task));
        }
        _cv.notify_all();
    }

    void ThreadPool::wait() {
        std::unique_lock<std::mutex> lock(_queueMutex);
        _cv.wait(lock, [this] { 
            return _tasks.empty() && _activeTasks == 0; 
        });
    }
}