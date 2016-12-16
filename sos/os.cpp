#include <iostream>
#include <list>
#include <vector>
#include <map>
#include <queue>
#include <stack>
#include "Job.h"
#include <stdlib.h> // defines EXIT_FAILURE

using namespace std;

// Data structures and global variables
std::vector<Job> JOBTABLE;
std::map<int, int> FREESPACETABLE; // address and size pair
std::queue<long> readyq;
std::queue<long> ioQueue;

//prototypes
void siodisk(long jobnum);
void siodrum(long jobnum, long jobsize, long coreaddress, long direction);
void initFST();
void addJobToJobtable (long jobNumber, long priority, long jobSize, long maxCpuTime, long currTime);
void removeJobFromJobTable (long jobNumber);
void addJobToFST(long jobNumber);
void printJobtable();
void printFST();
void printQueue(std::queue<long> myQueue, string name);
int  findFreeSpace(int jobSize);
void clearSpace(int index);
void clearSpace(int startIndex, int endIndex);
void clear_from_readyq(long);
int  findJob(long jobNum);
void swapper(long jobNumber);
void scheduler(void);
void block(void);
void requestIO(void);
void dispatcher(long , int );

/*
holds the job currently running or job which was just running
used in terminate service call
*/
long curr_job_num;

//void siodrum(int jobnum, int jobsize, int coreaddress, int direction){
 // Channel commands siodisk and siodrum are made available to you by the simulator.
 // siodisk has one argument: job number, of type int and passed by value.
 // siodrum has four arguments, all of type int and passed by value:
 // first argument is job number;
 // second argument is job size;
 // third argument is starting core address;
 // fourth argument is interpreted as follows:
 // 1 => move from core (memory) to drum
 // 0 => move from drum to core (memory)
//}

void ontrace(); // called without arguments
void offtrace(); // called without arguments
 // The 2 trace procedures allow you to turn the tracing mechanism on and off.
 // The default value is off. WARNING: ontrace produces a blow-by-blow description
 // of each event and results in an extremely large amount of output.
 // It should be used only as an aid in debugging.
 // Even with the trace off, performance statistics are
 // generated at regular intervals and a diagnostic message appears in case of a crash.
 // In either case, your OS need not print anything.

void startup()
{
 // Allows initialization of static system variables declared above.
 // Called once at start of the simulation.
    ontrace();
    initFST();
    curr_job_num = -1;
}

// INTERRUPT HANDLERS
    // The following 5 functions are the interrupt handlers. The arguments
    // passed from the environment are detailed with each function below.
    // See RUNNING A JOB, below, for additional information

void Crint (long &a, long p[])
{
    // Indicates the arrival of a new job on the drum.
    // At call: p [1] = job number
    // p [2] = priority
    // p [3] = job size, K bytes
    // p [4] = max CPU time allowed for job
    // p [5] = current time

    addJobToJobtable(p[1], p[2], p[3], p[4], p[5]);
    swapper(p[1]);
    scheduler();

    //printJobtable();
    //printFST();
}

void Dskint (long &a, long p[])
{
    // Disk interrupt.
    // At call: p [5] = current time
    int ioJobIndex = findJob(ioQueue.front());
    JOBTABLE[ioJobIndex].setIsDoingIO(false);
    JOBTABLE[ioJobIndex].setIORequest(JOBTABLE[ioJobIndex].getIORequest() - 1);
    ioQueue.pop();

    //send next job from io queue to disk
    ioJobIndex = ioQueue.front();
    JOBTABLE[ioJobIndex].setIsDoingIO(true);
    siodisk(ioQueue.front()); //call siodisk to swap job to disk
}

void Drmint (long &a, long p[])
{
    // Drum interrupt.
    // At call: p [5] = current time

    bool mem;
    int jobIndex = findJob(curr_job_num);
    mem = JOBTABLE[jobIndex].isInMemory();
    JOBTABLE[jobIndex].setInMemory(!mem);

    if(mem == true)
        readyq.push(JOBTABLE[jobIndex].getJobNumber());

    ///swapper(JOBTABLE[jobIndex].getJobNumber()); // Gives error: Job in already in core
    scheduler();
}

void Tro (long &a, long p[])
{
    // Timer-Run-Out.
    // At call: p [5] = current time

}
void Svc (long &a, long p[])
{
     // Supervisor call from user program.
     // At call: p [5] = current time
     // a = 5 => job has terminated
     // a = 6 => job requests disk i/o
     // a = 7 => job wants to be blocked un 5til all its pending
     // I/O requests are completed

      switch (a) {
          case 5: terminate();
                  break;
          case 6: requestIO();
                  break;
          case 7: block();
                  break;
          default: cout << "there was error with service request\n";
                  break;
      }
     return;

}

