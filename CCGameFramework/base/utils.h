#ifndef BASE_UTILS_H
#define BASE_UTILS_H

namespace std
{
    std::vector<std::string> split(const std::string &s, char delim = '\n');

    class semaphore
    {
    public:
        semaphore(long count = 0);

        void signal();
        void wait();

    private:
        std::mutex mutex_;
        std::condition_variable cv_;
        long count_;
    };

    template<typename T>
    class threadsafe_queue
    {
    private:
        mutable std::mutex mut;
        std::queue<T> data_queue;
        std::condition_variable data_cond;
    public:
        threadsafe_queue() {}

        void push(T new_value)
        {
            std::lock_guard<std::mutex> lk(mut);
            data_queue.push(std::move(new_value));
            data_cond.notify_one();
        }

        void wait_and_pop(T& value)
        {
            std::unique_lock<std::mutex> lk(mut);
            data_cond.wait(lk, [this] {return !data_queue.empty(); });
            value = std::move(data_queue.front());
            data_queue.pop();
        }

        std::shared_ptr<T> wait_and_pop()
        {
            std::unique_lock<std::mutex> lk(mut);
            data_cond.wait(lk, [this] {return !data_queue.empty(); });
            std::shared_ptr<T> res(
                std::make_shared<T>(std::move(data_queue.front())));
            data_queue.pop();
            return res;
        }

        bool try_pop(T& value)
        {
            std::lock_guard<std::mutex> lk(mut);
            if (data_queue.empty())
                return false;
            value = std::move(data_queue.front());
            data_queue.pop();
            return true;
        }

        std::shared_ptr<T> try_pop()
        {
            std::lock_guard<std::mutex> lk(mut);
            if (data_queue.empty())
                return std::shared_ptr<T>();
            std::shared_ptr<T> res(
                std::make_shared<T>(std::move(data_queue.front())));
            data_queue.pop();
            return res;
        }

        bool empty() const
        {
            std::lock_guard<std::mutex> lk(mut);
            return data_queue.empty();
        }
    };
}

#endif