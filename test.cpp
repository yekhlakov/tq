#include "tq.h"
#include <iostream>

/**
 * Test implementation of a task
 * The processing function sleeps for a given timeout in order to simulate long processing time
 * Not that the queue accepts any task built on top of TQTask
 */
class MyTask : public maxy::TQ::Task
{
    int x;
    int timeout;
    public:
        void operator() () noexcept
        {
            if (timeout <= 0)
            {
                std::cout << "TASK (" << x << ") ERROR!\n";
                return;
            }

            std::cout << "TASK (" << x << ") processing start\n";
            std::this_thread::sleep_for (std::chrono::milliseconds (timeout));
            std::cout << "TASK (" << x << ") processing end\n";
        };

        MyTask (int xx, int tt) : x {xx}, timeout {tt} { std::cout << "TASK (" << x << ") construction\n"; };
        ~MyTask () { std::cout << "TASK (" << x << ") destruction\n"; };
};

/**
 * Helper function that just waits for 100 ms
 */
void tick ()
{
    std::this_thread::sleep_for (std::chrono::milliseconds (100));
    std::cout << "--- Main thread tick\n";
}

/**
 * Main function simulates the operation of the queue.
 * It works for a number of 'time frames' and during each frame it posts some tasks to the queue
 * Note the processing of tasks takes longer than the execution time of the whole main() function
 */
int main()
{
    maxy::TQ q {std::chrono::milliseconds (50)};

    q.post (new MyTask {1, 100});
    tick ();
    q.post (new MyTask {2, 200});
    tick ();
    tick ();
    q.post (new MyTask {3, 100});
    q.post (nullptr);
    q.post (new MyTask {4, 200});
    q.post (new MyTask {5, 0}); // this will generate an error
    tick ();
    q.post (new MyTask {6, 100});
    tick ();
    std::cout << "--- Main thread termination\n";
}
