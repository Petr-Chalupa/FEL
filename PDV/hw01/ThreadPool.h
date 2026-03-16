#pragma once

#include <list>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

template<typename JobT, typename WorkerT>
class ThreadPool {
    std::list<JobT> job_queue{};
    std::vector<std::thread> worker_threads{};
    WorkerT worker_fn;
    std::mutex mutex{};
    std::condition_variable cond_var{};

public:
    ThreadPool(const size_t thread_count, WorkerT worker) : worker_fn(worker) {
        for (size_t i = 0; i < thread_count; ++i) {
            worker_threads.push_back(std::thread([this] {
                worker_loop();
            }));
        }
    }

    void process(const JobT job) {
        auto lock = std::unique_lock(mutex);
        job_queue.push_back(job);
        cond_var.notify_one();
    }

    void join() {
        for (auto &worker_thread: worker_threads) {
            worker_thread.join();
        }
    }

private:
    void worker_loop() {
        while (true) {
            auto lock = std::unique_lock(mutex);
            cond_var.wait(lock, [this] { return !job_queue.empty(); });

            auto job = job_queue.front();
            job_queue.pop_front();

            lock.unlock();

            if (!job) break; // If job is 0, end
            worker_fn(job); // Else do job
        }
    }
};
