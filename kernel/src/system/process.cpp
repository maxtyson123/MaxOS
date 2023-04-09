//
// Created by 98max on 15/11/2022.
//

#include <system/process.h>

using namespace maxOS;
using namespace maxOS::system;


Process::Process(void entrypoint(), ThreadManager* threadManager) {


    //Create main thread
    this -> threadManager = threadManager;




    mainThreadID = threadManager -> CreateThread(entrypoint);

    //Clear child threads
    for (int i = 0; i < 6; ++i) {
        childThreads[i] = -1;
    }

}

Process::~Process() {

    Kill();


}

void Process::threadMain(void entrypoint(), Process* process){
    entrypoint();                                           //Run the task
    process -> Kill();                                      //Kill the process

}

/**
 * @details Create a new thread, as a child of this process
 *
 * @param entrypoint Entry point of the thread
 */
void Process::CreateChildThread(void (*entrypoint)()) {

    RefreshProcess();
    if(numChildThreads < 6){                                                                            //If there is space for a new thread
        for (int i = 0; i < 6; ++i) {                                                                   //Loop through child threads
            if(childThreads[numChildThreads] == -1){                                                    //If the thread is empty
                childThreads[numChildThreads] = threadManager -> CreateThread(entrypoint);                 //Create a new thread
                numChildThreads++;                                                                      //Increase the number of child threads
                break;
            }
        }
    }

}

/**
 * @details Kill the process's child thread
 *
 * @param threadID ID of the thread to kill
 */
void Process::KillChildThread(int threadID) {

    for(int i = 0; i < numChildThreads; i++){                                     //Loop through child threads
        if(childThreads[i] == -1) continue;         //If the thread is empty, skip it

        if(childThreads[i] == threadID){                                          //If the thread is the one to kill
            threadManager -> TerminateThread(threadID);                       //Kill the thread
            childThreads[i] = -1;                                                 //Set the thread to empty
            numChildThreads--;                                                    //Decrease the number of child threads
        }
    }

}

/**
 * @details kill all the child threads
 */
void Process::KillAllChildThreads() {



    //Loop through child threads and kill them
    for(int i = 0; i < numChildThreads; i++){

        if(childThreads[i] == -1) continue;         //If the thread is empty, skip it

        threadManager -> TerminateThread(childThreads[i]);
        childThreads[i] = -1;
    }
    numChildThreads = 0;


}

/**
 * @details refresh the process, removing any empty child threads
 */
void Process::RefreshProcess() {

    //Loop through child threads and refresh them
    for(int i = 0; i < numChildThreads; i++){
        if(childThreads[i] == -1) continue;         //If the thread is empty, skip it

        if(!threadManager->CheckThreads(childThreads[i])){              //Check If thread still exists
            childThreads[i] = -1;
            numChildThreads--;
        }
    }


}

/**
 * @details Kill the process
 */
void Process::Kill() {
    //Kill all child threads
    KillAllChildThreads();

    //Kill main thread
    threadManager -> TerminateThread(mainThreadID);
}

