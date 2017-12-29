#ifndef MAXY_TQ_INCLUDE
#define MAXY_TQ_INCLUDE

#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <queue>

namespace maxy
{

/**
 * A Simple Task Queue
 * maxy@maxy.ru
 */
class TQ
{
    public:
        /**
         * Base task
         * An implementation should extend this base and provide its own processing and cleanup
         */
        class Task
        {
            public:
                virtual void process () noexcept = 0;
                virtual ~Task () = 0;
        };

        template<class A, class B> TQ (std::chrono::duration<A, B> timeout);
        TQ () : TQ (std::chrono::milliseconds (100)) {}; // default constructor uses 100 ms timeout
        ~TQ ();

        /**
         * the queue is neither copiable nor movable
         */
        TQ (const TQ &) = delete;
        TQ (const TQ &&) = delete;
        TQ & operator= (const TQ &) = delete;
        TQ & operator= (const TQ &&) = delete;

        int post (Task * task);    // enqueue a new task. this is to be used like q.post(new T{})
        void purge ();             // purge queue, discard all pending tasks

    private:
        std::thread thread;
        std::mutex mutex;
        std::condition_variable cv;
        std::queue<Task*> queue;
        bool terminated;
};

/**
 * Templated constructor to accept various std timeouts
 */
template<class A, class B> TQ::TQ (std::chrono::duration<A, B> timeout) : terminated (false)
{
    thread = std::thread ([this,timeout] () 
    {
        std::unique_lock<std::mutex> lock (mutex);

        while (!terminated)
        {
            if (cv.wait_for (lock, timeout) == std::cv_status::timeout) continue;
            if (terminated) break;

            while (queue.size ())
            {
                auto task = queue.front ();
                queue.pop ();

                if (task != nullptr)
                {
                    lock.unlock ();
                    task->process ();
                    delete task;
                    lock.lock ();
                }
            }
        }
    });
}

}

#endif
