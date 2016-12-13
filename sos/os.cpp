#include <iostream>
#include <list>
#include <vector>
#include <map>
#include <queue>
#include "Job.h"

using namespace std;


// Data structures and global variables
std::vector<Job> JOBTABLE;
std::map<int, int> FREESPACETABLE; // address and size pair
std::queue<int> readyq; //ready queue points to PCBs
std::queue<int*> ioQueue;


void siodisk(int jobnum);
void initFST();
void addJobToJobtable (long jobNumber, long priority, long jobSize, long maxCpuTime, long currTime);
void printJobtable();
void printFST();
void swapper(long jobNumber);

//prototypes
void addJobtoJobTable (long& , long& , long& , long& , long& );
void scheduler(void);
int findJob(long);
void addJobToFST(long);
//Job* makePtrtoJob(Job&);
Job& makePtrtoJob(Job& job);
void clearSpace (int ,int );
void clearSpace (int );


void siodrum(int jobnum, int jobsize, int coreaddress, int direction){
 // Channel commands siodisk and siodrum are made available to you by the simulator.
 // siodisk has one argument: job number, of type int and passed by value.
 // siodrum has four arguments, all of type int and passed by value:
 // first argument is job number;
 // second argument is job size;
 // third argument is starting core address;
 // fourth argument is interpreted as follows:
 // 1 => move from core (memory) to drum
 // 0 => move from drum to core (memory)


}

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

     long jobnum = p[1];
     addJobtoJobTable(jobnum , p[2], p[3],p[4], p[5]);
     swapper(jobnum);
     scheduler();
     return;

}

// Handles requests for swapping, starts a drum swap (using SOS function Siodrum), handles drum
// interrupts (Drmint), and selects which job currently on the drum should be swapped into memory.
void swapper(long jobNumber)
{
    int jobLocation = findJob(jobNumber);
    int direction = JOBTABLE[jobLocation].getDirection();

    // direction = 0, swap from drum to memory
    if (direction == 0)
    {
        addJobToFST(jobNumber);
        Job *newJobinMem = makePtrtoJob(JOBTABLE[jobLocation]); //push ptr to job table entry onto ready queue
        readyq.push (newJobinMem);
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
void clearSpace (int startIndex, int endIndex)
{
    for (int i = startIndex; i < endIndex; i++)
    {
        clearSpace(i);
    }
}

/*
1 of 2 clearSpace helper functions for swapper
clears specific index
*/
void clearSpace (int index)
{
    FREESPACETABLE[index] = 0;

}

/*
helper function for swapper
returns pointer to job
*/
Job& makePtrtoJob(Job& job)
{
    Job *jobPtr = job;
    return jobPtr;
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
helper function for swapper
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
void addJobtoJobTable (long& jobnum, long& priority, long& jobsize, long& maxCPUtime, long& currtime)
{
    Job newJobTableEntry = new Job(jobnum, jobsize, maxCPUtime, currtime, priority);
    JOBTABLE.push_back(newJobTableEntry); //add job entry to end of list
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
head of ready queue represents the job that asked to be terminated
*/
void terminate()
{
     int currJob_num = readyq.top();
     Job currJob = findJobinJOBTABLE(currJob_num);
     if(currJob.IsBlocked()){
          currJob.setset_to_terminate(true);\
          return;
     }
     clearSpace



}

Job findJobinJOBTABLE(int jobnum)
{
    for(int i = 0; i < JOBTABLE.size(); i++){
        if(JOBTABLE[i].getJobNumber() == jobnum)
            return JOBTABLE[i];
    }
    cout << "ERROR FINDING JOB IN JOBTABLE";
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
    map<int, int>::iterator fstIter;

    for (fstIter = FREESPACETABLE.begin(); fstIter != FREESPACETABLE.end(); fstIter++)
    {
        cout << fstIter->first << "  =>  " << fstIter->second << endl;
    }
}

// Find space based on job size
int findFreeSpace(int jobSize)
{
    map<int, int>::iterator fstIter;

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

void clearSpace(int index)
{
    FREESPACETABLE[index] = 0;

}


/*
scheduler uses round robin implementation:
picks the next job to run from the ready queue
and sets time quantum
*/
void scheduler()
{
    int timequantum = 2;
    Job *dummyPtr = new Job();
    Job *nextJobToRun = dummyPtr;

    //keep looking for jobs to run that are not blocked
    while(!readyq.empty()){
        readyq.top();
        while(nextJobToRun[1] == 0){
              readyq.pop();
              readyq.push();
              nextJobToRun = readyq.top();
        }
    }
    //meets this condition if queue is empty or all jobs are blocked
    if(nextJobToRun == dummyPtr){
          *a = 1; //no job to run
    } else {
         dispatcher(nextJobToRun, timequantum );
    }
}

void dispatcher(Job *nextJob, int timequantum)
{
    *a = 2; //run a job
    p[2] = nextJob->getAddress();
    p[3] = nextJob->getJobSize();
    p[4] = timequantum;
}

