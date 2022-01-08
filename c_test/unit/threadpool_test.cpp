#include "app.h"
#include "test.h"

#include "mlib/defer.imp.h"

#include <thread>
#include <mutex>
#include <queue>
#include <stack>
#include <functional>
#include <vector>
#include <array>

/*
New method:
Somewhat data-oriented?


*/

template<typename Data>
struct Thread_Pool_Context {
    std::mutex queue_lock;
    std::stack<Data> queue;
    std::vector<std::thread> threads;
};

// template<typename Func, typename Data>
// void pool_worker(Thread_Pool_Context<Data>* tpc, Func handler)
template<typename Data>
void pool_worker(Thread_Pool_Context<Data>* tpc, std::function<void(Data)> handler)
{
    int local_data;
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


TEST(ThreadPoolTest, Experimentation2)
{
    Thread_Pool_Context<int> pool;
    pool.queue.push(1);
    pool.queue.push(2);
    pool.queue.push(3);

    int result = 0;

    pool_run<int>(&pool, std::thread::hardware_concurrency(), [&result](int data) {
                result += data;
            });

    ASSERT_EQ(result, 6);
}



TEST(ThreadPoolTest, Experimentation)
{
    Thread_Pool_Context<int> pool;
    pool.queue.push(1);
    pool.queue.push(2);
    pool.queue.push(3);

    int result = 0;
    const size_t num_threads = std::thread::hardware_concurrency();

    for (size_t i = 0; i < num_threads; i++)
    {
        pool.threads.push_back(std::thread([&pool, &result]() {
            pool_worker<int>(&pool, [&result](int data) {
                result += data;
            });
        }));
    }

    for (auto &i : pool.threads)
    {
        i.join();
    }

    ASSERT_EQ(result, 6);
}

/*
Method:
Fill up a stack of worker-functions
Spin up n threads, each will pop from stack and execute the function
When the stack is empty: quit threads
Join threads
Evaluate result.



How to pop an lambda?

*/

// TBD: How much memory does these lambdas take up?
// template<typename WorkerFunction>
// struct ThreadPool {
//   // bool stop;
//   std::stack<std::function<WorkerFunction>> queue;
//   std::mutex queue_lock;
//   std::vector<std::thread> workers;
// };

// template<typename WorkerFunction>
// void tp_worker(ThreadPool<WorkerFunction>* self) {
//   WorkerFunction* func; // TODO: Problem now: how can I pop the lambda?
//   while(true) {
//     if(self->queue.empty()) break;
//     // func = self->queue.top();
//     memcpy(func, &self->queue.top(), sizeof(WorkerFunction));
//     func();
//   }
// }

// TEST(ThreadPoolTest, Dev) {
//     ThreadPool<void()> pool;

//     // int external = 42;
//     int result = 0;
//     int expected_result = 0;

//     for(int i=0; i<100; i++) {
//       pool.queue.push([&result, i]() {
//         result += i;
//       });

//       expected_result += i;
//     }

//     for(int i=0; i<1; i++) {
//       pool.workers.push_back(std::thread(tp_worker<void()>, &pool));
//     }

//     for(auto& t : pool.workers) {
//       t.join();
//     }

//     // queue.pop()
//     ASSERT_EQ(expected_result, result);

// }

// TEST(ThreadPoolTest, Experimentation) {
//     std::stack<std::function<int()>> queue;
//     int external = 42;
//     int result = 0;

//     queue.push([external]() {
//         return 2*external;
//     });

//     queue.push([external]() {
//         return 3*external;
//     });

//     std::thread t([&queue, &result]() {
//       while(!queue.empty()) {
//         result += queue.top()();
//         queue.pop();
//       }
//     });

//     t.join();

//     // queue.pop()
//     ASSERT_EQ(result, 2*external + 3*external);

// }

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}