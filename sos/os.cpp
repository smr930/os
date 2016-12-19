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
void swapper();
void scheduler(void);
void block(void);
void requestIO(void);
void dispatcher(long &, long []);

/*
holds the job currently running or job which was just running
used in terminate service call
*/
long curr_job_num;
/*
represents the job currently swapping or just swapped
*/
long curr_job_moving;
long a;
long p[6];
int count_swaps;

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
    count_swaps = 0;
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
    cout << "now in crint" << endl;
    cout << "a = " << a << endl;
    cout << "job num = " << p[1] << endl;
    cout << "job priority = " << p[2] << endl;
    cout << "job size = " << p[3] << endl;
    cout << "max cpu time = " << p[4] << endl;
    cout << "curr time = " << p[5] << endl;
    addJobToJobtable(p[1], p[2], p[3], p[4], p[5]);
    swapper();
    scheduler();
    return;

    //printJobtable();
    //printFST();
}

void Dskint (long &a, long p[])
{
    // Disk interrupt.
    // At call: p [5] = current time
    cout << endl << "now in dkint" << endl;
    int ioJobIndex = findJob(ioQueue.front());
    JOBTABLE[ioJobIndex].setIsDoingIO(false);
    JOBTABLE[ioJobIndex].setIORequest(JOBTABLE[ioJobIndex].getIORequest() - 1);
    ioQueue.pop();

    //send next job from io queue to disk
    ioJobIndex = ioQueue.front();
    JOBTABLE[ioJobIndex].setIsDoingIO(true);
    siodisk(ioQueue.front()); //call siodisk to swap job to disk
    swapper();
    scheduler();
}

void Drmint (long &a, long p[])
{
    // Drum interrupt.
    // At call: p [5] = current time
    cout << endl << "in drmint" << endl;
    cout << "bookmarking job num " << curr_job_moving << endl;
    //bool mem;
    int jobIndex = findJob(curr_job_moving);
    //mem = JOBTABLE[jobIndex].isInMemory();
    JOBTABLE[jobIndex].setInMemory(!JOBTABLE[jobIndex].isInMemory());
    //if moved from core to drum
    if(!(JOBTABLE[jobIndex].isInMemory()))
    {
         removeJobFromJobTable (JOBTABLE[jobIndex].getJobNumber());
         clearSpace(jobIndex);
         cout << "clearing job num " << curr_job_moving << " from readyq" << endl;
         clear_from_readyq(curr_job_moving);
         JOBTABLE[jobIndex].setDirection(0);

    } else if(JOBTABLE[jobIndex].isInMemory())
    {
        cout << "pushing onto readyq" << endl;
        readyq.push(JOBTABLE[jobIndex].getJobNumber());
        JOBTABLE[jobIndex].setDirection(1);

    }
    swapper(); // Gives error: Job in already in core
    scheduler();
    return;
}

void Tro (long &a, long p[])
{
    // Timer-Run-Out.
    // At call: p [5] = current time
    cout << endl << "in Tro" << endl;
    terminate();
    swapper();
    scheduler();

}
void Svc (long &a, long p[])
{
     // Supervisor call from user program.
     // At call: p [5] = current time
     // a = 5 => job has terminated
     // a = 6 => job requests disk i/o
     // a = 7 => job wants to be blocked un 5til all its pending
     // I/O requests are completed
      cout << endl << "in Svc" << endl;
      cout << "a = " << a << endl;
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
      swapper();
      scheduler();
      return;
}

/*
called when the last running job requests service
sets block flag so that job may not run nor be terminated until all IO requests are met
*/
void block()
{
    cout << endl << "in block()" << endl;
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
     cout << endl << "in terminate" << endl;
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
    cout << endl << "in clear_from_readyq" << endl;
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
    cout << endl << "now adding job to jobtable" << endl;
    // Place the parameters in a temp job container
    Job newJob(jobNumber, jobSize, maxCpuTime, currTime, priority);

    // Add the temp vector to the JOBTABLE
    JOBTABLE.push_back(newJob);
}

void removeJobFromJobTable (long jobNumber)
{
    cout << endl << "now removing job from table" << endl;
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
    cout << endl << "finding job table entry for job num" << endl;
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
    cout << endl << "in clearSpace" << endl;
    FREESPACETABLE[index] = 0;

}

void clearSpace (int startIndex, int endIndex)
{
    cout << endl << "in clearSpace" << endl;
    for (int i = startIndex; i < endIndex; i++)
    {
        clearSpace(i);
    }
}

