/*
 * Semaphore.h
 *
 *  Created on: Mar 16, 2016
 *      Author: Saman Barghi
 */

#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_
#include <semaphore.h>

class semaphore
{
public:
    semaphore()
    {
        sem_init(&sem_, 0, 0);
    }

    ~semaphore()
    {
        sem_destroy(&sem_);
    }

    void wait()
    {
        sem_wait(&sem_);
    }

    void post()
    {
        sem_post(&sem_);
    }

private:
    sem_t               sem_;

    semaphore(semaphore const&);
    semaphore& operator = (semaphore const&);
};





#endif /* SEMAPHORE_H_ */
