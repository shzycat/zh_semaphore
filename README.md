# zh::semaphore
Before C++20, there is not a semaphore provided by standard library.
This is a simple implementation of semaphore, use C++11.

# Demo
```
/* for binary_semaphore */
zh::semaphore sem;
sem.wait(); // or sem.wait_for(/*milliseconds*/)
//...other thread...
//sem.notify();

/* for counting_semaphore */
zh::counting_semaphore sem;
//you can acquire specific count.
sem.acquire(6);
//...other thread...
//sem.release(16);


```