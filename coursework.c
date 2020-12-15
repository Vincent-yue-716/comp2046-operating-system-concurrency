/**************************************************************************************************************************
			DO NOT CHANGE THE CONTENTS OF THIS FILE FOR YOUR COURSEWORK. ONLY WORK WITH THE OFFICIAL VERSION
**************************************************************************************************************************/

#include <stdlib.h>
#include <sys/time.h>
#include "coursework.h"
#include <stdio.h>

int iPid = 0;

/*
 * Function that generates a single job and initialises the fields. Process will have an increasing job id, reflecting the order in which they were created.
 * Note that the objects returned are allocated in dynamic memory, and that the caller is responsible for free-ing the memory when it is no longer in use.
 *
 * REMARK: note that the random generator will generate a fixed sequence of random numbers. I.e., every time the code is run, the times that are generated will be the same, although the individual 
 * numbers themselves are "random". This is achieved by seeding the generator (by default), and is done to facilitate debugging if necessary.
 */

struct process * generateProcess()
{
	struct process * oTemp = (struct process *) malloc (sizeof(struct process));
	if(oTemp == NULL)
	{
		printf("Malloc Failed\n");
		exit(-1);
	}
	oTemp->iProcessId = iPid++;
	oTemp->iRemainingBurstTime = oTemp->iPreviousBurstTime = oTemp->iInitialBurstTime = (rand() % MAX_BURST_TIME) + 1;
	oTemp->iPriority = (rand() % MAX_PRIORITY);
	gettimeofday(&(oTemp->oTimeCreated), NULL);
	gettimeofday(&(oTemp->oMostRecentTime), NULL);
	return oTemp;
}

/*
 * Function returning the time difference in milliseconds between the two time stamps, with start being the earlier time, and end being the later time.
 */
long int getDifferenceInMilliSeconds(struct timeval start, struct timeval end)
{
	long int iSeconds = end.tv_sec - start.tv_sec;
	long int iUSeconds = end.tv_usec - start.tv_usec;
 	long int mtime = (iSeconds * 1000 + iUSeconds / 1000.0);
	return mtime;
}

/*
 * Function to call when simulating a non-preemptive job. This function will:
 * - change the state to running
 * - run the job
 * - set the burst time of the process to 0
 * - update the state to finished
 */
void runNonPreemptiveJob(struct process * oTemp, struct timeval * oStartTime, struct timeval * oEndTime)
{
	runProcess(oTemp->iInitialBurstTime, oStartTime, oEndTime);
	oTemp->iRemainingBurstTime = 0;
}

/*
 * Function to call when simulating a NON-BLOCKING round robin job. This function will:
 * - calculate the (remaining) burst time
 * - set the state to running
 * - run the job
 * - reduce the burst time of the process with the time that it ran
 * - change the state to finished if the burst time reaches 0, set it to ready if the process used its entire time slice and was taken off the CPU
 */

void runPreemptiveJob(struct process * oTemp, struct timeval * oStartTime, struct timeval * oEndTime)
{
	int iBurstTime = oTemp->iRemainingBurstTime > TIME_SLICE ? TIME_SLICE : oTemp->iRemainingBurstTime;
	runProcess(iBurstTime, oStartTime, oEndTime);
	oTemp->iPreviousBurstTime = oTemp->iRemainingBurstTime;
	oTemp->iRemainingBurstTime -= iBurstTime;
	gettimeofday(&oTemp->oMostRecentTime, NULL);
}

/*
 * Simulates the job running on a CPU for a number of milli seconds
 */
void runProcess(int iBurstTime, struct timeval * oStartTime, struct timeval * oEndTime)
{
	struct timeval oCurrent;
	long int iDifference = 0;
	gettimeofday(oStartTime, NULL);
	do
	{
		gettimeofday(&oCurrent, NULL);
		iDifference = getDifferenceInMilliSeconds((*oStartTime), oCurrent);
	} while(iDifference < iBurstTime);
	gettimeofday(oEndTime, NULL);
}
