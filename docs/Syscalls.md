# Syscalls
(To be moved into a better docs setup, ie search for all functions syscal_(name) and build the table from their?)

In the fututure there will be a syscall abstraction method allowing to switch out the syscall implementation for different platforms.
Ie a program could load linux vX if it supports it, or windows vX if it supports it. This will allow for versions such as MaxOSvX.
This will be done via a translation layer to convert it into the correct syscall for the platform.

This is helpful for updating syscall table (idk if that will even happen or it will just grow) becuase it can be dsigned in a way the previous version can be translated into the latest. This may be bad but idk yet. Ie Linux v2 could target MaxosV2 but then would have to be translated to MaxOsV3.


## Interrupt Arguments
| Syscall | Arg0 | Arg1 | Arg2 | Arg3 | Arg4 | Arg5 |
|---------|------|------|------|------|------|------|
| rax     | rdi  | rsi  | rdx  | r10  | r8   | r9   |

## Syscall Table (v1)
| Number | Name                  | Description                     | Arg0                                   | Arg1       | Arg2        | Arg3 | Arg4 | Arg5 | Return                                         |
|--------|-----------------------|---------------------------------|----------------------------------------|------------|-------------|------|------|------|------------------------------------------------|
| 0      | close_process         | Exit a process                  | uint64_t pid (= 0 for current process) | int status |             |      |      |      |                                                |
| 1      | klog                  | Log a message to the kernel log | char* msg                              |            |             |      |      |      |                                                |
| 2      | created_shared_memory | Create a shared memory region   | size_t size                            | char* name |             |      |      |      | void*  address (null if failed)                |
| 3      | open_shared_memory    | Open a shared memory region     | char* name                             |            |             |      |      |      | void*  address (null if failed)                |
| 4      | allocate_memory       | Allocate memory                 | size_t size                            |            |             |      |      |      | void*  address (null if failed)                |
| 5      | free_memory           | Free memory                     | void* address                          |            |             |      |      |      |                                                |
| 6      | create_ipc_endpoint   | Create an IPC endpoint          | char* name                             |            |             |      |      |      | void*  message buffer address (null if failed) |
| 7      | send_ipc_message      | Send an IPC message             | char* name                             | void* data | size_t size |      |      |      |                                                |
| 8      | remove_ipc_endpoint   | Remove an IPC endpoint          | char* name                             |            |             |      |      |      |                                                |
| 9      | thread_yield          | Yield the current thread        |                                        |            |             |      |      |      |                                                | 
| 10     | thread_sleep          | Put the current thread to sleep | uint64_t time (ms)                     |            |             |      |      |      |                                                |
| 11     | thread_exit           | Exit the current thread         |                                        |            |             |      |      |      |                                                |