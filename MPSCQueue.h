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
class MPSCQueue {
public:
    class Node{
    public:
        friend class MPSCQueue<T>;
        Node* volatile  next;
    public:
        Node():next(nullptr){};
    } __attribute__((__packed__));

private:
    std::atomic<Node*>  tail;
    Node                stub;

    Node*               head;

    void insert(Node& first, Node& last){
      last.next = nullptr;
      Node* prev = tail.exchange(&last, std::memory_order_relaxed);
      prev->next = &first;
    }

public:
    MPSCQueue(): tail(&stub),head(&stub){}

    void push(T& elem){insert(elem, elem);}
    //Accepting an intrusive list is a better idea?
    void push(T& first, T& last){insert(first, last);}

    // pop operates in chunk of elements and re-inserts stub after each chunk
    T* pop(){
        if(head == &stub){                     // current chunk empty
            if (tail == &stub) return nullptr; // add CAS for block here
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
