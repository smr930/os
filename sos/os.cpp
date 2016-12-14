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
std::map<long, long> FREESPACETABLE; // address and size pair
std::queue<long> readyq; //ready queue points to PCBs
std::queue<long> ioQueue;


void siodisk(int jobnum);
void initFST();
void addJobToJobtable (long jobNumber, long priority, long jobSize, long maxCpuTime, long currTime);
void printJobtable();
void printFST();
void swapper(long jobNumber);

//prototypes
void addJobtoJobTable (long , long , long , long , long );
void scheduler(void);
int findJob(long);
void addJobToFST(long);
void clearSpace (long, long);
void clearSpace (long);
void clear_from_readyq(long);
int findFreeSpace(long);
void block(void);
void requestIO(void);
void dispatcher(long , int );

/*
holds the job currently running or job which was just running
used in terminate service call
*/
long curr_job_num;
//long a;
//long p[5];

/*
Channel commands siodisk and siodrum are made available to you by the simulator.
siodisk has one argument: job number, of type int and passed by value.
siodrum has four arguments, all of type int and passed by value:
first argument is job number;
second argument is job size;
third argument is starting core address;
fourth argument is interpreted as follows:
1 => move from core (memory) to drum
0 => move from drum to core (memory)
*/
void siodrum(long jobnum, long jobsize, long coreaddress, long direction);




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

     //long& jobnum = p[1];
     cout << "in crint, job num = " + p[1] << endl;
     addJobtoJobTable(p[1] , p[2], p[3],p[4], p[5]);
     swapper(p[1]);
     scheduler();
     return;

}

// Handles requests for swapping, starts a drum swap (using SOS function Siodrum), handles drum
// interrupts (Drmint), and selects which job currently on the drum should be swapped into memory.
void swapper(long jobNumber)
{
    cout << "in swapper, job num = " + jobNumber << endl;
    int jobLocation = findJob(jobNumber);
    int direction = JOBTABLE[jobLocation].getDirection();

    // direction = 0, swap from drum to memory
    if (direction == 0)
    {
        addJobToFST(jobNumber);
        readyq.push (JOBTABLE[jobLocation].getJobNumber() );
    }

    // direction = 1, swap from memory to drum
    else if (direction == 1)
    {
        if (JOBTABLE[jobLocation].getIORequest() != true || JOBTABLE[jobLocation].isLatched() != true)
        {
            if (!readyq.empty())
                readyq.pop();

            if (!ioQueue.empty())
                ioQueue.pop();


            siodrum(JOBTABLE[jobLocation].getJobNumber(), JOBTABLE[jobLocation].getJobSize(),
                     JOBTABLE[jobLocation].getAddress(), 1);
            JOBTABLE[jobLocation].setInMemory(false);
            clearSpace(jobLocation);
        }
    }
     return;
}

/*
1 of 2 helper function for swapper
clears range
*/
void clearSpace (long startIndex, long endIndex)
{
    for (long i = startIndex; i < endIndex; i++)
    {
        clearSpace(i);
    }
}


/*
1 of 2 clearSpace helper functions for swapper
index represents the starting address!
*/
void clearSpace (long index)
{
    FREESPACETABLE[index] = 0;

}




/*
function finds free space and bookkeeps
*/
void addJobToFST(long jobNumber)
{
    cout << "\n---------------------" << endl;
    cout << "Inside addJobToFST(): " << endl;

    long currJobSize = JOBTABLE[jobNumber-1].getJobSize();
    long currJobAddress = findFreeSpace(currJobSize);
    JOBTABLE[jobNumber-1].SetAddress(currJobAddress);
    JOBTABLE[jobNumber-1].setInMemory(true);

    FREESPACETABLE[currJobAddress] = currJobSize;

    cout << "jobSize: " << currJobSize << endl;
    cout << "jobAddr: " << currJobAddress << endl;
    cout << "FREESPACETABLE[" << currJobAddress << "] = " << currJobSize << endl;
}


/*
helper function for swapper & terminate service call
returns index of job in JOBTABLE
*/
int findJob(long jobNum)
{
    for(int i = 0; i < JOBTABLE.size(); i++)
    {
        if(JOBTABLE[i].getJobNumber() == jobNum)
            return i;
    }

    cout << "findJob(" << jobNum << "): " << "Job not found!" << endl;
    return -1;
}

/*
function adds a new job into the JOBTABLE
*/
void addJobtoJobTable (long jobnum, long priority, long jobsize, long maxCPUtime, long currtime)
{
    cout << "in add job to job table, job num = " + jobnum << endl;
    Job newjob(jobnum, jobsize, maxCPUtime, currtime, priority);
    JOBTABLE.push_back(newjob); //add job entry to end of list
    return;
}


void Dskint (long &a, long p[])
{
 // Disk interrupt.
 // At call: p [5] = current time
}

void Drmint (long &a, long p[])
{
 // Drum interrupt.
 // At call: p [5] = current time

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
 // a = 7 => job wants to be blocked until all its pending
 // I/O requests are completed

     switch (a){
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


//Job findJobinJOBTABLE(int jobnum)
//{
//    for(int i = 0; i < JOBTABLE.size(); i++){
//        if(JOBTABLE[i].getJobNumber() == jobnum)
//            return JOBTABLE[i];
//    }
//    cout << "ERROR FINDING JOB IN JOBTABLE";
//    return;
//
//}



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
    Job newJob(jobNumber, priority, jobSize, maxCpuTime, currTime);

    // Add the temp vector to the JOBTABLE
    JOBTABLE.push_back(newJob);
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
    cout << " -- FST -- " << endl;
    cout << "Addr  Size" << endl;
    map<long, long>::iterator fstIter;

    for (fstIter = FREESPACETABLE.begin(); fstIter != FREESPACETABLE.end(); fstIter++)
    {
        cout << fstIter->first << "  =>  " << fstIter->second << endl;
    }
}

// Find space based on job size
int findFreeSpace(long jobSize)
{
    map<long, long>::iterator fstIter;

    for (fstIter = FREESPACETABLE.begin(); fstIter != FREESPACETABLE.end(); fstIter++)
    {
        if (fstIter->second >= jobSize)
        {
            return fstIter->first;
        }
    }

    // if there is no free space
    return -1;
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
    a = 2; //run a job
    p[2] = JOBTABLE[job_idx].getAddress();
    p[3] = JOBTABLE[job_idx].getJobSize();
    p[4] = timequantum;
    return;
}

void requestIO(){
    JOBTABLE[curr_job_num].setIORequest(JOBTABLE[curr_job_num].getIORequest() + 1);
    ioQueue.push(curr_job_num);
    return;
}
