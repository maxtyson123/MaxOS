/**
 * @file test.cpp
 * @brief Implements a basic test class for MaxOS
 *
 * @date 23rd November 2025
 * @author Max Tyson
 */

#include <tests/test.h>
#include <tests/common.h>

using namespace MaxOS;
using namespace MaxOS::tests;

Test::Test(string const& name, TestType type)
: m_name(name),
  m_type(type)
{
	TestRunner::add_test(this);
}

/**
 * @brief Runs the test and returns the result
 *
 * @return TestStatus The status of the test after running
 */
TestStatus Test::run() {

	// Run the test
	auto result = execute();
	StringBuilder msg;

	// Build the message
	msg << "Test ";
	switch(result) {
		case TestStatus::PASS:
			msg << "PASSED";
			break;
		case TestStatus::SKIP:
			msg << "SKIPPED";
			break;
		case TestStatus::FAIL:
			msg << "FAILED";
			break;
	}
	msg << " - " << m_name << "\n";

	// Print the outcome
	if (result == TestStatus::FAIL)
		Logger::ERROR() << msg.out;
	else
		Logger::TEST() << msg.out;

	return result;
}

/**
 * @brief Gets the name of the test
 *
 * @return The name of the test
 */
const string& Test::name() const {
	return m_name;
}

/**
 * @brief Gets the type of the test
 *
 * @return The type of the test
 */
TestType Test::type() const {
	return m_type;
}

Test::~Test() = default;


ConditionalTest::ConditionalTest(string const& name, TestType type, bool (* condition)())
: Test(name, type),
  m_condition(condition)
{
}

/**
 * @brief Executes the conditional test based on the provided condition function
 *
 * @return TestStatus The status of the test after execution
 */
TestStatus ConditionalTest::execute() {

	if (m_condition())
		return TestStatus::PASS;
	else
		return TestStatus::FAIL;

}


ConditionalTest::~ConditionalTest() = default;

/**
 * @brief Runs all registered tests
 */
void TestRunner::run_all_tests() {
	Logger::TEST() << "Running all tests... (" << (int )s_tests.size() << " tests found)\n";
	for (auto& test : s_tests)
		test->run();
}

/**
 * @brief Runs all tests of a specific type
 *
 * @param type The type of tests to run
 */
void TestRunner::run_all_types(TestType type) {
	for (auto& test : s_tests)
		if (test->type() == type)
			test->run();
}

/**
 * @brief Adds a test to the test runner
 *
 * @param test The test to add
 */
void TestRunner::add_test(Test* test) {
	s_tests.push_back(test);
}

/**
 * @brief Adds all predefined tests to the test runner
 */
void TestRunner::add_all_tests() {
	register_tests_common();
}

/**
 * @brief Setups the TestRunner and runs all kernel tests
 */
void TestRunner::test_kernel() {

	// Add all tests
	s_tests = common::Vector<Test*>();
	add_all_tests();

	// Run all system tests
	run_all_tests();

}
