/**
 * @file driver.cpp
 * @brief Implementation of a generic Driver class
 *
 * @date 24th November 2025
 * @author Max Tyson
*/

#include <driver.h>

using namespace LibDriver;
using namespace MaxOS;
using namespace MaxOS::KPI::ipc;

Driver::Driver() = default;

Driver::~Driver() = default;

/**
 * @brief Activate the driver
 */
void Driver::activate() {

}

/**
 * @brief Deactivate the driver
 */
void Driver::deactivate() {

}

/**
 * @brief Initialise the driver
 */
void Driver::initialise() {

}

/**
 * @brief Reset the driver
 *
 * @return How long in milliseconds it took to reset the driver
 */
uint32_t Driver::reset() {
	return 0;
}

/**
 * @brief Get who created the device
 *
 * @return The vendor name of the driver
 */
string Driver::vendor_name() {
	return "Generic";
}

/**
 * @brief Get the device name of the driver
 *
 * @return The device name of the driver
 */
string Driver::device_name() {
	return "Unknown Driver";
}


/**
 * @brief Construct a new Driver Server object and register the driver RPC calls
 *
 * @param driver The instance of the driver to register
 */
DriverServer::DriverServer(Driver* driver) {

	// Setup references
	s_instance = this;
	m_driver = driver;

	// Register RPC calls
	rpc_register_function("activate", activate_wrapper);
	rpc_register_function("deactivate", deactivate_wrapper);
	....
}

/**
 * @brief Get the server instance
 *
 * @return  The driver server instance or nullptr if not created yet
 */
DriverServer* DriverServer::server() {
	return s_instance;
}

/**
 * @brief Get the driver instance
 *
 * @return The driver instance or nullptr if not set
 */
Driver* DriverServer::driver() {

	if(s_instance == nullptr)
		return nullptr;

	return s_instance->m_driver;
}

/**
 * @brief Start the driver server and listen for requests
 */
void DriverServer::start(string id) {

	// Create the name of the server
	string server_name = (string)"driver_" + id;
	rpc_server_loop(server_name.c_str());


}

DriverServer::~DriverServer() = default;

DriverClient::DriverClient(MaxOS::string id) {

	m_id = "driver_" + id;

}

void DriverClient::activate() {
	ArgList _args;
	ArgList _returns;
	rpc_call(m_id.c_str(), "activate",  &_args, &_returns);
}


void activate_wrapper(ArgList* args, ArgList* returns) {
	Driver* driver = DriverServer::driver();
	if(driver != nullptr)
		driver->activate();
	// add returns and stuff if not a void
}