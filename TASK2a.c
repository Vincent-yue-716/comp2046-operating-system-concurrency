// Yifei YUE 20125896 scyyy2

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include "coursework.h"
#include "linkedlist.h"

// the binary semaphore and counting semaphores
sem_t sync, full, empty;
// number of items produced by producer or consumed by consumer
int produceNum = 0, consumeNum = 0;
// total response time and turn around time
long int totalResponseTime = 0, totalTurnAroundTime = 0;
// bounded buffer
struct element * bufferHead = NULL, * bufferTail = NULL;


void * producer(void * id);
void * consumer(void * id);
void bubbleSort(struct element * start);
void swap(struct element *a, struct element *b);


int main()
{
    sem_init(&sync, 0, 1);
    sem_init(&empty, 0, MAX_BUFFER_SIZE);
    sem_init(&full, 0, 0);
    pthread_t tProducers[NUMBER_OF_PRODUCERS];
    pthread_t tConsumers[NUMBER_OF_CONSUMERS];

    // Id of each producer and consumer
    int proId[NUMBER_OF_PRODUCERS];
    int conId[NUMBER_OF_CONSUMERS];

    // consumers and producers are created and assigned unique ids
    for(int i = 0; i < NUMBER_OF_PRODUCERS; i++)
    {
        proId[i] = i;
        pthread_create(&tProducers[i], NULL, producer, (void *)&proId[i]);
    }

    for(int i = 0; i < NUMBER_OF_CONSUMERS; i++)
    {
        conId[i] = i;
        pthread_create(&tConsumers[i], NULL, consumer, (void *)&conId[i]);
    }

    // consumers and producers are joined
    for(int i = 0; i < NUMBER_OF_PRODUCERS; i++)
    {
        pthread_join(tProducers[i], NULL);
    }

    for(int i = 0; i < NUMBER_OF_CONSUMERS; i++)
    {
        pthread_join(tConsumers[i], NULL);
    }
    // free resources
    sem_destroy(&full);
    sem_destroy(&empty);
    sem_destroy(&sync);

    // calculate average response time and turn around time
    printf("Average Response Time = %lf\n", (double) totalResponseTime / MAX_NUMBER_OF_JOBS);
    printf("Average Turn Around Time = %lf\n", (double) totalTurnAroundTime / MAX_NUMBER_OF_JOBS);

    return 0;
}


void * producer(void * id)
{
    int producerId = *((int *)id);
    while(1)
    {
        sem_wait(&empty);
        // exit the thread when produce more than max number of jobs
        if(produceNum >= MAX_NUMBER_OF_JOBS)
        {
            break;
        }
        sem_wait(&sync);

        struct process * p = generateProcess();
        addLast(p, &bufferHead, &bufferTail);
        produceNum++;

        sem_post(&sync);
        printf("Producer = %d, Items Produced = %d, New Process Id = %d, Burst Time = %d\n", producerId, produceNum, p->iProcessId, p->iInitialBurstTime);
        sem_post(&full);
    }
    pthread_exit(NULL);
}

void * consumer(void * id)
{
    int consumerId = *((int *)id);

    while (1)
    {
        // exit the thread when consume more than max number of jobs
        if(consumeNum >= MAX_NUMBER_OF_JOBS)
        {
            break;
        }
        sem_wait(&full);
        sem_wait(&sync);

        // bubble sort the buffer
        bubbleSort(bufferHead);
        // fetch the first process which has the shortest burst time
        struct process * p = bufferHead->pData;
        removeFirst(&bufferHead, &bufferTail);
        consumeNum++;

        sem_post(&sync);

        struct timeval pStartTime;
        runNonPreemptiveJob(p, &pStartTime, &(p->oMostRecentTime));
        long int responseTime = getDifferenceInMilliSeconds(p->oTimeCreated, pStartTime);
        long int turnAroundTime = getDifferenceInMilliSeconds(p->oTimeCreated, p->oMostRecentTime);
        printf("Consumer = %d, Process Id = %d, Previous Burst Time = %d, New Burst Time = %d, Response Time = %ld, Turnaround Time = %ld\n",
               consumerId, p->iProcessId, p->iPreviousBurstTime, p->iRemainingBurstTime, responseTime, turnAroundTime);
        totalResponseTime += responseTime;
        totalTurnAroundTime += turnAroundTime;
        free(p);
        sem_post(&empty);
    }
    pthread_exit(NULL);
}

// swap two element
void swap(struct element *a, struct element *b)
{
    struct process * temp = a->pData;
    a->pData = b->pData;
    b->pData = temp;
}

// bubble sort a linked list
void bubbleSort(struct element *start)
{
    int swapped, i;
    struct element *ptr1;
    struct element *lPtr = NULL;

    /* Checking for empty list */
    if (start == NULL)
    {
        return;
    }
    do
    {
        swapped = 0;
        ptr1 = start;

        while (ptr1->pNext != lPtr)
        {
            struct process *process = ptr1->pData;
            struct process *nextProcess = ptr1->pNext->pData;
            // process->iInitialBurstTime;
            if (process->iInitialBurstTime > nextProcess->iInitialBurstTime)
            {
                swap(ptr1, ptr1->pNext);
                swapped = 1;
            }
            ptr1 = ptr1->pNext;
        }
        lPtr = ptr1;
    }
    while (swapped);
}
