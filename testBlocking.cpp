/*
 * main.cpp
 *
 *  Created on: Mar 12, 2016
 *      Author: saman
 */
#include "BlockingMPSCQ.h"
#include "Semaphore.h"
#include <iostream>
#include <unistd.h>
#include <thread>
#include <random>
#include <chrono>

using namespace std;
const uint64_t MAX = 100000;
const int THREADS = 10;
semaphore sem;

class Element : public BlockingMPSCQueue<Element>::Node{
public:
    uint64_t value;;
    Element(uint64_t val): value(val){};
};

void run(int id, uint64_t* counter, BlockingMPSCQueue<Element>* q){
    Element* node;
    std::mt19937_64 eng{std::random_device{}()};  // or seed however you want
    std::uniform_int_distribution<> dist{10, 100};

    for(size_t i =0; i < MAX; i++){
        node = new Element(i+(id*MAX));
        if(q->push(*node))
            sem.post();
        (*counter)++;
        node = nullptr;
        if(i%1000 == 0)
            std::this_thread::sleep_for(std::chrono::milliseconds{dist(eng)});
    }
}

int main(){
    BlockingMPSCQueue<Element> q;

    uint64_t sum = 0;
    uint64_t expected = (MAX*THREADS)*( (MAX*THREADS)-1)/2;

    std::thread threads[THREADS];
    uint64_t tcount[THREADS] = {0};
    for(int i =0; i < THREADS; i++){
        threads[i] = std::thread(run, i, &tcount[i], &q);
    }

    uint64_t count = 0;
    while(count < MAX*THREADS){
        Element* e=q.pop();
        if(e){
            sum += e->value;
            ++count;
            delete e;
            e = nullptr;
        }else{
           sem.wait();
        }
    }


    cout << "Sum:\t\t" << sum << endl;
    cout << "Expected:\t" << expected << endl;
    cout << count << endl;
    for(int i = 0; i < THREADS; i++)
        cout << tcount[i] << endl;

    for(int i = 0; i < THREADS; i++)
        threads[i].join();

}

