#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>

class ThreadPool {
public:
    explicit ThreadPool(size_t threadCount = 8)
    : pool_{std::make_shared<Pool>()} {
        assert(threadCount > 0);
        
        for(size_t i = 0; i < threadCount; ++i) {
            workers_.emplace_back([this] {
                WorkerThread();
            });
        }
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) noexcept = default;
    ThreadPool& operator=(ThreadPool&&) noexcept = default;
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(pool_->mtx);
            pool_->isClosed = true;
        }
        pool_->cond.notify_all();
        for(auto& worker : workers_) {
            worker.join();
        }
    }

    template<typename F, typename = std::enable_if_t<std::is_invocable_r_v<void, F>>>
    void AddTask(F&& task) {
        if constexpr(std::is_same_v<F, std::function<void()>>) {
            std::unique_lock<std::mutex> lock(pool_->mtx);
            if(task) {
                pool_->tasks.emplace(std::move(task));
                pool_->cond.notify_one();
            }
        } else {
            AddTask(std::function<void()>{std::forward<F>(task)});
        }
    }

private:
    void WorkerThread() {
        std::unique_lock<std::mutex> lock(pool_->mtx);
        while(!pool_->isClosed || !pool_->tasks.empty()) {
            if(pool_->tasks.empty()) {
                pool_->cond.wait(lock);
            } else {
                auto task = std::move(pool_->tasks.front());
                pool_->tasks.pop();
                lock.unlock();
                task();
                lock.lock();
            }
        }
    }

    struct Pool {
        std::mutex mtx;
        std::condition_variable cond;
        bool isClosed{false};
        std::queue<std::function<void()>> tasks;
    };

    std::shared_ptr<Pool> pool_;
    std::vector<std::thread> workers_;
};

#endif // THREADPOOL_H