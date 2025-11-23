/**
 * @file common.cpp
 * @brief Implements the tests for all common components of MaxOS
 *
 * @date 23rd November 2025
 * @author Max Tyson
*/

#include <tests/common.h>

using namespace::MaxOS;
using namespace::MaxOS::tests;


void register_buffer_tests(){

}

void register_colour_tests(){

}

void register_graphicsContext_tests(){

}




/**
 * @brief Registers all common tests with the test runner
 */
void MaxOS::tests::register_tests_common() {

	// String Tests

	MAXOS_CONDITIONAL_TEST(StringConcatenation, TestType::COMMON)
	{
		string a = "Hello";
		a += " World";
		return a == "Hello World";
	});



}