/*
called when the last running job requests service
sets block flag so that job may not run nor be terminated until all IO requests are met
*/
void block()
{
    int job_idx = findJob(curr_job_num);
    JOBTABLE[job_idx].setBlocked(true);
    return;
}

/*
a job requests to be terminated if it has exceeded its max CPU time or an error occurred
head of ready queue represents the job that asked to be terminated
*/
void terminate()
{
     int job_idx = findJob(curr_job_num);
     //set flag to terminate once unblocked
     if(JOBTABLE[job_idx].isBlocked()){
          JOBTABLE[job_idx].setset_to_terminate(true);
          return;
     }
     //if exceeded CPU time, TODO
     //if error occurred, TODO
     clearSpace(JOBTABLE[job_idx].getAddress());
     clear_from_readyq(job_idx);
}

void clear_from_readyq(long job_num)
{
    if(readyq.empty()){
        cout << "ERROR, EMPTY Q: cannot clear entry from readyq" << endl;
        exit(EXIT_FAILURE);
    }

    //search for element and save the elements you pass through
    std::stack<long> tmp;
    long curr = readyq.front();
    while((!readyq.empty()) && curr != job_num){
        readyq.pop();
        tmp.push(curr);
        curr = readyq.front();
    }

    //if got to this point either found element OR queue has been emptied
    if(readyq.empty()){
        cout << "ERROR, EMPTY Q: cannot clear entry from readyq" << endl;
        exit(-1);
    }

    readyq.pop(); //remove job from readyq!

    while(!tmp.empty()){
        curr = tmp.top();
        tmp.pop();
        readyq.push(curr);
    }
    return;
}

// Create 100 elements in FST
void initFST()
{
    for (int i = 0; i < 100; i++)
    {
        FREESPACETABLE.insert(pair<int, int> (i, 0));
    }
}

void addJobToJobtable (long jobNumber, long priority, long jobSize, long maxCpuTime, long currTime)
{
    // Place the parameters in a temp job container
    Job newJob(jobNumber, jobSize, maxCpuTime, currTime, priority);

    // Add the temp vector to the JOBTABLE
    JOBTABLE.push_back(newJob);
}

void removeJobFromJobTable (long jobNumber)
{
	long jobLocation = findJob(jobNumber);
	JOBTABLE.erase(JOBTABLE.begin() + jobLocation);
}

void printJobtable()
{
    cout << "\n -- JOBTABLE --" << endl;
    for (int i = 0; i < JOBTABLE.size(); i++)
    {
        cout << "Job Num: " << JOBTABLE[i].getJobNumber() << '\t'
             << "Job Size: " << JOBTABLE[i].getJobSize() << '\t'
             << endl;
    }
}

void printFST()
{
    cout << "\n -- FREE SPACE TABLE -- " << endl;
    cout << "Addr   Size" << endl;
    map<int, int>::iterator fstIter;
    int i = 0;

    for (fstIter = FREESPACETABLE.begin(); fstIter != FREESPACETABLE.end(); fstIter++)
    {
        if (i < 10)
           cout << fstIter->first << "   =>  " << fstIter->second << endl;
        else
            cout << fstIter->first << "  =>  " << fstIter->second << endl;

        i++;
    }
}

void printQueue(std::queue<long> myQueue, string name)
{
    cout << "\n -- " << name << " --" << endl;
    std::queue<long> tempQ = myQueue;
    long tempJob;
    long jobCount = 0;

    if (tempQ.empty())
    {
        cout << "The queue is empty\n" << endl;
        return;
    }

    while (!tempQ.empty())
    {
        tempJob = tempQ.front();
        cout << "\tJob Num: " << tempJob << endl;
        tempQ.pop();
        jobCount++;
    }
    cout << "There are " << jobCount << " job in the queue!\n" << endl;
}

// Find space based on job size
int findFreeSpace(int jobSize)
{
    int contigous = 0;
    int index = 0;
    bool foundIndex = false;
    map<int, int>::iterator fstIter;

    for (fstIter = FREESPACETABLE.begin(); fstIter != FREESPACETABLE.end(); fstIter++)
    {
        if (fstIter->second == 0)
        {
            contigous++;
            if (!foundIndex)
                index = fstIter->first;
                foundIndex = true;
        }
        else
            continue;
    }

    if (contigous >= jobSize)
    {
        //cout << "contigous: " << contigous << endl;
        return index;
    }

    // if there is no free space
    cout << "findFreeSpace(" << jobSize << "): " << "No free space found!" << endl;
    return -1;
}

