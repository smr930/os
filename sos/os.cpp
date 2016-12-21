#include <iostream>
#include <list>
#include <vector>
#include <map>
#include <queue>
#include <stack>
#include "Job.h"

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
void dispatcher();
void terminateJob(void);

/*
holds the job currently running or job which was just running
used in terminate service call
*/
long curr_job_num;
long curr_job_doingIO;
/*
represents the job currently swapping or just swapped
*/
long curr_job_moving;
long* aRef;
long* pRef;
int count_swaps;
/*
is updated by every interrupt handler
*/
long curr_time;

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
    curr_job_doingIO = -1;
    curr_time = 0;
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

    // assign the global a, p in os.cpp with address of SOS's a, p
    aRef = &a;
    pRef = p;

    cout << "now in crint" << endl;
    cout << "a = " << a << endl;
    cout << "job num = " << p[1] << endl;
    cout << "job priority = " << p[2] << endl;
    cout << "job size = " << p[3] << endl;
    cout << "max cpu time = " << p[4] << endl;
    cout << "curr time = " << p[5] << endl;
    curr_time = pRef[5];
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
    cout << "job num " << ioQueue.front() << "just finished 1 IO request" << endl;
    cout << "curr_job_doingIO = " << curr_job_doingIO << endl;

    curr_time = pRef[5];
    cout << "the current time is " << curr_time << endl;
    int ioJobIndex = findJob(ioQueue.front());
    JOBTABLE[ioJobIndex].setIORequest(JOBTABLE[ioJobIndex].getIORequest() - 1);
    cout << JOBTABLE[ioJobIndex].getIORequest() << " IO requests left" << endl;

    if(JOBTABLE[ioJobIndex].getIORequest() <= 0 ){
         ioQueue.pop();
         if(JOBTABLE[ioJobIndex].isBlocked()){
            JOBTABLE[ioJobIndex].setBlocked(false);
         }
         if(JOBTABLE[ioJobIndex].is_SetToTerminated()){
            terminateJob();
         }
    }
    //JOBTABLE[ioJobIndex].setIsDoingIO(false);
    JOBTABLE[ioJobIndex].setIsDoingIO(false);
    if(!ioQueue.empty()){
         //send next job from io queue to disk
         ioJobIndex = findJob(ioQueue.front());
         JOBTABLE[ioJobIndex].setIsDoingIO(true);
         curr_job_doingIO = ioQueue.front();
         siodisk(ioQueue.front()); //call siodisk to swap job to disk: input is job num
    }

    swapper();
    scheduler();
    return;
}

