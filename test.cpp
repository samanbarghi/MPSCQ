/*
 * main.cpp
 *
 *  Created on: Mar 12, 2016
 *      Author: saman
 */
#include "MPSCQueue.h"
#include <iostream>
#include <unistd.h>
#include <thread>

using namespace std;
const uint64_t MAX = 1000000;
const int THREADS = 10;

class Element : public MPSCQueue<Element>::Node{
public:
    uint64_t value;;
    Element(uint64_t val): value(val){};
};

void run(int id, uint64_t* counter, MPSCQueue<Element>* q){
    Element* node;
    for(size_t i =0; i < MAX; i++){
        node = new Element(i+(id*MAX));
        q->push(*node);
        (*counter)++;
        node = nullptr;
    }
}

int main(){
    MPSCQueue<Element> q;

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

