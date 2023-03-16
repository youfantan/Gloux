#pragma once

#include <queue>
#include <future>
#include <functional>

namespace gloux::utils {
    template<int size>
    class ThreadPool
    {
    private:
        std::vector<std::thread> threads;
        std::queue<std::function<void()>> queue_;
        std::mutex mtx_;
        std::condition_variable cv_;
        bool stop;
    public:
        ThreadPool() : stop(false) {
            for(int i = 0; i < size; ++i){
                threads.emplace_back(std::thread([this](){
                    while(true){
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(mtx_);
                            cv_.wait(lock, [this](){
                                return stop || !queue_.empty();
                            });
                            if(stop && queue_.empty()) return;
                            task = queue_.front();
                            queue_.pop();
                        }
                        task();
                    }
                }));
            }
        }
        ~ThreadPool() {
            {
                std::unique_lock<std::mutex> lock(mtx_);
                stop = true;
            }
            cv_.notify_all();
            for(std::thread &th : threads){
                if(th.joinable())
                    th.join();
            }
        }
        template<class F, class... Args>
        auto add(F&& f, Args&&... args)-> std::future<typename std::result_of<F(Args...)>::type> {
            using return_type = typename std::result_of<F(Args...)>::type;
            auto task = std::make_shared< std::packaged_task<return_type()> >(
                    std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );
            std::future<return_type> res = task->get_future();
            {
                std::unique_lock<std::mutex> lock(mtx_);

                queue_.emplace([task](){ (*task)(); });
            }
            cv_.notify_one();
            return res;
        }

    };
}