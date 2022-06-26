#ifndef SAFEQUEUE_H
#define SAFEQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <string>
#include <atomic>

namespace wd
{
    template<typename T>
    class Safequeue
    {
    public:
        Safequeue()
        {
            m_bTermination = false;
        }
        ~Safequeue(void)
        {

        }
        //（1）没有调用termination时，每调用一次出队一个元素，直到队列为空本方法阻塞线程。
        //（2）在调用了termination后，本方法永不阻塞，如果原本已经处于阻塞状态，接触阻塞状态。
        //（3）返回true时，value值有效。返回false时，value值无效。调用了termination且队列为空时返回false.
        bool waitPop(T& value)
        {
            std::unique_lock<std::mutex> lock(mtx);
            data_cond.wait(lock, [this]{ return ((!data_queue.empty()) || m_bTermination); });
            //不为空则出队
            if (!data_queue.empty())
            {
                value = std::move(*data_queue.front());
                data_queue.pop();
                return true;
            }
            //队列为空则返回失败
            return false;
        }

        //队列为空返回false
        bool tryPop(T& value)
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (data_queue.empty())
                return false;
            //
            value = move(*data_queue.front());
            data_queue.pop();
            return true;
        }
        std::shared_ptr<T> waitPop()
        {
            std::unique_lock<std::mutex> lock(mtx);
            data_cond.wait(lock, [this] { return ((!data_queue.empty()) || m_bTermination); });
            if (!data_queue.empty())
            {
                std::shared_ptr<T> res = data_queue.front();
                data_queue.pop();
                return res;
            }
            return nullptr;
        }
        //队列为空返回null
        std::shared_ptr<T> tryPop()
        {
            std::lock_guard<std::mutex> lock(mtx);
            if(data_queue.empty())
            {
                return nullptr;
            }
            std::shared_ptr<T> res = data_queue.front();
            data_queue.pop();
            return res;
        }
        //插入一项
        void push(T new_value)
        {
            if (m_bTermination)
                return;
            //
            std::shared_ptr<T> data(std::make_shared<T>(move(new_value)));
            std::lock_guard<std::mutex> lock(mtx);
            data_queue.push(data);
            data_cond.notify_one();
        }
        bool empty()
        {
            std::lock_guard<std::mutex> lock(mtx);
            return data_queue.empty();
        }
        int size()
        {
            std::lock_guard<std::mutex> lock(mtx);
            return data_queue.size();
        }
        //设置队列为退出状态。在退出状态下，忽略入队，可以执行出队，但当队列为空时，wait_and_pop不会阻塞。
        void exit()
        {
            std::lock_guard<std::mutex> lock(mtx);
            m_bTermination = true;
            data_cond.notify_all();
        }
        //是退出状态吗
        bool isExit()
        {
            return m_bTermination;
        }
        void clear()
        {
            if (data_queue.size() == 0)
                return;
            //
            std::lock_guard<std::mutex> lock(mtx);
            std::queue<std::shared_ptr<T>> empty;
            std::swap(empty, data_queue);
        }
    private:
        std::mutex mtx;
        std::queue<std::shared_ptr<T>> data_queue;
        std::condition_variable data_cond;
        std::atomic<bool> m_bTermination;
    };
}


#endif // SAFEQUEUE_H
