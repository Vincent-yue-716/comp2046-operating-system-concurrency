// Yifei YUE 20125896 scyyy2

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include "coursework.h"
#include "linkedlist.h"


struct buffer{
    struct element *pHead;
    struct element *pTail;
    int items;
};

struct priorityQueue
{
    int priorityId;
    int counter;
    struct element *qHeader;
    struct element *qTail;
};

// the binary semaphore and counting semaphores
sem_t sync, full, empty;

struct buffer * processBuffer;
long int totalResponseTime = 0, totalTurnAroundTime = 0;
int isComplete = 0;

// handling conditions
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t produce, consume = PTHREAD_COND_INITIALIZER;


void * consumer(void * id);
void * producer(void * id);

int main()
{
    sem_init(&sync, 0, 1);
    sem_init(&empty, 0, MAX_BUFFER_SIZE);
    sem_init(&full, 0, 0);

    // buffer for containing processes
    processBuffer = (struct buffer *)malloc(sizeof(struct buffer));
    processBuffer->pHead = NULL;
    processBuffer->pTail = NULL;
    processBuffer->items = 0;

    // bounded buffer for all priority queues
    for(int i = 0; i < MAX_PRIORITY; i++)
    {
        struct priorityQueue *pQueue = (struct priorityQueue *) malloc(sizeof(struct priorityQueue));
        pQueue->priorityId = i;
        pQueue->counter = 0;
        pQueue->qHeader = NULL;
        pQueue->qTail = NULL;
        addLast(pQueue, &processBuffer->pHead, &processBuffer->pTail);
    }

    pthread_t tProducers[NUMBER_OF_PRODUCERS];
    pthread_t tConsumers[NUMBER_OF_CONSUMERS];

    // id of each producer and consumer
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
    pthread_mutex_destroy(&mtx);
    free(processBuffer);

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
        pthread_mutex_lock(&mtx);
        // if created job is less than MAX_NUMBER_OF_JOBS
        if(isComplete != 1)
        {
            // if produced jobs are more than MAX_BUFFER_SIZE
            if(processBuffer->items < MAX_BUFFER_SIZE)
            {
                struct process * process = generateProcess();

                // find the corresponding priority queue
                struct element * curPriority = processBuffer->pHead;
                for(int i = 0; i < process->iPriority; i++)
                {
                    curPriority = curPriority->pNext;
                }
                // add the process to corresponding queue
                struct priorityQueue *currentPQ = curPriority->pData;

                addLast(process, &(currentPQ->qHeader), &(currentPQ->qTail));

                // increase the counter of queue and buffer
                currentPQ->counter += 1;
                processBuffer->items++;

                int producedNum = process->iProcessId + 1;
                printf("Producer = %d, Items Produced = %d, New Process Id = %d, Burst Time = %d\n",
                       producerId, producedNum, process->iProcessId, process->iInitialBurstTime);

                // if already produced MAX_NUMBER_OF_JOBS
                if(producedNum >= MAX_NUMBER_OF_JOBS)
                {
                    isComplete = 1;
                }
                // wake up the consumer
                pthread_cond_broadcast(&consume);
            }
            else
            {
                // if process in the buffer is euqal to MAX_BUFFER_SIZE
                pthread_cond_wait(&produce, &mtx);
            }
            pthread_mutex_unlock(&mtx);
        }
        else
        {
            pthread_mutex_unlock(&mtx);
            break;
        }
    }
    pthread_exit(NULL);
}


void * consumer(void * id)
{
    int consumerId = *((int *)id);
    while(1)
    {
        struct process *p = NULL;
        struct priorityQueue *currentPQ = NULL;
        struct element *pQueue = NULL;
        struct timeval startTime, endTime;
        pthread_mutex_lock(&mtx);

        // finish consuming all of the processes
        if(processBuffer->items == 0 && isComplete == 1)
        {
            pthread_mutex_unlock(&mtx);
            break;
        }
        else
        {
            // when buffer is empty
            if(processBuffer->items == 0)
            {
                pthread_cond_wait(&consume, &mtx);
                pthread_mutex_unlock(&mtx);
            }
            else
            {
                pQueue = processBuffer->pHead;
                while(pQueue != NULL)
                {
                    currentPQ = pQueue->pData;
                    if(currentPQ->counter == 0)
                    {
                        pQueue = pQueue->pNext;
                    }
                    else
                    {
                        p = removeFirst(&(currentPQ->qHeader), &(currentPQ->qTail));
                        currentPQ->counter -= 1;
                        break;
                    }
                }
                // wake up the producer
                pthread_cond_broadcast(&produce);
                pthread_mutex_unlock(&mtx);
                // if the process is not null
                if(p != NULL)
                {
                    // run the process and calculate time
                    struct process * curProcess = p;
                    runPreemptiveJob(curProcess, &startTime, &endTime);
                    // when the burst time of the process is larger than TIME_SLICE
                    if(curProcess->iPreviousBurstTime == curProcess->iInitialBurstTime && curProcess->iInitialBurstTime > TIME_SLICE)
                    {
                        long int responseTime = getDifferenceInMilliSeconds(p->oTimeCreated, startTime);
                        printf("Consumer = %d, Process Id = %d, Priority = %d, Previous Burst Time = %d, New Burst Time = %d, Respond Time = %d\n",
                               consumerId, curProcess->iProcessId, currentPQ->priorityId, curProcess->iPreviousBurstTime, curProcess->iRemainingBurstTime, responseTime);
                        totalResponseTime += responseTime;
                    }
                    else if(curProcess->iRemainingBurstTime == 0)
                    {
                        long int turnAroundTime = getDifferenceInMilliSeconds(p->oTimeCreated, endTime);
                        printf("Consumer = %d, Process Id = %d, Priority = %d, Previous Burst Time = %d, New Burst Time = %d, TurnaroundTime = %d\n",
                               consumerId, curProcess->iProcessId, currentPQ->priorityId, curProcess->iPreviousBurstTime, curProcess->iRemainingBurstTime, turnAroundTime);
                        totalTurnAroundTime += turnAroundTime;
                    }
                    else if(curProcess->iPreviousBurstTime == curProcess->iInitialBurstTime && curProcess->iInitialBurstTime > 0)
                    {
                        long int responseTime = getDifferenceInMilliSeconds(p->oTimeCreated, startTime);
                        long int turnAroundTime = getDifferenceInMilliSeconds(p->oTimeCreated, endTime);
                        printf("Consumer = %d, Process Id = %d, Priority = %d, Previous Burst Time = %d, New Burst Time = %d, Respond Time = %d, TurnaroundTime = %d\n",
                               consumerId, curProcess->iProcessId, currentPQ->priorityId, curProcess->iPreviousBurstTime, curProcess->iRemainingBurstTime, responseTime, turnAroundTime);
                        totalResponseTime += responseTime;
                        totalTurnAroundTime += turnAroundTime;
                    }
                    else
                    {
                        printf("Consumer = %d, Process Id = %d, Priority = %d, Previous Burst Time = %d, New Burst Time = %d\n",
                               consumerId, curProcess->iProcessId, currentPQ->priorityId, curProcess->iPreviousBurstTime, curProcess->iRemainingBurstTime);
                    }

                    // if the process is not finished, then add to the last of the queue
                    if(p->iRemainingBurstTime > 0)
                    {
                        addLast(p, &(currentPQ->qHeader), &(currentPQ->qTail));
                        currentPQ->counter += 1;
                    }
                    else
                    {
                        // reduce item in buffer and free the process
                        processBuffer->items--;
                        free(p);
                    }
                }
            }
        }
    }
    pthread_exit(NULL);
}
