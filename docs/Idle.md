## How the idle proc works:
 I was debugging along and released that on the first ever schedule it will
 set current_thread->execution_state = cpu_state; where it is assumed cpu_state
 is the current thread state (ie what we have just been scheduling) and
 thus saves it in that thread. However, as the first thread has not had a
 chance to be scheduled yet, the current state is not the expected first
 thread's state and instead is the cpu state of the kernel.
 Now I could either fix that or leave it in as a cool way of never fully
 leaving kernelMain and  also having a idle_proc