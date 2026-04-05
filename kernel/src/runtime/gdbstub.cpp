/**
 * @file gdbstub.cpp
 * @brief GDB Stub implementation for remote debugging userspace processes
 *
 * @date 3rd April 2026
 * @author Max Tyson
 */

#include <runtime/gdbstub.h>

#include "processes/scheduler.h"

using namespace MaxOS;
using namespace MaxOS::runtime;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::common;
using namespace MaxOS::console;
using namespace MaxOS::processes;
using namespace MaxOS::system;
using namespace MaxOS::memory;


GDBServer::GDBServer(SerialConsole *serial)
: InterruptHandler(0x24, 4, 0x18),
  m_serial(serial),
  m_trap_flag_handler(this),
  m_break_point_handler(this)
{
}

GDBServer::~GDBServer() = default;

void GDBServer::attach(Thread* thread) {

    // Pause the thread
    thread->thread_state = ThreadState::WAITING;

    // Read the thread info
    m_attached_thread = thread;
    thread_regs = (const cpu_status_t&)m_attached_thread->execution_state;

}

void GDBServer::handle_interrupt() {

    // Read the packet
    while (m_serial->can_read())
       current_command_buffer += m_serial->read_char();

    try_handle_command();
}


void GDBServer::try_handle_command() {

    // Find the checksum
    int checksum_start = -1;
    for (int i = 0; i < current_command_buffer.length(); ++i)
        if (current_command_buffer[i] == '#') {
            checksum_start = i;
            break;
        }

    // Haven't received the full command yet
    if (checksum_start == -1 || current_command_buffer.length() < checksum_start + 3)
        return;

    // Find the command
    int command_start = -1;
    for (int i = 0; i < current_command_buffer.length(); ++i)
        if (current_command_buffer[i] == '$') {
            command_start = i;
            break;
        }

    // Parse the command
    string command          = current_command_buffer.substring(command_start + 1, checksum_start - command_start - 1);
    string checksum         = current_command_buffer.substring(checksum_start + 1, 2);
    current_command_buffer  = current_command_buffer.substring(checksum_start + 3, current_command_buffer.length() - checksum_start - 2);

    // Discard any garbage messages
    if (!check_checksum(command, checksum)) {
        Logger::DEBUG() << "Dropping: " << command << " (expected: " << checksum << " calced " << get_checksum(command) << ")\n";
        return;
    }

    // Handle the command
    delegate_command(command);

    // More commands to process
    if (current_command_buffer.length() > 0)
        try_handle_command();
}

void GDBServer::delegate_command(const string &command) {

    // GDB querying why halted?
    if (command == "?") {
        send_message("S05");
        return;
    }

    // GDB step instruction
    if (command == "s") {
        handle_step_instruction();
        return;
    }

    // GDB requesting register dump
    if (command == "g") {
        send_registers();
        return;
    }

    // GDB continue execution
    if (command == "c") {
        handle_continue();
        return;
    }

    // GDB Detaching
    if (command == "D"){
        handle_detach();
        return;
    }

    // GDB requesting mem dump
    if (command.starts_with("m")) {
        send_memory(command);
        return;
    }

    // GDB requesting insert breakpoint (software bp)
    if (command.starts_with("Z0")) {
        handle_set_breakpoint(command);
        return;
    }

    // GDB requesting remove breakpoint (software bp)
    if (command.starts_with("z0")) {
        handle_remove_breakpoint(command);
        return;
    }

    // GDB requesting what features this server supports
    if (command.starts_with("qSupported")){
        send_supported(command);
        return;
    }

    // GDB asking if already attached
    if (command.starts_with("qAttached")){
        send_message("1");
        return;
    }

    // GDB asking for arch
    if (command.starts_with("qXfer")) {
        send_message("l<target version=\"1.0\"><architecture>i386:x86-64</architecture></target>");
        return;
    }

    // Unknown command
    send_empty();

}

constexpr char* HEX_CHARS = "0123456789abcdef";
string GDBServer::format_register(uint64_t value, int num_bytes) {

    string out = "";

    for (int i = 0; i < num_bytes; ++i) {

        // Get as little endian
        uint8_t byte = (value >> (i * 8)) & 0xFF;

        // Reverse
        out += String(HEX_CHARS[(byte >> 4) & 0x0F]);
        out += String(HEX_CHARS[byte & 0x0F]);
    }

    return out;
}

