// Yifei YUE 20125896 scyyy2

#include <stdlib.h>
#include "coursework.h"
#include "linkedlist.h"

struct priorityQueue {
    int priorityId;
    int counter;
    struct element *pHead;
    struct element *pTail;
};

int main()
{
    // priority queue
    struct element *pQHead = NULL;
    struct element *pQTail = NULL;

    // process queue
    struct element *pHead = NULL;
    struct element *pTail = NULL;

    // priority queue pointer
    struct element *queuePointer = NULL;

    // total response time and turn around time
    long int totalResponseTime = 0;
    long int totalTurnAroundTime = 0;

    // create a single linked list to store all the priority queues
    for (int i = 0; i < MAX_PRIORITY; i++)
    {
        struct priorityQueue *currentPQ = (struct priorityQueue *) malloc(sizeof(struct priorityQueue));
        if(currentPQ == NULL)
        {
            printf("Malloc Failed\n");
            exit(-1);
        }
        currentPQ -> priorityId = i;
        currentPQ -> pHead = NULL;
        currentPQ -> pTail = NULL;
        currentPQ -> counter = 0;
        addLast(currentPQ, &pQHead, &pQTail);
    }

    // add process to corresponding priority queue
    for (int i = 0; i < NUMBER_OF_PROCESSES; i++)
    {
        struct process * process = generateProcess();
        // set the pointer to the first priority
        queuePointer = pQHead;
        // find the corresponding priority
        for (int j = 0; j < process->iPriority; j++)
        {
            queuePointer = queuePointer->pNext;
        }
        // add all the processes to the priority queue
        struct priorityQueue *currentPQ = queuePointer->pData;
        addLast(process, &(currentPQ->pHead), &(currentPQ->pTail));
        currentPQ->counter = currentPQ->counter + 1;
    }

    // print all processes and its corresponding priority queue
    printf("PROCESS LIST:\n");

    // set the pointer to the first priority
    queuePointer = pQHead;
    while (queuePointer != NULL)
    {
        struct priorityQueue *currentPQ = queuePointer->pData;
        if (currentPQ->counter != 0)
        {
            printf("Priority %d\n", currentPQ->priorityId);
            struct element *curQueue = currentPQ->pHead;
            while (curQueue != NULL)
            {
                struct process * process = curQueue->pData;
                printf("\tProcess Id = %d, Priority = %d, Initial Burst Time = %d, Remaining Burst Time = %d\n", process->iProcessId, currentPQ->priorityId, process->iInitialBurstTime, process->iRemainingBurstTime);
                curQueue = curQueue->pNext;
            }
        }
        queuePointer = queuePointer->pNext;
    }
    printf("END\n\n");

    // set the pointer to the first priority
    queuePointer = pQHead;
    // run all processes
    while (queuePointer != NULL)
    {
        struct priorityQueue * currentPQ = queuePointer->pData;
        // if no element inside this priority queue, then point to the next
        while(currentPQ->pHead != NULL)
        {
            struct process * process = removeFirst(&(currentPQ->pHead), &(currentPQ->pTail));
            struct timeval pStartTime, pEndTime;

            runPreemptiveJob(process, &pStartTime, &pEndTime);
            printf("Process Id = %d, Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d",
                   process -> iProcessId, currentPQ->priorityId, process -> iPreviousBurstTime, process -> iRemainingBurstTime);

            if (process -> iPreviousBurstTime == process -> iInitialBurstTime)
            {
                // calculate response time
                long int responseTime = getDifferenceInMilliSeconds(process -> oTimeCreated, pStartTime);
                printf(", Response Time = %ld", responseTime);
                totalResponseTime += responseTime;
            }
            if (process -> iRemainingBurstTime == 0)
            {
                // calculate turn around time
                long int turnAroundTime = getDifferenceInMilliSeconds(process -> oTimeCreated, pEndTime);
                printf(", Turnaround Time = %ld", turnAroundTime);
                totalTurnAroundTime += turnAroundTime;
                // deallocate the memory
                free(process);
            }
            else
            {
                // add the process to the end of queue
                addLast(process, &(currentPQ->pHead), &(currentPQ->pTail));
            }
            printf("\n");
        }
        queuePointer = queuePointer->pNext;
    }

    // calculate average response time and turn around time
    printf("Average response time = %lf\n", (double) totalResponseTime / NUMBER_OF_PROCESSES);
    printf("Average turn around time = %lf\n", (double) totalTurnAroundTime / NUMBER_OF_PROCESSES);

    return 0;
}
