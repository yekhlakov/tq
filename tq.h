#ifndef MAXY_tq_INCLUDE
#define MAXY_tq_INCLUDE

#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <queue>

namespace maxy
{
	/**
	 * A Simple task Queue
	 * maxy@maxy.ru
	 */
	class tq
	{
		public:
			/**
			 * Base task
			 * An implementation should extend this base and provide its own processing and cleanup
			 */
			class task
			{
				public:
					virtual void operator() () noexcept = 0;
					virtual ~task () {};
			};

			template<class A, class B> tq (std::chrono::duration<A, B> timeout);
			tq () : tq (std::chrono::milliseconds (100)) {}; // default constructor uses 100 ms timeout
			~tq ()
			{
				if (terminated) return;
				terminated = true;
				thread.join ();
				purge ();
			}

			/**
			 * the queue is neither copiable nor movable
			 */
			tq (const tq &) = delete;
			tq (const tq &&) = delete;
			tq & operator= (const tq &) = delete;
			tq & operator= (const tq &&) = delete;

			/**
			* Post a task to the queue like q.post(new T{})
			* This may throw if the std::queue fails to push
			*/
			bool post (task * task)
			{
				if (terminated)
				{
					delete task;
					return false;
				}
				mutex.lock ();
				try
				{
					queue.push (task);
				}
				catch (...)
				{
					mutex.unlock ();
					throw;
				}
				mutex.unlock ();
				cv.notify_all ();
				return true;
			}

			/**
			* Purge queue (remove all pending tasks without processing them)
			* Any task that is in processing now will be processed normally.
			* The queue is locked during purge operation.
			*/
			void purge ()
			{
				mutex.lock ();
				while (queue.size ())
				{
					delete queue.front ();
					queue.pop ();
				}
				mutex.unlock ();
			}

		private:
			std::thread thread;
			std::mutex mutex;
			std::condition_variable cv;
			std::queue<task*> queue;
			bool terminated;
	};

	/**
	 * Templated constructor to accept various std timeouts
	 */
	template<class A, class B> tq::tq (std::chrono::duration<A, B> timeout) : terminated (false)
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
						(*task) ();
						delete task;
						lock.lock ();
					}
				}
			}
		});
	}

}

#endif
