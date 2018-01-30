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
}

#endif