void addJobToFST(long jobNumber)
{
    cout << "\n---------------------" << endl;
    cout << "Inside addJobToFST(): " << endl;

    long currJobSize = JOBTABLE[jobNumber-1].getJobSize();
    long currJobAddress = findFreeSpace(currJobSize);
    JOBTABLE[jobNumber-1].SetAddress(currJobAddress);

    FREESPACETABLE[currJobAddress] = currJobSize;

    cout << "jobSize: " << currJobSize << endl;
    cout << "jobAddr: " << currJobAddress << endl;
    cout << "FREESPACETABLE[" << currJobAddress << "] = " << currJobSize << endl;
}

int findJob(long jobNum)
{
    for(int i = 0; i < JOBTABLE.size(); i++)
    {
        if(JOBTABLE[i].getJobNumber() == jobNum)
            return i ;
    }

    cout << "findJob(" << jobNum << "): " << "Job not found!" << endl;
    return -1;
}

void clearSpace (int index)
{
    FREESPACETABLE[index] = 0;

}

void clearSpace (int startIndex, int endIndex)
{
    for (int i = startIndex; i < endIndex; i++)
    {
        clearSpace(i);
    }
}

// Handles requests for swapping, starts a drum swap (using SOS function Siodrum), handles drum
// interrupts (Drmint), and selects which job currently on the drum should be swapped into memory.
void swapper(long jobNumber)
{
    int jobLocation = findJob(jobNumber);
    int direction = JOBTABLE[jobLocation].getDirection();
    bool foundSpace = (findFreeSpace(JOBTABLE[jobLocation].getJobSize()) != -1 ? true : false);

    // direction = 0, swap from drum to memory
    if (direction == 0)
    {
        if (foundSpace)
        {
            addJobToFST(jobNumber);
            curr_job_num = jobNumber;
            readyq.push (JOBTABLE[jobLocation].getJobNumber());
            JOBTABLE[jobLocation].setInMemory(true);
            //printQueue(readyq, "READY QUEUE");
            siodrum(JOBTABLE[jobLocation].getJobNumber(), JOBTABLE[jobLocation].getJobSize(),
                         JOBTABLE[jobLocation].getAddress(), 0);
        }
        // a job can't be swapped to memory because there is no space in FST
        else
        {
            return;
        }
    }

    // direction = 1, swap from memory to drum
    else if (direction == 1)
    {
        if (JOBTABLE[jobLocation].getIsDoingIO() != true || JOBTABLE[jobLocation].isLatched() != true)
        {
            if (!readyq.empty())
            {
                if (readyq.front() == jobNumber)
                    readyq.pop();
            }

            siodrum(JOBTABLE[jobLocation].getJobNumber(), JOBTABLE[jobLocation].getJobSize(),
                     JOBTABLE[jobLocation].getAddress(), 1);
            JOBTABLE[jobLocation].setInMemory(false);
            removeJobFromJobTable (jobNumber);
            clearSpace(jobLocation);
        }
    }
}

/*
scheduler uses round robin implementation:
picks the next job to run from the ready queue
and sets time quantum
*/
void scheduler()
{
    //if there is no job to run
    if(readyq.empty()){
         return;
    }

    int timequantum = 2;
    long curr_q_entry = readyq.front();
    std::stack<long> tmp;
    /*
    look for first job that is not blocked
    store the elements you go through to put back after search
    */
    int job_idx = findJob(curr_q_entry);
    while((!readyq.empty()) && (JOBTABLE[job_idx].isBlocked() || JOBTABLE[job_idx].getIsDoingIO() ) ) {
         tmp.push(curr_q_entry);
         readyq.pop();
         curr_q_entry = readyq.front();
    }
    //return after all jobs are put into original place given all jobs are blocked
    if(readyq.empty()){
         while(!tmp.empty()){
            readyq.push(tmp.top());
            tmp.pop();
         }
         return;
    }
    //only gets to this point if there is a job to run
    dispatcher(timequantum, job_idx );
    return;
}

/*
dispatcher receives time quantum
and the index used to access the next running job's info in the JOBTABLE
*/
void dispatcher(long timequantum, int job_idx)
{
    // a and p needs to be passed in
    /*
    a = 2; //run a job
    p[2] = JOBTABLE[job_idx].getAddress();
    p[3] = JOBTABLE[job_idx].getJobSize();
    p[4] = timequantum;
    */
    return;
}

void requestIO(){
    JOBTABLE[curr_job_num].setIORequest(JOBTABLE[curr_job_num].getIORequest() + 1);
    ioQueue.push(curr_job_num);
    return;
}
