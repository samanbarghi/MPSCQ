/*
 * MPSCQueue.h
 *
 *  Created on: Mar 12, 2016
 *      Author: Saman Barghi
 */

#ifndef MPSCQUEUE_H_
#define MPSCQUEUE_H_

#include <atomic>
#include <cassert>
#include <iostream>

/*
 * Intrusive Nonblocking MPSC Queue
 */

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

    //Push a single element
    void push(T& elem){insert(elem, elem);}
    //Push multiple elements in a form of a linked list, linked by next
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

/*
 * Intrusive Blocking MPSC Queue
 */


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

    //push a single element
    bool push(T& elem){return insert(elem, elem);}
    //Push multiple elements in a form of a linked list, linked by next
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


/*
 * NonIntrusive Blocking MPSC Queue
 */

template<typename T>
class NonIntrusiveBlockingMPSCQueue {
public:
    class Node {
        friend class NonIntrusiveBlockingMPSCQueue<T>;
        Node* volatile next;
        T*  state;
      public:
        constexpr Node(T* s) : next(nullptr), state(s) {}
        constexpr Node() : next(nullptr), state(nullptr) {}
        void setState(T* s){ state = s;};
        T* getState(){return state;};

    } __attribute__((__packed__));
private:
    Node*   volatile    head;
    Node*               stub;

    std::atomic<Node*>  tail;

    bool insert(Node& first, Node& last){
      last.Node::next = nullptr;
      Node* prev = tail.exchange(&last, std::memory_order_acquire);
      bool was_empty = ((uintptr_t)prev & 1) != 0;
      prev = (Node*)((uintptr_t)prev & ~1);
      prev->Node::next = &first;
      return was_empty;
    }

public:
    NonIntrusiveBlockingMPSCQueue(): tail(nullptr){
        stub = new Node();
        tail.store( (Node*) ((uintptr_t)stub | 1 ));
        head = stub;
    };

    //push a single element
    bool push(Node& elem){return insert(elem, elem);}
    //Push multiple elements in a form of a linked list, linked by next
    bool push(Node& first, Node& last){return insert(first, last);}

    // pop operates in chunk of elements and re-inserts stub after each chunk
    Node* pop(){
        Node* hhead = head;
l_retry:
        Node* next = head->next;
        if(next){
            head = next;
            hhead->state = next->state;
            return hhead;
        }
        Node* ttail = tail;
        if(((uintptr_t)ttail & 1) != 0) return nullptr;

        if(head != ttail){                     // current chunk empty
            // wait for producer in insert()
            while(head->next == nullptr) asm volatile("pause");
            goto l_retry;
        }else{
            //If tail is marked empty, queue should not have been scheduled
            Node* expected = head;
            Node* xchg = (Node*)((uintptr_t)expected | 1);
            if(tail.compare_exchange_strong(expected, xchg, std::memory_order_release)){
                return nullptr; //The Queue is empty
            }//otherwise another thread is inserting
            goto l_retry;
        }
    }
};

#endif /* MPSCQUEUE_H_ */
