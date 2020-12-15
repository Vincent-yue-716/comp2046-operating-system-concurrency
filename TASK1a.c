// Yifei YUE 20125896 scyyy2

#include "coursework.h"
#include "linkedlist.h"
#include <stdlib.h>

void bubbleSort(struct element * start);
void swap(struct element *a, struct element *b);

int main()
{
    // queue of processes
    struct element * pHead = NULL;
    struct element * pTail = NULL;

    // total response time and turn around time
    long int totalResponseTime = 0;
    long int totalTurnAroundTime = 0;

    // create all processes and add to linked list
    for (int i = 0; i < NUMBER_OF_PROCESSES; i++) {
        struct process * process = generateProcess();
        addLast(process, &pHead, &pTail);
    }

    bubbleSort(pHead);

    while (pHead != NULL) {
        // take the first process
        struct process * process = removeFirst(&pHead, &pTail);

        // complete the process
        struct timeval pStartTime, pEndTime;
        runNonPreemptiveJob(process, &pStartTime, &pEndTime);

        // calculate response time and turn around time
        long int responseTime = getDifferenceInMilliSeconds(process -> oTimeCreated, pStartTime);
        totalResponseTime += responseTime;
        long int turnAroundTime = getDifferenceInMilliSeconds(process -> oTimeCreated, pEndTime);
        totalTurnAroundTime += turnAroundTime;

        printf("Process Id = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Response Time = %ld, Turnaround Time = %ld\n",
               process -> iProcessId, process -> iPreviousBurstTime, process -> iRemainingBurstTime, responseTime, turnAroundTime);
        // deallocate memory
        free(process);
    }
    // calculate average response time and turn around time
    printf("Average response time = %lf\n", (double) totalResponseTime / NUMBER_OF_PROCESSES);
    printf("Average turn around time = %lf\n", (double) totalTurnAroundTime / NUMBER_OF_PROCESSES);

    return 0;
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
    if (start == NULL)
    {
        return;
    }
    do
    {
        swapped = 0;
        ptr1 = start;
        while(ptr1->pNext != lPtr)
        {
            struct process *process = ptr1->pData;
            struct process *nextProcess = ptr1->pNext->pData;
            // process->iInitialBurstTime;
            if(process->iInitialBurstTime > nextProcess->iInitialBurstTime)
            {
                swap(ptr1, ptr1->pNext);
                swapped = 1;
            }
            ptr1 = ptr1->pNext;
        }
        lPtr = ptr1;
    }
    while(swapped);
}
