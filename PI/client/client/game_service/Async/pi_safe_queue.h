#pragma once

#include <queue>
#include <mutex>


template<typename T>
class mw_safe_queue
{
public:
    void push(const T & t)
    {
        std::lock_guard<std::mutex> lck(mutex_);
        queue_.push(t);
    }

    void pop()
    {
        std::lock_guard<std::mutex> lck(mutex_);
        queue_.pop();
    }

    bool pop(T& t)
    {
        std::lock_guard<std::mutex> lck(mutex_);
        if (queue_.empty())
        {
            return false;
        }
        t = queue_.front();
        queue_.pop();
        return true;
    }

    bool front(T& t)
    {
        std::lock_guard<std::mutex> lck(mutex_);
        if (queue_.empty())
        {
            return false;
        }
        t = queue_.front();
        return true;
    }

private:
    std::mutex mutex_;//ע����������ͷ�����caiyprecursive_mutex
    std::queue<T> queue_;
};

