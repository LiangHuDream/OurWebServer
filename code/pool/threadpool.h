/*
 * @Author: Caviar
 * @Date: 2024-05-26 16:06:49
 * @LastEditors: Caviar
 * @LastEditTime: 2024-05-26 17:40:02
 * @Description: 
 */

#pragma once

#include <type_traits>

constexpr size_t DEFAULT_THREADS_NUM{8}; // 默认线程池容量

// 定义一个线程池类
class ThreadPool {
private:
    // 工作线程执行的函数
    void WorkerThread() {
        std::unique_lock<std::mutex> lock(pool_->mtx);
        while(!pool_->isClosed || !pool_->tasks.empty()) { // 当未关闭或有任务时继续
            if(pool_->tasks.empty()) { // 无任务则等待
                pool_->cond.wait(lock);
            } else {
                // 获取并移除队首任务，解锁执行，完成后重新上锁
                auto task = std::move(pool_->tasks.front());
                pool_->tasks.pop();
                lock.unlock();
                task();
                lock.lock();
            }
        }
    }

    // 线程池内部使用的数据结构，包括同步锁、条件变量、关闭标志和任务队列
    struct Pool {
        std::mutex mtx;
        std::condition_variable cond;
        bool isClosed{false};
        std::queue<std::function<void()>> tasks;
    };

    // 实例变量：共享池状态指针和工作线程集合
    std::shared_ptr<Pool> pool_;
    std::vector<std::thread> workers_;

public:
    // 构造函数，允许指定线程池中的线程数量
    explicit ThreadPool(size_t threadCount = DEFAULT_THREADS_NUM)
    : pool_{std::make_shared<Pool>()} { // 使用共享指针存储线程池状态
        assert(threadCount > 0); // 确保线程数量大于0
        
        // 创建指定数量的工作线程，每个线程都运行WorkerThread函数
        for(size_t i = 0; i < threadCount; ++i) {
            workers_.emplace_back([this] {
                WorkerThread(); // 捕获this指针以访问ThreadPool的成员
            });
        }
    }

    // 禁止拷贝构造和赋值操作，防止资源管理问题
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // 默认实现移动构造和移动赋值操作符
    ThreadPool(ThreadPool&&) noexcept = default;
    ThreadPool& operator=(ThreadPool&&) noexcept = default;

    // 析构函数，确保所有线程完成并退出
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(pool_->mtx); // 锁住状态变量
            pool_->isClosed = true; // 设置关闭标志
        }
        pool_->cond.notify_all(); // 唤醒所有等待的线程
        // 等待所有工作线程结束
        for(auto& worker : workers_) {
            worker.join();
        }
    }

    // 添加任务到线程池的模板函数
    template<typename F, typename = std::enable_if_t<std::is_invocable_r_v<void, F>>>
    void AddTask(F&& task) {
        // 特化处理std::function<void()>
        if constexpr(std::is_same_v<F, std::function<void()>>) {
            std::unique_lock<std::mutex> lock(pool_->mtx);
            if(task) { // 防止空函数被添加
                pool_->tasks.emplace(std::move(task)); // 将任务移入任务队列
                pool_->cond.notify_one(); // 通知一个等待的线程有新任务
            }
        } else { // 非std::function<void()>类型的任务则转换后添加
            AddTask(std::function<void()>{std::forward<F>(task)});
        }
    }
};
