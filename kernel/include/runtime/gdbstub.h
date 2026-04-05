/**
 * @file gdbstub.h
 * @brief GDB Stub definition for remote debugging userspace processes
 *
 * @see https://www.chciken.com/tlmboy/2022/04/03/gdb-z80.html - Adapted from here, very helpful
 *
 * @todo almost zero validation going on
 *
 * @date 3rd April 2026
 * @author Max Tyson
 */


#ifndef MAXOS_RUNTIME_GDBSTUB_H
#define MAXOS_RUNTIME_GDBSTUB_H

#include <console/serial.h>
#include <string.h>
#include <processes/process.h>
#include <hardwarecommunication/interrupts.h>

namespace MaxOS::runtime {

    class GDBServer;
    class TrapFlagExcpetion : public hardwarecommunication::ExceptionHandler {

        private:
            GDBServer* m_gdb_handler;

        public:
            TrapFlagExcpetion(GDBServer* server);
            ~TrapFlagExcpetion();

            system::cpu_status_t *handle_interrupt(system::cpu_status_t *status) override;

    };

    class BreakPointExcpetion : public hardwarecommunication::ExceptionHandler {

        private:
            GDBServer* m_gdb_handler;

        public:
            BreakPointExcpetion(GDBServer* server);
            ~BreakPointExcpetion();

            system::cpu_status_t* handle_interrupt(system::cpu_status_t *status) override;

    };

    class GDBServer : public hardwarecommunication::InterruptHandler{

        private:

            processes::Thread* m_attached_thread;

            console::SerialConsole* m_serial;
            void handle_interrupt() override;

            string current_command_buffer;
            void try_handle_command();
            void delegate_command(const string& command);

            string format_register(uint64_t value, int num_bytes);

            TrapFlagExcpetion   m_trap_flag_handler;
            BreakPointExcpetion m_break_point_handler;

            common::Map<void*, uint8_t> m_breakpoints;

        public:
            GDBServer(console::SerialConsole* serial);
            ~GDBServer();

            void attach(processes::Thread* thread);

            string get_checksum(const string& payload);
            bool check_checksum(const string& payload, const string& checksum);

            string build_message(const string& payload);
            void send_message(const string& payload);

            void send_empty();
            void send_supported(const string& peer_supported);
            void send_registers();
            void send_memory(const string& message);
            void send_memory(void* address, size_t length);
            void send_breakpoint_reached();

            void handle_step_instruction();
            void handle_set_breakpoint(const string& message);
            void handle_set_breakpoint(void* address);
            void handle_remove_breakpoint(const string& message);
            void handle_remove_breakpoint(void* address);
            void handle_continue();
            void handle_detach();

            processes::Thread* attached_thread();
            system::cpu_status_t thread_regs;

    };

}

#endif //MAXOS_RUNTIME_GDBSTUB_H
