/**
 * @file selector.h
 * @brief Implements classes for selecting drivers in the Driver Manager
 *
 * @date 24th November 2025
 * @author Max Tyson
 */

#include <core/selector.h>

using namespace DriverManager;
using namespace DriverManager::core;
using namespace LibDriver;

SelectorEventHandler::SelectorEventHandler() = default;

SelectorEventHandler::~SelectorEventHandler() = default;

/**
 * @brief This function is called when a driver is selected
 *
 * @param driver The driver that was selected
 */
void SelectorEventHandler::on_driver_selected(Driver* driver) {
}

Selector::Selector() = default;

Selector::~Selector() = default;

/**
 * @brief Select the drivers available on the system
 *
 * @param handler The event handler to notify when a driver is selected
 */
void Selector::select_drivers(SelectorEventHandler* handler) {
}