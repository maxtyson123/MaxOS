//
// Created by 98max on 27/10/2022.
//

#include <system/syscalls.h>

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::hardwarecommunication;
using namespace maxOS::system;

///__Handler__///

SyscallHandler::SyscallHandler(InterruptManager* interruptManager, uint8_t InterruptNumber)
        :    InterruptHandler(InterruptNumber  + interruptManager->HardwareInterruptOffset(), interruptManager)
{
}

SyscallHandler::~SyscallHandler()
{
}

/**
 * @details Handles the interrupt for a system call
 *
 * @param esp The stack frame
 */
uint32_t SyscallHandler::HandleInterrupt(uint32_t esp)
{
    CPUState_Thread* cpu = (CPUState_Thread*)esp;


    switch(cpu->eax)
    {
        case 4:                                 //Write
            //printf((char*)cpu->ebx);
            break;

         default:
            break;
    }


    return (uint32_t)cpu;
}

///__Syscall__///

/**
 * @details terminate the calling process
 *
 * @param status The exit code
 */
void sys_exit(int status)
{
    asm("int $0x80" : : "a" (1), "b" (status));     //Call the interrupt, passing the syscall number and the argument
}

/**
 * @details creates a new process by duplicating the calling process.
       The new process is referred to as the child process.  The calling
       process is referred to as the parent process.

 * @param cpu  The CPU state of the calling process
 */
void sys_fork(CPUState_Thread* cpu)
{
    asm("int $0x80" : : "a" (2), "b" (cpu));
}

/**
 * @details attempts to read up to count bytes from file descriptor fd
       into the buffer starting at buf.

 * @param fd  The file descriptor
 * @param buf The buffer to read into
 * @param count The number of bytes to read
 * @return On success, the number of bytes read is returned (zero indicates
       end of file), and the file position is advanced by this number. On error, -1 is returned
 */
size_t sys_read(unsigned int fd, char *buf,	size_t count){

    uint32_t data = 3;
    asm volatile( "int $0x80" : "=a"(data) : "b"(fd), "c" (buf), "d" (count));             //Call the interrupt, passing the syscall number and the argument
    return data;

    //https://man7.org/linux/man-pages/man2/read.2.html

}

/**
 * @details attempts to write up to count bytes from the buffer starting
       at buf to the file referred to by the file descriptor fd.

 * @param fd  The file descriptor
 * @param buf The buffer to write from
 * @param count The number of bytes to write
 * @return On success, the number of bytes written is returned (zero indicates nothing was written). On error, -1 is returned
 */
size_t sys_write(int fd, const void *buf, size_t count){


    uint32_t data = 4;
    asm volatile( "int $0x80" : "=a"(data) :  "b"(fd), "c" (buf), "d"(count));             //Call the interrupt, passing the syscall number and the argument
    return data;

    //https://man7.org/linux/man-pages/man2/write.2.html

}

/**
 * @details opens the file specified by pathname.  If
       the specified file does not exist, it may optionally (if O_CREAT
       is specified in flags) be created by open().

 * @param filename  The file to open
 * @param flags The flags to open the file with
 * @param mode The mode to open the file with
 * @return On success, these system calls return a nonnegative integer that is a file descriptor for the
       newly opened file.  On error, -1 is returned
 */
int sys_open(const char *pathname, int flags, mode_t mode){

        asm("int $0x80" : : "a" (5), "b" (pathname), "c" (flags), "d" (mode));
        //TODO: Return the file descriptor
        //https://man7.org/linux/man-pages/man2/open.2.html

}

/**
 * @details closes a file descriptor, so that it no longer refers to any
       file and may be reused.

 * @param fd  The file descriptor to close
 * @return On success, zero is returned.  On error, -1 is returned
 */
int sys_close(int fd){

        uint32_t data = 6;
        asm volatile( "int $0x80" : "=a"(data) :  "b"(fd));             //Call the interrupt, passing the syscall number and the argument
        return data;
        //https://man7.org/linux/man-pages/man2/close.2.html
}

/**
 * @details waits for a child process to stop or terminate.
 *
 * @param pid  The process ID of the child process
 * @param status The exit code of the child process
 * @param options The options to wait with
 * @return On success, returns the process ID of the child whose state has changed; if WNOHANG was
       specified and one or more child(ren) specified by pid exist, but have not yet changed state,
       then 0 is returned.  On error, -1 is returned
 */
pid_t sys_waitpid(pid_t pid, int *status, int options) {

    uint32_t data = 7;
    asm volatile( "int $0x80" : "=a"(data) : "b" (pid), "c" (status), "d" (options));             //Call the interrupt, passing the syscall number and the argument
    return data;
    //https://man7.org/linux/man-pages/man2/waitpid.2.html
}

/**
 * @ details equivalent to calling open() with flags
       equal to O_CREAT|O_WRONLY|O_TRUNC.

 * @param pathname
 * @param mode
 * @return On success, these system calls return a nonnegative integer that is a file descriptor for the
       newly opened file.  On error, -1 is returned
 */
int sys_creat(const char *pathname, mode_t mode){

    uint32_t data = 8;
    asm volatile( "int $0x80" : "=a"(data) :"b" (pathname), "c" (mode));
    //TODO: Return the file descriptor
    //https://man7.org/linux/man-pages/man2/open.2.html#:~:text=O_TRUNC%20is%20unspecified.-,creat,-()%0A%20%20%20%20%20%20%20A%20call
}

/**
 * @details creates a new link (also known as a hard link) to an
       existing file.

 * @param oldpath The path to the existing file
 * @param newpath The path to the new link
 * @return On success, zero is returned.  On error, -1 is returned
 */
int sys_link(const char *oldpath, const char *newpath){

    uint32_t data = 9;
    asm volatile( "int $0x80" : "=a"(data) : "b" (oldpath), "c" (newpath));             //Call the interrupt, passing the syscall number and the argument
    return data;
    //https://man7.org/linux/man-pages/man2/link.2.html
}

/**
 * @details deletes a name from the file system.  If that name was the
       last link to a file and no processes have the file open, the file
       is deleted and the space it was using is made available for reuse.

 * @param pathname The path to the file to unlink
 * @return On success, zero is returned.  On error, -1 is returned
 */
int sys_unlink(const char *pathname){

    uint32_t data = 10;
    asm volatile( "int $0x80" : "=a"(data) : "b" (pathname));             //Call the interrupt, passing the syscall number and the argument
    return data;

    //https://man7.org/linux/man-pages/man2/unlink.2.html
}