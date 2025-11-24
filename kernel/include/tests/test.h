/**
 * @file test.h
 * @brief Defines a basic test class for MaxOS
 *
 * @date 23rd November 2025
 * @author Max Tyson
 */

#ifndef MAXOS_TESTS_TEST_H
#define MAXOS_TESTS_TEST_H

#include <common/string.h>
#include <common/logger.h>
#include <common/macros.h>

/// Simplified macro to define a conditional test
#define MAXOS_CONDITIONAL_TEST(name, type) \
    auto CONCATENATE(test_, __COUNTER__) = new ConditionalTest(#name, type, []()

namespace MaxOS::tests {

	/**
	 * @enum TestStatus
	 * @brief The possible outcomes of a test
	 */
	enum class TestStatus {
		PASS,
		FAIL,
		SKIP
	};

	/**
	 * @enum TestType
	 * @brief The different types of tests
	 */
	enum class TestType {
		COMMON,
		DRIVER,
		FILESYSTEM,
		GUI,
		HARDWARE_COMMUNICATION,
		MEMORY,
		NETWORK,
		PROCESSES,
		RUNTIME,
		SYSTEM,
		MAX_TEST_TYPES
	};

	/// String representations of the TestType enum
	constexpr const char* TEST_TYPE_STRINGS[] = {
		"COMMON",
		"DRIVER",
		"FILESYSTEM",
		"GUI",
		"HARDWARE_COMMUNICATION",
		"MEMORY",
		"NETWORK",
		"PROCESSES",
		"RUNTIME",
		"SYSTEM",
		"UNKNOWN"
	};

	/**
	 * @class Test
	 * @brief A basic test class
	 */
	class Test {

		private:
			string m_name;
			TestType m_type;

		public:
			Test(const string& name, TestType type);
			virtual ~Test();

			TestStatus run();

			/**
			 * @brief Executes the test and returns the status of the test
			 *
			 * @return The result of the test execution
			 */
			virtual TestStatus execute() = 0;

			const string& name() const;
			TestType type() const;
	};

	/**
	 * @brief Compares two values and logs a warning if they are not equal
	 *
	 * @tparam T1 The type of the actual value
	 * @tparam T2 The type of the expected value
	 * @param actual The actual value
	 * @param expected The expected value
	 * @return True if the values are equal, false otherwise
	 */
	template<typename T1, typename T2> bool compare(T1 const& actual, T2 const& expected) {

		// Direct comparison
		if (actual == expected)
			return true;

		// Report what failed
		Logger::WARNING() << "Test failed comparison: expected [" << expected << "], got [" << actual << "]\n";
		return false;
	}

	/**
	 * @class ConditionalTest
	 * @brief A test that only checks a condition to determine pass/fail
	 */
	class ConditionalTest : public Test {
		private:
			bool (*m_condition)();

		public:
			ConditionalTest(const string& name, TestType type, bool (*condition)());
			~ConditionalTest() override;

			TestStatus execute() override;
	};

	/**
	 * @class TestRunner
	 * @brief A class to run all tests
	 *
	 * @todo Revisit when cbf
	 */
	class TestRunner {

		private:
			inline static common::Vector<Test*> s_tests = common::Vector<Test*>();

		public:

			static void test_kernel();

			static void run_all_tests();
			static void run_all_types(TestType type);

			static void add_all_tests();
			static void add_test(Test* test);

	};
}

#endif //MAXOS_TESTS_TEST_H
