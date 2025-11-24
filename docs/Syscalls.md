# Syscalls
(To be moved into a better docs setup, ie search for all functions syscall_(name) and build the table from their?)

In the future there will be a syscall abstraction method allowing to switch out the syscall implementation for different platforms.
Ie a program could load linux vX if it supports it, or windows vX if it supports it. This will allow for versions such as MaxOSvX.
This will be done via a translation layer to convert it into the correct syscall for the platform.

This is helpful for updating syscall table (I don't know if that will even happen, or it will just grow) because it can be designed in a way the previous version can be translated into the latest. This may be bad but dk yet. Ie Linux v2 could target MaxosV2 but then would have to be translated to MaxOsV3.


## Interrupt Arguments
| Syscall | Arg0 | Arg1 | Arg2 | Arg3 | Arg4 | Arg5 |
|---------|------|------|------|------|------|------|
| rax     | rdi  | rsi  | rdx  | r10  | r8   | r9   |

## Syscall Table (v1)
| Number | Name            | Description                     | Arg0                                   | Arg1         | Arg2         | Arg3         | Arg4 | Arg5 | Return                                |
|--------|-----------------|---------------------------------|----------------------------------------|--------------|--------------|--------------|------|------|---------------------------------------|
| 0      | close_process   | Exit a process                  | uint64_t pid (= 0 for current process) | int status   |              |              |      |      |                                       |
| 1      | klog            | Log a message to the kernel log | char* msg                              |              |              |              |      |      |                                       |
| 2      | allocate_memory | Allocate memory                 | size_t size                            |              |              |              |      |      | void*  address (null if failed)       |
| 3      | free_memory     | Free memory                     | void* address                          |              |              |              |      |      |                                       |
| 4      | resource_create | Create a resource               | ResourceType type                      | char* name   | size_t flags |              |      |      | int success (1 = success, 0 = failed) |
| 5      | resource_open   | Open a resource                 | ResourceType type                      | char* name   | size_t flags |              |      |      | uint64_t handle (0 if failed)         |
| 6      | resource_close  | Close a resource                | uint64_t handle                        | size_t flags |              |              |      |      |                                       |
| 7      | resource_write  | write to a resource             | uint64_t handle                        | void* buffer | size_t size  | size_t flags |      |      | size_t bytes written (0 if failed)    |
| 8      | resource_read   | read from a resource            | uint64_t handle                        | void* buffer | size_t size  | size_t flags |      |      | size_t bytes read (0 if failed)       |
| 9      | thread_yield    | Yield the current thread        |                                        |              |              |              |      |      |                                       | 
| 10     | thread_sleep    | Put the current thread to sleep | uint64_t time (ms)                     |              |              |              |      |      |                                       |
| 11     | thread_exit     | Exit the current thread         |                                        |              |              |              |      |      |                                       |