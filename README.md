# MPSCQ

This is an intrusive lock-free multiple-producer, single-consumer queue inspired by Dmitry Vyukov's implementation 
( [here](http://www.1024cores.net/home/lock-free-algorithms/queues/intrusive-mpsc-node-based-queue), and 
[here](https://www.google.com/url?q=https://groups.google.com/d/msg/lock-free/nvjCNJgb0bA/9eDuTeDcMXQJ&sa=D&ust=1459199281327000&usg=AFQjCNESQjG_pfvey8S35QKNIMll9JUzdw)).

There is an intrusive blocking, an intrusive non-blocking, and non-intrusive blocking version in the _src/MPSCQ.h_. For all  cases consumer only spins if the producer is blocked before 
performing `prev->next = &first;`.
The while loops can be replaced with `return` statements to avoid unnecessary spinning
(e.g., to check other queues or perform other work until the producer is unblocked). 


Blocking queues only return true and false pop which indicates whether the queue is empty or not, and thus whether the caller should block or not (blocking is done by the caller). There are examples in _test_ directory. 
