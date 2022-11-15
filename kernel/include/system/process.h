//
// Created by 98max on 15/11/2022.
//

#ifndef MAXOS_PROCESS_H
#define MAXOS_PROCESS_H

#include <system/multithreading.h>
#include <common/types.h>

namespace maxOS{

    namespace system{

            class Process;



            class Process{
                friend class ProcessManager;
                private:
                    ThreadManager* threadManager;
                    int mainThreadID;
                    int childThreads[6];
                    int numChildThreads = 0;

                public:
                    void CreateChildThread(void entrypoint());
                    void KillChildThread(int threadID);
                    void KillAllChildThreads();
                    void RefreshProcess();
                    void Kill();

                    void threadMain(void entrypoint(), Process* process);



                    Process(void entrypoint(), ThreadManager* threadManager);
                    ~Process();
            };

    }


}

#endif //MAXOS_PROCESS_H