void Drmint (long &a, long p[])
{
    // Drum interrupt.
    // At call: p [5] = current time
    cout << endl << "in drmint" << endl;
    cout << "bookmarking job num " << curr_job_moving << endl;

    curr_time = pRef[5];
    cout << "the current time is " << curr_time << endl;
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

    curr_time = pRef[5];
    cout << "the current time is " << curr_time << endl;

    /*
    bookkeep time spent in CPU
    */
    int index = findJob(curr_job_num);
    JOBTABLE[index].time_in_CPU = curr_time - JOBTABLE[index].time_sent_to_CPU;

    if(readyq.size() > 1 || JOBTABLE[index].time_in_CPU >= JOBTABLE[index].getmaxCPUtime()){
         if(JOBTABLE[index].time_in_CPU >= JOBTABLE[index].getmaxCPUtime() ){
            cout << "job num " << curr_job_num << " exceeded max cpu time so terminate" << endl;
         }
         cout << "readyq has others waiting so terminate job!" << endl;
         terminateJob();
    } else {
         cout << "no other jobs set to run so don't terminate!" << endl;
    }
    swapper();
    scheduler();
    return;

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

      curr_time = pRef[5];
      cout << "the current time is " << curr_time << endl;

      /*
        bookkeep time spent in CPU
      */
     int index = findJob(curr_job_num);
     JOBTABLE[index].time_in_CPU = curr_time - JOBTABLE[index].time_sent_to_CPU;

      switch (a) {
          case 5: cout << "in svc switch: job requested to be terminated" << endl;
                  terminateJob();
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

    cout << "job num " << curr_job_num << " has " << JOBTABLE[job_idx].getIORequest()
         << " IO requests and requested to be blocked" << endl;
    if(JOBTABLE[job_idx].getIORequest() > 0){
        cout << "now setting to blocked" << endl;
        JOBTABLE[job_idx].setBlocked(true);
    } else{
         cout << "not enough IO requests to block!" << endl;
    }
    return;
}

/*
a job requests to be terminated if it has exceeded its max CPU time or an error occurred
head of ready queue represents the job that asked to be terminated
*/
void terminateJob()
{
     cout << endl << "in terminateJob" << endl;
     cout << "job num " << curr_job_num << " requested to be terminated" << endl;

     cout << "size of readyq=" << readyq.size() << endl;
     int job_idx = findJob(curr_job_num);
     //set flag to terminate once unblocked
     if(JOBTABLE[job_idx].isBlocked()){
          JOBTABLE[job_idx].setset_to_terminate(true);
          return;
     }
     //if exceeded CPU time, TODO
     //if error occurred, TODO

     clearSpace(JOBTABLE[job_idx].getAddress());
     clear_from_readyq(curr_job_num);
     return;
}

void clear_from_readyq(long job_num)
{
    cout << endl << "in clear_from_readyq" << endl;
    if(readyq.empty()){
        cout << "ERROR, EMPTY Q: cannot clear entry from readyq" << endl;
        //exit(EXIT_FAILURE);
    }

    //search for element and save the elements you pass through
    std::stack<long> tmp;
    long curr = -1;
    cout << "curr before loop = " << curr << endl;
    while(!readyq.empty()){
        curr = readyq.front();
        cout << "curr is equal to readyq.front= " << curr << endl;
        readyq.pop();
        if(curr != job_num){
             cout << "curr job num " << curr << " ! = " << job_num << endl;
             tmp.push(curr);
        } else{
            cout << "curr job num " << curr << " == " << job_num << endl;
        }
    }

    //if got to this point either found element OR queue has been emptied
    /*
    if(readyq.empty()){
        cout << "ERROR, EMPTY Q: cannot clear entry from readyq" << endl;
    }
    */


    while(!tmp.empty()){
        curr = tmp.top();
        readyq.push(curr);
        tmp.pop();
    }
    printQueue(readyq, "readyq after removing an entry");
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
    cout << endl << "finding job table entry for job num=" << jobNum << endl;
    for(int i = 0; i < JOBTABLE.size(); i++)
    {
        if(JOBTABLE[i].getJobNumber() == jobNum)
            return i;
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
                     curr_job_num = JOBTABLE[i].getJobNumber();
                     //readyq.push (JOBTABLE[i].getJobNumber());
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
scheduler uses round robin implementation:
picks the next job to run from the ready queue
and sets time quantum
*/
void scheduler()
{

    cout << endl << "now in scheduler" << endl;
    cout << "a = " << *aRef << endl;


    //if there is no job to run
    if(readyq.empty()){
         cout << "EMPTY READYQ: there is no job to run"  << endl;
         *aRef = 1; //set CPU idle
         return;
    }

    int timequantum = 5;
    //ensure no side effects by making copy of q
    std::queue<long> copy = readyq;
    cout << "size of readyq=" << readyq.size() << " size of copy=" << copy.size() << endl;
    long curr_q_entry = copy.front();
    cout << "front of q and curr_q_entry= " << curr_q_entry << endl;
    int readyq_size = copy.size();
    /*
    look for first job that is not blocked
    store the elements you go through to put back after search
    */
    int job_idx = findJob(curr_q_entry);
    while(copy.empty() == false && JOBTABLE[job_idx].isBlocked() == true){
         curr_q_entry = copy.front();
         job_idx = findJob(curr_q_entry);
         cout << "now seeing if can run job num " << curr_q_entry << " at index " << job_idx << endl;
         if(JOBTABLE[job_idx].isBlocked() == false && JOBTABLE[job_idx].isInMemory() && JOBTABLE[job_idx].time_in_CPU + timequantum <= JOBTABLE[job_idx].getmaxCPUtime())
              break;
         else{
              cout << "cannot run job num " << curr_q_entry << endl;
              cout << "job is blocked is " << JOBTABLE[job_idx].isBlocked() << endl;
              copy.pop();
         }
    }

    if(copy.empty()){
         cout << "\ndid not find any job to run" << endl;
         *aRef = 1; //set CPU to idle
         return;
    }
    //cout << "scheduler, job to run= " << JOBTABLE[job_idx].getJobNumber() << endl;

    /*
    only gets to this point if there is a job to run
    job_idx represents index for job to be run
    /if(a = 1){  //if the CPU is idle, run job
    */
    cout << "found job num " << JOBTABLE[job_idx].getJobNumber() << " to run" << endl;

    cout << "timequantum = " << timequantum << " maxCPU time for job = " << JOBTABLE[job_idx].getmaxCPUtime() << endl;
    if(JOBTABLE[job_idx].time_in_CPU + timequantum <= JOBTABLE[job_idx].getmaxCPUtime()){
        cout << "time in CPU + timequantum = " << timequantum << " is less than or equal to maxCPU time = " << JOBTABLE[job_idx].getmaxCPUtime() << endl;
        /*
        timequantum = JOBTABLE[job_idx].getmaxCPUtime() - JOBTABLE[job_idx].time_in_CPU;
        cout << "time quantum now = " << timequantum << endl;
        if(timequantum <= 0){
            cout << "time ran out on this job" << endl;
            return;
        }
        */
    }

    ///set time you WOULD sent job to CPU
    long old_time = JOBTABLE[job_idx].time_sent_to_CPU;
    JOBTABLE[job_idx].time_sent_to_CPU = curr_time;

    *aRef = 2; //run a job
    cout << "a now equal to " << *aRef << endl;
    pRef[2] = JOBTABLE[job_idx].getAddress();
    cout << "job addr = " << pRef[2] << endl;
    pRef[3] = JOBTABLE[job_idx].getJobSize();
    cout << "job size = " << pRef[3] << endl;
    pRef[4] = timequantum;
    cout << "time quantum = " << pRef[4] << endl;

    dispatcher();
    return;
}

/*
dispatcher receives time quantum
and the index used to access the next running job's info in the JOBTABLE
*/
void dispatcher()
{
    cout << "in dispatcher\n" << "a = " << *aRef << endl;
    *aRef = 2;

    // a and p needs to be passed in

    return;
}

void requestIO(){
    cout << endl << "now in requestIO()" << endl;

    int index = findJob(curr_job_num);
    cout << "job num " << curr_job_num << "requested IO and is at index " << index << endl;
    JOBTABLE[index].setIORequest(JOBTABLE[index].getIORequest() + 1);

    if(!ioQueue.empty()){
        cout << "IO queue busy: add to queue" << endl;
        ioQueue.push(curr_job_num);
        return;
    }
    //do not leave drum idle!
    cout << "sending job to disk!" << endl;
    JOBTABLE[index].setIsDoingIO(true);
    curr_job_doingIO = curr_job_num;
    ioQueue.push(curr_job_num); ///is checked in dskint
    siodisk(curr_job_num);

    return;
}

// OLD SWAPPER CODE
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