// Handles requests for swapping, starts a drum swap (using SOS function Siodrum), handles drum
// interrupts (Drmint), and selects which job currently on the drum should be swapped into memory.
void swapper()
{
    cout << endl << "now in swapper" << endl;
    //count_swaps++;
    //if(count_swaps > 1){
        for(int i = 0; i < JOBTABLE.size(); i++){
            //find first job on the drum and see if can find space
            if(JOBTABLE[i].getDirection() == 0){
                bool foundSpace = (findFreeSpace(JOBTABLE[i].getJobSize()) != -1 ? true : false);
                if (foundSpace){
                     cout << "found space in memory" << endl;
                     addJobToFST(JOBTABLE[i].getJobNumber());
                     //curr_job_num = jobNumber;
                     //readyq.push (JOBTABLE[i].getJobNumber());
                     //JOBTABLE[i].setInMemory(true);
                     //printQueue(readyq, "READY QUEUE");
                     curr_job_moving = JOBTABLE[i].getJobNumber();
                     siodrum(JOBTABLE[i].getJobNumber(), JOBTABLE[i].getJobSize(),
                     JOBTABLE[i].getAddress(), 0);
                }
                return;
           } else { //else is in core
                  if(JOBTABLE[i].getIORequest() > 0){
                       return;
                  }
                  /*
                  keep in core to finish running if there is no other job that wants to run
                  */
                  if(readyq.size() > 1){ // then move from core
                        //clear_from_readyq(JOBTABLE[i].getJobNumber());
                        curr_job_moving = JOBTABLE[i].getJobNumber();
                        siodrum(JOBTABLE[i].getJobNumber(), JOBTABLE[i].getJobSize(),
                                JOBTABLE[i].getAddress(), 1);
                        //JOBTABLE[i].setInMemory(false);

                  }

           }
    }
}

/*
 int jobLocation = findJob(jobNumber);
    int direction = JOBTABLE[jobLocation].getDirection();


    // direction = 0, swap from drum to memory
    if (direction == 0)
    {
        if (foundSpace)
        {
            cout << "found space in memory" << endl;
            addJobToFST(jobNumber);
            //curr_job_num = jobNumber;
            readyq.push (JOBTABLE[jobLocation].getJobNumber());
            JOBTABLE[jobLocation].setInMemory(true);
            //printQueue(readyq, "READY QUEUE");
            curr_job_moving = jobNumber;
            siodrum(JOBTABLE[jobLocation].getJobNumber(), JOBTABLE[jobLocation].getJobSize(),
                         JOBTABLE[jobLocation].getAddress(), 0);
        }
        // a job can't be swapped to memory because there is no space in FST
        //job remains on the drum
        return;
    }

    // direction = 1, swap from memory to drum
    else if (direction == 1 && readyq.size() > 1) //ADDED CONDITION AS TEST
    {
        if (JOBTABLE[jobLocation].getIsDoingIO() != true || JOBTABLE[jobLocation].isLatched() != true)
        {
            if (!readyq.empty())
            {
                if (readyq.front() == jobNumber)
                    readyq.pop();
            }

        }
    }
}

*/


/*
scheduler uses round robin implementation:
picks the next job to run from the ready queue
and sets time quantum
*/
void scheduler()
{

    cout << endl << "now in scheduler" << endl;
    cout << "a = " << a << endl;


    //if there is no job to run
    if(readyq.empty()){
         cout << "EMPTY READYQ: there is no job to run"  << endl;
         return;
    }

    int timequantum = 5;
    long curr_q_entry = readyq.front();
    std::stack<long> tmp;
    /*
    look for first job that is not blocked
    store the elements you go through to put back after search
    */
    int job_idx = findJob(curr_q_entry);
    while((!readyq.empty()) && (JOBTABLE[job_idx].isInMemory() == false) && (JOBTABLE[job_idx].isBlocked() || JOBTABLE[job_idx].getIsDoingIO() ) ) {
         tmp.push(curr_q_entry);
         readyq.pop();
         curr_q_entry = readyq.front();
         job_idx++;
    }
    cout << "scheduler, job to run= " << JOBTABLE[job_idx] << endl;
    //return after all jobs are put into original place given all jobs are blocked
    if(readyq.empty()){
         while(!tmp.empty()){
            readyq.push(tmp.top());
            tmp.pop();
         }
         return;
    }
    //only gets to this point if there is a job to run
    //if(a = 1){  //if the CPU is idle, run job
        a = 2; //run a job
        cout << "a now equal to " << a << endl;
        p[2] = JOBTABLE[job_idx].getAddress();
        cout << "job addr = " << p[2] << endl;
        p[3] = JOBTABLE[job_idx].getJobSize();
        cout << "job size = " << p[3] << endl;
        p[4] = timequantum;
        cout << "time quantum = " << p[4] << endl;
   // }
    dispatcher(a, p);
    return;
}

/*
dispatcher receives time quantum
and the index used to access the next running job's info in the JOBTABLE
*/
void dispatcher(long &a, long p[])
{
    cout << "in dispatcher\n" << "a = " << a << endl;
    a =2;

    // a and p needs to be passed in

    return;
}

void requestIO(){
    cout << endl << "now in requestIO()" << endl;
    JOBTABLE[curr_job_num].setIORequest(JOBTABLE[curr_job_num].getIORequest() + 1);
    ioQueue.push(curr_job_num);
    return;
}
