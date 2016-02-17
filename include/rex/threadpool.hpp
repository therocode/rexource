#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

namespace rex
{
    class TaskComparer
    {
        public:
            bool operator() (const std::pair<int32_t, std::function<void()>>& a, const std::pair<int32_t, std::function<void()>>& b)
            {
                return a.first > b.first;
            }
    };
    
    class ThreadPool {
    public:
        ThreadPool(size_t threadCount);
        template<class Task, class... Args>
        std::future<typename std::result_of<Task(Args...)>::type> enqueue(Task&& task, int32_t priority = 0, Args&&... args);
        std::vector<std::thread::id> getThreadIds();
        ~ThreadPool();
    private:
        std::vector<std::thread> mWorkers;
        std::priority_queue<std::pair<int32_t, std::function<void()>>, 
                            std::vector<std::pair<int32_t, std::function<void()>>>,
                            TaskComparer> mTasks;
        
        // synchronization
        std::mutex mQueueMutex;
        std::condition_variable mCondition;
        bool mStop;
    };
     
    inline ThreadPool::ThreadPool(size_t threadCount) :
        mStop(false)
    {
        for(size_t i = 0; i < threadCount; ++i)
        {
            mWorkers.emplace_back(
                [this]
                {
                    for(;;)
                    {
                        std::unique_lock<std::mutex> lock(mQueueMutex);

                        while(!mStop && mTasks.empty())
                            mCondition.wait(lock);

                        if(mStop && mTasks.empty())
                            return;

                        std::function<void()> task(mTasks.top().second);
                        mTasks.pop();
                        lock.unlock();
                        task();
                    }
                }
            );
        }
    }
    
    inline std::vector<std::thread::id> ThreadPool::getThreadIds()
    {
        std::lock_guard<std::mutex> lock(mQueueMutex);
        std::vector<std::thread::id> result;
    
        for(const auto& thread : mWorkers)
            result.push_back(thread.get_id());
    
        return result;
    }
    
    template<class Task, class... Args>
    std::future<typename std::result_of<Task(Args...)>::type> ThreadPool::enqueue(Task&& task, int32_t priority, Args&&... args) 
    {
        using ReturnType = typename std::result_of<Task(Args...)>::type;
        
        if(mStop)
            throw std::runtime_error("enqueue on stopped ThreadPool");
    
        auto packagedTask = std::make_shared<std::packaged_task<ReturnType()>>(
                std::bind(std::forward<Task>(task), std::forward<Args>(args)...)
            );
            
        std::future<ReturnType> result = packagedTask->get_future();
        {
            std::unique_lock<std::mutex> lock(mQueueMutex);
            mTasks.push({priority, [packagedTask](){ (*packagedTask)(); }});
        }
        mCondition.notify_one();
        return result;
    }
    
    inline ThreadPool::~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(mQueueMutex);
            mStop = true;
        }

        mCondition.notify_all();

        for(size_t i = 0; i < mWorkers.size(); ++i)
            mWorkers[i].join();
    }
}