string GDBServer::get_checksum(const string &payload) {

    // Sum each character
    uint8_t checksum = 0;
    for (int i = 0; i < payload.length(); ++i)
        checksum += (uint8_t)payload[i];

    // Take first 8 bits
    checksum &= 0xFF;

    // Convert to hex
    string hex = string((uint64_t)checksum);
    if (hex.length() == 1)
        hex = string("0") + hex;

    return hex;

}

bool GDBServer::check_checksum(const string &payload, const string &checksum) {
    return get_checksum(payload).to_lower() == checksum;
}

string GDBServer::build_message(const string &payload) {
    return string("+$") + payload + "#" + get_checksum(payload);
}

void GDBServer::send_message(const string &payload) {
    string message = build_message(payload.c_str());
    m_serial->write(message);
}

void GDBServer::send_empty() {
    send_message("");
}

void GDBServer::send_supported(const string& peer_supported) {

    string out = "PacketSize=4000;qXfer:features:read+;";

    // Must support hardware breakpoints
    Vector<string> features = peer_supported.split("+;");
    for (const auto& feature : features)
        if (feature == "hwbreak")
            out += "hwbreak+;";

    send_message(out);
}

void GDBServer::send_registers() {

    string payload = "";

    // General Purpose Registers
    payload += format_register(thread_regs.rax, 8);
    payload += format_register(thread_regs.rbx, 8);
    payload += format_register(thread_regs.rcx, 8);
    payload += format_register(thread_regs.rdx, 8);
    payload += format_register(thread_regs.rsi, 8);
    payload += format_register(thread_regs.rdi, 8);
    payload += format_register(thread_regs.rbp, 8);
    payload += format_register(thread_regs.rsp, 8);

    // Secondary General Purpose Registers
    payload += format_register(thread_regs.r8, 8);
    payload += format_register(thread_regs.r9, 8);
    payload += format_register(thread_regs.r10, 8);
    payload += format_register(thread_regs.r11, 8);
    payload += format_register(thread_regs.r12, 8);
    payload += format_register(thread_regs.r13, 8);
    payload += format_register(thread_regs.r14, 8);
    payload += format_register(thread_regs.r15, 8);

    // Instruction Pointer
    payload += format_register(thread_regs.rip, 8);

    // Flags / Segments
    payload += format_register(thread_regs.rflags, 4);
    payload += format_register(thread_regs.cs, 4);
    payload += format_register(thread_regs.ss, 4);

    // Remaining
    payload += (string)"x" * (8 * 4);

    send_message(payload);
}

void GDBServer::send_memory(const string &message) {

    // Parse
    Vector<String> parts = message.substring(1,message.length( ) - 1).split(",");
    string address_str = parts[0].to_upper();
    string length_str = parts[1].to_upper();

    send_memory((void*)address_str.hex_to_uint64(), length_str.hex_to_uint64());
}

void GDBServer::send_memory(void *address, size_t length) {

    // Cant read
    if (address < (void*)0x1000) {
        send_message("E01");
        return;
    }

    // Switch into the processes' address space
    MemoryManager* old = GlobalScheduler::current_process()->memory_manager;
    MemoryManager::switch_active_memory_manager(GlobalScheduler::get_process(m_attached_thread->parent_pid)->memory_manager);

    // Send data
    String payload(address, length);
    send_message(payload.to_lower());

    // Restore executing process's address space
    MemoryManager::switch_active_memory_manager(old);

}

void GDBServer::handle_step_instruction() {

    // Acknowledge
    m_serial->write_char('+');

    // Enable trap flag for the thread and allow it to run
    thread_regs.rflags |= (1 << 8);
    m_attached_thread->thread_state = ThreadState::READY;

    // Make sure change is mirrored if not in kernel
    if (thread_regs.rip == m_attached_thread->execution_state.rip)
        m_attached_thread->execution_state.rflags |= (1 << 8);

    // wait for the step to occur and be caught by the exception handler
    // ...
}

void GDBServer::handle_set_breakpoint(const string &message) {

    // Parse
    Vector<String> parts = message.substring(2,message.length( ) - 2).split(",");
    string address_str = parts[1].to_upper();

    handle_set_breakpoint((void*)address_str.hex_to_uint64());

}

