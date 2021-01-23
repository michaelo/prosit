#pragma once

#include <thread>
#include <mutex>
#include <stack>
#include <functional>
#include <vector>

#include "mlib/defer.imp.h"

template<typename Data>
struct Thread_Pool_Context {
    std::mutex queue_lock;
    std::stack<Data> queue;
    std::vector<std::thread> threads;
};

template<typename Data>
void pool_worker(Thread_Pool_Context<Data>* tpc, std::function<void(Data)> handler)
{
    Data local_data;
    while (true)
    {
        // Mod-region
        {
            tpc->queue_lock.lock();
            defer(tpc->queue_lock.unlock());

            if (tpc->queue.empty())
                break;

            local_data = tpc->queue.top();
            tpc->queue.pop();
        }

        // Do specific logic
        handler(local_data);
    }
}

// Precondition: pool.queue is filled
template<typename Data>
void pool_run(Thread_Pool_Context<Data>* tpc, size_t num_threads, std::function<void(Data)> func) {
    // tpc->threads
    for (size_t i = 0; i < num_threads; i++)
    {
        tpc->threads.push_back(std::thread([&tpc, func]() {
            pool_worker<Data>(tpc, func);
        }));
    }

    for (auto &i : tpc->threads)
    {
        i.join();
    }
}
