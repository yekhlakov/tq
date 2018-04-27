# tq - A simple task queue

A new and improved, header-only version.

* Include `tq.h` where you wish to use the queue.
* Create your task class (or classes) deriving them from `tq::task`.
* Instantiate the queue.
* Post tasks to the queue like `q.post (new X{});`
* `test.cpp` provides an example of queue usage.


