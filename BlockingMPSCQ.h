/*
 * MPSCQueue.h
 *
 *  Created on: Mar 12, 2016
 *      Author: saman
 */

#ifndef MPSCQUEUE_H_
#define MPSCQUEUE_H_

#include <atomic>
#include <cassert>
#include <iostream>

template<typename T>
class BlockingMPSCQueue {
public:
    class Node{
    public:
        friend class BlockingMPSCQueue<T>;
        Node* volatile  next;
    public:
        Node():next(nullptr){};
    } __attribute__((__packed__));

private:
    std::atomic<Node*>  tail;
    Node                stub;

    Node*               head;

    bool insert(Node& first, Node& last){
      last.next = nullptr;
      Node* prev = tail.exchange(&last, std::memory_order_acquire);
      bool was_empty = ((uintptr_t)prev & 1) != 0;
      prev = (Node*)((uintptr_t)prev & ~1);
      prev->next = &first;
      return was_empty;
    }

public:
    BlockingMPSCQueue(): tail(&stub),head(&stub){}

    bool push(T& elem){return insert(elem, elem);}
    //Accepting an intrusive list is a better idea?
    bool push(T& first, T& last){return insert(first, last);}

    // pop operates in chunk of elements and re-inserts stub after each chunk
    T* pop(){
        Node* ttail = tail;
        //If tail is marked empty, queue should not have been scheduled
        assert(((uintptr_t)ttail & 1) == 0);
        if(head == &stub){                     // current chunk empty
                Node* expected = &stub;
                Node* xchg = (Node*)((uintptr_t)expected | 1);
                if(tail.compare_exchange_strong(expected, xchg, std::memory_order_release)){
                    return nullptr; //The Queue is empty
                }//otherwise another thread is inserting
            // wait for producer in insert()
            while(stub.next == nullptr) asm volatile("pause");
            head = stub.next;                  // remove stub
            insert(stub, stub);                // re-insert stub at end
        }
        // wait for producer in insert()
        while(head->next == nullptr) asm volatile("pause");
        // retrieve and return first element
        Node* l = head;
        head = head->next;
        return (T*)l;
    }
};

#endif /* MPSCQUEUE_H_ */
