#include "tq.h"

using namespace maxy;

/**
 * Dummy destructor for generalized task
 */
TQ::Task::~Task () {};

/**
 * Purge queue (remove all pending tasks without processing them)
 * Any task that is in processing now will be processed normally.
 * The queue is locked during purge operation.
 */
void TQ::purge ()
{
    mutex.lock ();
    while (queue.size ())
    {
        delete queue.front ();
        queue.pop ();
    }
    mutex.unlock ();
}

/**
 * Destruction of task queue. All pending tasks will be discarded
 */
TQ::~TQ ()
{
    if (terminated) return;
    terminated = true;
    thread.join ();
    purge ();
}

/**
 * Post a task to the queue
 * This may throw if the std::queue fails to push
 */
int TQ::post (TQ::Task * task)
{
    if (terminated)
    {
        delete task;
        return 1;
    }
    mutex.lock();
    try
    {
        queue.push(task);
    }
    catch(...)
    {
        mutex.unlock();
        throw;
    }
    mutex.unlock();
    cv.notify_all();
    return 0;
}