void GDBServer::handle_set_breakpoint(void* address) {

    // Invaild address
    if (address < (void*)0x1000) {
        send_message("OK");
        return;
    }

    // Already set or invalid address
    if (m_breakpoints.find(address) != m_breakpoints.end()) {
        send_message("OK");
        return;
    }

    // Switch into the processes' address space
    MemoryManager* old = GlobalScheduler::current_process()->memory_manager;
    MemoryManager::switch_active_memory_manager(GlobalScheduler::get_process(m_attached_thread->parent_pid)->memory_manager);

    // Store old byte
    uint8_t old_byte = *(uint8_t*)address;
    m_breakpoints.insert(address, old_byte);

    // Replace with breakpoint (INT3)
    CPU::disable_write_protect();
    *(uint8_t*)address = 0xCC;
    CPU::enable_write_protect();

    // Restore executing process's address space
    MemoryManager::switch_active_memory_manager(old);
    send_message("OK");
}

void GDBServer::handle_remove_breakpoint(const string &message) {

    // Parse
    Vector<String> parts = message.substring(2,message.length( ) - 2).split(",");
    string address_str = parts[1].to_upper();

    handle_remove_breakpoint((void*)address_str.hex_to_uint64());

}

void GDBServer::handle_remove_breakpoint(void *address) {

    // Invaild address
    if (address < (void*)0x1000) {
        send_message("OK");
        return;
    }

    // Doesnt exist
    auto breakpoint = m_breakpoints.find(address);
    if (breakpoint == m_breakpoints.end()) {
        send_message("OK");
        return;
    }

    // Switch into the processes' address space
    MemoryManager* old = GlobalScheduler::current_process()->memory_manager;
    MemoryManager::switch_active_memory_manager(GlobalScheduler::get_process(m_attached_thread->parent_pid)->memory_manager);

    // Replace with old byte
    CPU::disable_write_protect();
    *(uint8_t*)address = breakpoint->second;
    CPU::enable_write_protect();

    // Remove reference
    m_breakpoints.erase(breakpoint);

    // Restore executing process's address space
    MemoryManager::switch_active_memory_manager(old);
    send_message("OK");

}

void GDBServer::handle_continue() {

    // Acknowledge
    m_serial->write_char('+');

    // Let the thread be scheduled
    m_attached_thread->thread_state = ThreadState::READY;
}

void GDBServer::handle_detach() {

    // Remove all breakpoints
    for (const auto& bp : m_breakpoints)
        handle_remove_breakpoint(bp.first);

    // Allow program to continue rest of execution
    m_attached_thread->thread_state = ThreadState::READY;

}

Thread* GDBServer::attached_thread() {

    return m_attached_thread;

}

TrapFlagExcpetion::TrapFlagExcpetion(GDBServer *server)
: ExceptionHandler(0x1),
  m_gdb_handler(server)
{

}

TrapFlagExcpetion::~TrapFlagExcpetion() = default;

cpu_status_t * TrapFlagExcpetion::handle_interrupt(cpu_status_t* status) {

    // Clear flag
    status->rflags &= ~(1 << 8);
    m_gdb_handler->thread_regs = *status;
    m_gdb_handler->attached_thread()->thread_state = ThreadState::WAITING;

    // Notify the server that it stopped
    m_gdb_handler->send_message("S05");

    // Dont execute further until instructed by GDB
    GlobalScheduler::current_thread()->yield();

    // Copy the registers back as they may have been modified elsewhere
    *status = m_gdb_handler->thread_regs;
    return status;
}

BreakPointExcpetion::BreakPointExcpetion(GDBServer *server)
: ExceptionHandler(0x3),
  m_gdb_handler(server)
{

}

BreakPointExcpetion::~BreakPointExcpetion() = default;

cpu_status_t* BreakPointExcpetion::handle_interrupt(cpu_status_t *status) {

    // Move back
    status->rip -= 1;
    m_gdb_handler->thread_regs = *status;
    m_gdb_handler->attached_thread()->thread_state = ThreadState::WAITING;

    // Notify the server that it stopped
    m_gdb_handler->send_message("S05");

    // Dont execute further until instructed by GDB
    GlobalScheduler::current_thread()->yield();

    // Copy the registers back as they may have been modified elsewhere
    *status = m_gdb_handler->thread_regs;
    return status;


}
