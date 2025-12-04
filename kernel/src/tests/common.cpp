/**
 * @file common.cpp
 * @brief Implements the tests for all common components of MaxOS
 *
 * @date 23rd November 2025
 * @author Max Tyson
*/

#include <tests/common.h>
#include <buffer.h>
#include <colour.h>
#include <common/graphicsContext.h>
#include <inputStream.h>
#include <common/logger.h>
#include <map.h>
#include <outputStream.h>
#include <rectangle.h>
#include <string.h>
#include <time.h>
#include <vector.h>

using namespace ::MaxOS;
using namespace ::MaxOS::tests;
using namespace ::MaxOS::common;

/**
 * @brief Registers all buffer tests
 */
void register_buffer_tests() {

	MAXOS_CONDITIONAL_TEST(Buffer_Construct_Size, TestType::COMMON)
	{
		Buffer buf(10);
		return compare(buf.capacity(), 10);
	});

	MAXOS_CONDITIONAL_TEST(Buffer_Construct_FromSource, TestType::COMMON)
	{
		// Create from existing data
		uint8_t data[5] = { 1, 2, 3, 4, 5 };
		Buffer buf(data, 5);

		// Make sure data matches
		for(size_t i = 0; i < 5; i++)
			if(!compare(buf.read(0), data[i]))
				return false;

		return true;
	});

	MAXOS_CONDITIONAL_TEST(Buffer_Clear, TestType::COMMON)
	{
		// Create buffer and fill with non-zero data
		Buffer buf(5);
		buf.full(0xFF);

		// Ensure buffer is cleared to zeroes
		buf.clear();
		for(size_t i = 0; i < buf.capacity(); i++)
			if(!compare(buf.read(0), 0x00))
				return false;

		return true;
	});

	MAXOS_CONDITIONAL_TEST(Buffer_Full, TestType::COMMON)
	{
		// Create buffer and fill with specific byte
		Buffer buf(5);
		buf.full(0xAA);

		// Ensure buffer is filled with the correct byte
		for(size_t i = 0; i < buf.capacity(); i++)
			if(!compare(buf.read(0), 0xAA))
				return false;

		return true;
	});

	MAXOS_CONDITIONAL_TEST(Buffer_WriteRead_Offset, TestType::COMMON)
	{
		// Setup buffer with byes at specific offsets
		Buffer buf(3);
		buf.write(0x12);
		buf.write(1, 0x34);

		// Read back bytes and verify
		if(!compare(buf.read(0), 0x12)) return false;
		if(!compare(buf.read(1), 0x34)) return false;
		return true;
	});

	MAXOS_CONDITIONAL_TEST(Buffer_Resize, TestType::COMMON)
	{
		Buffer buf(5);
		buf.resize(10);
		return compare(buf.capacity(), 10);
	});

	MAXOS_CONDITIONAL_TEST(Buffer_CopyFromBuffer, TestType::COMMON)
	{
		// Create source and destination buffers with known data
		Buffer src(5);
		Buffer dst(5);
		src.full(0x55);

		// Ensure data is copied correctly
		dst.copy_from(&src);
		for(size_t i = 0; i < dst.capacity(); i++)
			if(!compare(dst.read(0), 0x55))
				return false;

		return true;
	});

	MAXOS_CONDITIONAL_TEST(Buffer_CopyToBuffer, TestType::COMMON)
	{
		// Create source and destination buffers with known data
		Buffer src(5);
		Buffer dst(5);
		src.full(0x77);

		// Ensure data is copied correctly
		src.copy_to(&dst);
		for(size_t i = 0; i < dst.capacity(); i++)
			if(!compare(dst.read(0), 0x77))
				return false;

		return true;
	});

	MAXOS_CONDITIONAL_TEST(Buffer_CopyFromRawPointer, TestType::COMMON)
	{
		// Create raw data and copy into buffer
		uint8_t data[3] = { 0x01, 0x02, 0x03 };
		Buffer buf(3);
		buf.copy_from(data, 3);

		// Ensure data is copied correctly
		return compare(buf.read(0), 0x01)
		       && compare(buf.read(1), 0x02)
		       && compare(buf.read(2), 0x03);
	});

	MAXOS_CONDITIONAL_TEST(Buffer_CopyToRawPointer, TestType::COMMON)
	{
		// Create buffer with known data
		Buffer buf(3);
		buf.full(0x99);

		// Copy data to raw pointer
		uint8_t out[3] = { 0 };
		buf.copy_to(out, 3);

		return compare(out[0], 0x99)
		       && compare(out[1], 0x99)
		       && compare(out[2], 0x99);
	});

	MAXOS_CONDITIONAL_TEST(Buffer_Offset_Update, TestType::COMMON)
	{
		// Create a buffer and fill with data that will move the offset
		Buffer buf(3, true);
		buf.write(0x10);
		buf.write(0x20);

		// Overwrite first byte and check offset moved correctly
		buf.set_offset(0);
		buf.write(0x30);
		return compare(buf.read(0), 0x30)
		       && compare(buf.read(1), 0x20)
		       && compare(buf.read(2), 0x00);
	});

	MAXOS_CONDITIONAL_TEST(Buffer_Offset_Disabled, TestType::COMMON)
	{
		// Create a buffer and fill with data that will move the offset
		Buffer buf(3, false);
		buf.write(0x11);
		buf.write(0x22);

		// Overwrite first byte and check offset did not move
		buf.set_offset(0);
		buf.write(0x33);
		return compare(buf.read(0), 0x33)
		       && compare(buf.read(1), 0x22)
		       && compare(buf.read(2), 0x00);
	});

	MAXOS_CONDITIONAL_TEST(Buffer_CopyPartial_FromBuffer, TestType::COMMON)
	{
		// Setup source and destination buffers
		Buffer src(5);
		Buffer dst(5);
		src.full(0x55);

		// Verify that only part of the buffer is copied
		dst.copy_from(&src, 3);
		return dst.read(0) == 0x55 && dst.read(1) == 0x55 && dst.read(2) == 0x55 && dst.read(3) == 0x00;
	});

	MAXOS_CONDITIONAL_TEST(Buffer_CopyPartial_ToBuffer, TestType::COMMON)
	{
		// Setup source and destination buffers
		Buffer src(5);
		Buffer dst(5);
		src.full(0x66);

		// Verify that only part of the buffer is copied
		src.copy_to(&dst, 3);
		return compare(dst.read(0), 0x66)
		       && compare(dst.read(1), 0x66)
		       && compare(dst.read(2), 0x66)
		       && compare(dst.read(3), 0x00);
	});

	MAXOS_CONDITIONAL_TEST(Buffer_Full_Partial, TestType::COMMON)
	{
		Buffer buf(5);
		buf.full(0x99, 1, 3);
		return compare(buf.read(0), 0x00)
		       && compare(buf.read(1), 0x99)
		       && compare(buf.read(2), 0x99)
		       && compare(buf.read(3), 0x99)
		       && compare(buf.read(4), 0x00);
	});

	// TODO: Test bounds checking (some sorta exception handling)
}

/**
 * @brief Registers all colour tests
 */
void register_colour_tests() {

	MAXOS_CONDITIONAL_TEST(Colour_DefaultConstructor, TestType::COMMON)
	{
		Colour c;
		return compare(c.red, 0)
		       && compare(c.green, 0)
		       && compare(c.blue, 0)
		       && compare(c.alpha, 0);
	});

	MAXOS_CONDITIONAL_TEST(Colour_RGBConstructor, TestType::COMMON)
	{
		Colour c(10, 20, 30);
		return compare(c.red, 10)
		       && compare(c.green, 20)
		       && compare(c.blue, 30)
		       && compare(c.alpha, 0);
	});

	MAXOS_CONDITIONAL_TEST(Colour_RGBAConstructor, TestType::COMMON)
	{
		Colour c(10, 20, 30, 40);
		return compare(c.red, 10)
		       && compare(c.green, 20)
		       && compare(c.blue, 30)
		       && compare(c.alpha, 40);
	});

	MAXOS_CONDITIONAL_TEST(Colour_ConsoleColourConstructor_Red, TestType::COMMON)
	{
		Colour c(ConsoleColour::Red);
		return compare(c.red, 255)
		       && compare(c.green, 0)
		       && compare(c.blue, 0);
	});

	MAXOS_CONDITIONAL_TEST(Colour_HexStringConstructor_Magenta, TestType::COMMON)
	{
		Colour c("FF00FF");
		return compare(c.red, 255)
		       && compare(c.green, 0)
		       && compare(c.blue, 255);
	});

	MAXOS_CONDITIONAL_TEST(Colour_ANSIStringConstructor_FGGreen, TestType::COMMON)
	{
		Colour c("\033[0;32m");
		return compare(c.red, 0)
		       && compare(c.green, 255)
		       && compare(c.blue, 0);
	});

	MAXOS_CONDITIONAL_TEST(Colour_ToConsoleColour_Red, TestType::COMMON)
	{
		Colour c(255, 0, 0);
		return c.to_console_colour() == ConsoleColour::Red;
	});

	MAXOS_CONDITIONAL_TEST(Colour_ConsoleColourRoundTrip_Blue, TestType::COMMON)
	{
		Colour c(ConsoleColour::Blue);
		return c.to_console_colour() == ConsoleColour::Blue;
	});

	MAXOS_CONDITIONAL_TEST(Colour_AlphaPreservation, TestType::COMMON)
	{
		Colour c(10, 20, 30, 128);
		return compare(c.alpha, 128);
	});

}

/**
 * @brief Registers all map tests
 */
void register_map_tests() {

	MAXOS_CONDITIONAL_TEST(Map_EmptyOnConstruct, TestType::COMMON)
	{
		Map<int, int> m;
		return compare(m.empty(), true);
	});

	MAXOS_CONDITIONAL_TEST(Map_PushBack_SizeAndFind, TestType::COMMON)
	{
		// Create map and add an element
		Map<int, int> m;
		m.push_back(1, 100);

		// Verify size is correct
		if(!compare(m.size(), 1))
			return false;

		// Verify element can be found
		auto it = m.find(1);
		return it != m.end() && compare(it->first, 1) && compare(it->second, 100);
	});

	MAXOS_CONDITIONAL_TEST(Map_PushBack_PopBack, TestType::COMMON)
	{
		// Create map and add elements
		Map<int, int> m;
		m.push_back(1, 10);
		m.push_back(2, 20);

		// Verify size
		if(!compare(m.size(), 2))
			return false;

		// Confirm last element is correct
		Pair<int, int> last = m.pop_back();
		if(!compare(last.first, 2)) return false;
		if(!compare(last.second, 20)) return false;

		// Verify size after pop
		return compare(m.size(), 1);
	});

	MAXOS_CONDITIONAL_TEST(Map_PushFront_PopFront, TestType::COMMON)
	{
		// Create map and add elements
		Map<int, int> m;
		m.push_back(2, 20);
		m.push_front(1, 10);

		// Verify size
		if(!compare(m.size(), 2))
			return false;

		// Confirm first element is correct
		Pair<int, int> first = m.pop_front();
		if(!compare(first.first, 1)) return false;
		if(!compare(first.second, 10)) return false;

		// Verify size after pop
		return compare(m.size(), 1);
	});

	MAXOS_CONDITIONAL_TEST(Map_Insert_AddAndUpdate, TestType::COMMON)
	{
		// Create map and add an element
		Map<int, int> m;
		m.insert(1, 100);

		// Verify size after insert
		if(!compare(m.size(), 1))
			return false;

		// Update the existing element and verify size remains the same
		m.insert(1, 200);
		if(!compare(m.size(), 1)) return false;

		// Verify the value was updated
		auto it = m.find(1);
		if(it == m.end()) return false;
		return compare(it->second, 200);
	});

	MAXOS_CONDITIONAL_TEST(Map_Erase_ByKey, TestType::COMMON)
	{
		// Create map and add elements
		Map<int, int> m;
		m.push_back(1, 10);
		m.push_back(2, 20);

		// Verify size before erase
		if(!compare(m.size(), 2)) return false;

		// Erase one element and verify size and contents
		m.erase(1);
		if(!compare(m.size(), 1)) return false;
		return (m.find(1) == m.end()) && (m.find(2) != m.end());
	});

	MAXOS_CONDITIONAL_TEST(Map_Erase_ByIterator, TestType::COMMON)
	{
		// Create map and add elements
		Map<int, int> m;
		m.push_back(1, 11);
		m.push_back(2, 22);

		// Verify size before erase
		if(!compare(m.size(), 2)) return false;

		// Erase one element
		auto it = m.find(1);
		if(it == m.end()) return false;
		m.erase(it);

		// Verify size and contents after erase
		if(!compare(m.size(), 1)) return false;
		return (m.find(1) == m.end()) && (m.find(2) != m.end());
	});

	MAXOS_CONDITIONAL_TEST(Map_Clear_Empties, TestType::COMMON)
	{
		// Create map and add elements
		Map<int, int> m;
		m.push_back(1, 1);
		m.push_back(2, 2);

		// Clear the map and verify it's empty
		m.clear();
		return compare(m.empty(), true) && compare(m.size(), 0);
	});

	MAXOS_CONDITIONAL_TEST(Map_ReserveAndIncreaseSize, TestType::COMMON)
	{
		// Create map and reserve space
		Map<int, int> m;
		m.reserve(16);

		// Verify size is still zero
		if(!compare(m.size(), 0)) return false;

		// Increase size and verify still zero //TODO: also verify capacity increased?
		m.increase_size();
		return compare(m.size(), 0);
	});
}

/**
 * @brief Registers all rectangle tests
 */
void register_rectangle_tests() {

	MAXOS_CONDITIONAL_TEST(Rectangle_DefaultConstructor, TestType::COMMON)
	{
		Rectangle<int> r;

		if(!compare(r.left, 0)) return false;
		if(!compare(r.top, 0)) return false;
		if(!compare(r.width, 0)) return false;
		if(!compare(r.height, 0)) return false;
		return true;
	});

	MAXOS_CONDITIONAL_TEST(Rectangle_Construct_PositiveValues, TestType::COMMON)
	{
		Rectangle<int> r(1, 2, 3, 4);

		if(!compare(r.left, 1)) return false;
		if(!compare(r.top, 2)) return false;
		if(!compare(r.width, 3)) return false;
		if(!compare(r.height, 4)) return false;
		return true;
	});

	MAXOS_CONDITIONAL_TEST(Rectangle_Construct_NegativeSize_Normalises, TestType::COMMON)
	{
		Rectangle<int> r(-2, -3, -4, -5);

		if(!compare(r.left, -6)) return false;
		if(!compare(r.top, -8)) return false;
		if(!compare(r.width, 4)) return false;
		if(!compare(r.height, 5)) return false;
		return true;
	});


	MAXOS_CONDITIONAL_TEST(Rectangle_Intersects_Overlaps, TestType::COMMON)
	{
		Rectangle<int> a(0, 0, 10, 10);
		Rectangle<int> b(5, 5, 2, 2);

		return compare(a.intersects(b), true);
	});

	MAXOS_CONDITIONAL_TEST(Rectangle_Intersects_EdgeTouch_NoIntersect, TestType::COMMON)
	{
		Rectangle<int> a(0, 0, 10, 10);
		Rectangle<int> b(10, 10, 5, 5);
		Rectangle<int> c(11, 0, 2, 2);

		if(!compare(a.intersects(b), false)) return false;
		if(!compare(a.intersects(c), false)) return false;
		return true;
	});

	MAXOS_CONDITIONAL_TEST(Rectangle_Intersection_CorrectValues, TestType::COMMON)
	{
		Rectangle<int> a(0, 0, 10, 10);
		Rectangle<int> b(5, 5, 10, 10);
		Rectangle<int> inter = a.intersection(b);

		if(!compare(inter.left, 5)) return false;
		if(!compare(inter.top, 5)) return false;
		if(!compare(inter.width, 5)) return false;
		if(!compare(inter.height, 5)) return false;
		return true;
	});

	MAXOS_CONDITIONAL_TEST(Rectangle_Contains_Rectangle_Inside, TestType::COMMON)
	{
		Rectangle<int> outer(0, 0, 10, 10);
		Rectangle<int> inner(2, 2, 3, 3);

		return compare(outer.contains(inner), true);
	});

	MAXOS_CONDITIONAL_TEST(Rectangle_Contains_Rectangle_EdgeTouch_False, TestType::COMMON)
	{
		Rectangle<int> a(0, 0, 4, 4);
		Rectangle<int> b(4, 1, 2, 2);
		Rectangle<int> c(-1, -1, 1, 1);

		if(!compare(a.contains(b), false)) return false;
		if(!compare(a.contains(c), false)) return false;
		return true;
	});

	MAXOS_CONDITIONAL_TEST(Rectangle_Contains_Point, TestType::COMMON)
	{
		Rectangle<int> r(1, 1, 4, 4);

		if(!compare(r.contains(1, 1), true)) return false;
		if(!compare(r.contains(4, 1), false)) return false;
		if(!compare(r.contains(3, 3), true)) return false;
		if(!compare(r.contains(0, 0), false)) return false;
		return true;
	});

	MAXOS_CONDITIONAL_TEST(Rectangle_Subtract_NoIntersection_ReturnsOriginal, TestType::COMMON)
	{
		Rectangle<int> a(0, 0, 4, 4);
		Rectangle<int> b(5, 5, 2, 2);

		auto res = a.subtract(b);
		const Rectangle<int>& r = res[0];

		if(!compare((int)res.size(), 1)) return false;
		if(!compare(r.left, a.left)) return false;
		if(!compare(r.top, a.top)) return false;
		if(!compare(r.width, a.width)) return false;
		if(!compare(r.height, a.height)) return false;
		return true;
	});

	MAXOS_CONDITIONAL_TEST(Rectangle_Subtract_FullyCovered_EmptyResult, TestType::COMMON)
	{
		Rectangle<int> a(0, 0, 4, 4);
		Rectangle<int> b(0, 0, 4, 4);

		auto res = a.subtract(b);
		return compare((int)res.size(), 0);
	});

}

/**
 * @brief Registers all string tests
 */
void register_string_tests() {


	MAXOS_CONDITIONAL_TEST(String_DefaultConstructor, TestType::COMMON)
	{
		String s;
		return compare(s.length(), (size_t)0);
	});

	MAXOS_CONDITIONAL_TEST(String_CharConstructor, TestType::COMMON)
	{
		String s('A');
		return compare(s.length(), (size_t)1) && compare(s[0], 'A');
	});

	MAXOS_CONDITIONAL_TEST(String_CStringConstructor, TestType::COMMON)
	{
		String s("Hello");
		return compare(s.length(), (size_t)5) && compare(s, String("Hello"));
	});

	MAXOS_CONDITIONAL_TEST(String_CopyConstructor_Assignment, TestType::COMMON)
	{
		String a("CopyMe");
		String b(a);
		String c;
		c = a;
		return compare(b, a) && compare(c, a);
	});

	MAXOS_CONDITIONAL_TEST(String_Concat_Plus_PlusEquals, TestType::COMMON)
	{
		String a("Hello");
		String b("World");

		String combined = a + String(" ") + b;
		a += String(" ");
		a += b;

		return compare(combined, String("Hello World")) && compare(a, combined);
	});

	MAXOS_CONDITIONAL_TEST(String_Substring_StartsWith, TestType::COMMON)
	{
		String s("This is a test");
		String sub = s.substring(5, 2);

		return compare(sub, String("is")) && compare(s.starts_with(String("This")), true);
	});

	MAXOS_CONDITIONAL_TEST(String_Split_Simple, TestType::COMMON)
	{
		String s("one,two,three");
		auto parts = s.split(String(","));

		if(!compare((size_t)parts.size(), (size_t)3)) return false;
		if(!compare(parts[0], String("one"))) return false;
		if(!compare(parts[1], String("two"))) return false;
		return compare(parts[2], String("three"));
	});

	MAXOS_CONDITIONAL_TEST(String_Strip_Center, TestType::COMMON)
	{
		String s("   padded   ");
		String stripped = s.strip(' ');
		String centered = String("hi").center(6, '.');

		if(!compare(stripped, String("   padded"))) return false;
		return compare(centered, String("..hi.."));
	});

	MAXOS_CONDITIONAL_TEST(String_Formatted_Integer_Hex_String, TestType::COMMON)
	{
		String f = String::formatted("num:%d hex:%x str:%s", 42, (uint64_t)0x2A, (char*)"ok");

		return compare(f, String("num:42 hex:2A str:ok"));
	});

	MAXOS_CONDITIONAL_TEST(String_Length_AnsiHandling, TestType::COMMON)
	{
		String s = String("\033[0;31m") + String("X") + String("\033[0m") + String("Y");


		if(!compare(s.length(true), (size_t)8)) return false;
		return compare(s.length(false), (size_t)2);
	});

	MAXOS_CONDITIONAL_TEST(String_OperatorMultiply, TestType::COMMON)
	{
		String s("ab");
		String r = s * 3;

		return compare(r, String("ababab")) && compare(r.length(), (size_t)6);
	});

	MAXOS_CONDITIONAL_TEST(String_Comparisons_LexValue, TestType::COMMON)
	{
		String a("aaa");
		String b("bbb");

		if(!compare(a < b, true)) return false;
		if(!compare(b > a, true)) return false;
		if(!compare(a <= a, true)) return false;
		return compare(b >= a, true);
	});

	MAXOS_CONDITIONAL_TEST(String_StringBuilder_Append, TestType::COMMON)
	{
		StringBuilder sb;
		sb << "val:" << 123 << (uint64_t)0x10 << true;
		String out = sb.out;

		if(!compare(out.starts_with(String("val:")), true)) return false;
		if(!compare(out.length() > (size_t)5, true)) return false;
		return true;
	});

}

/**
 * @brief Registers all time tests
 */
void register_time_tests() {


	MAXOS_CONDITIONAL_TEST(Time_IsLeapYear_True, TestType::COMMON)
	{
		Time t { 2020, 2, 1, 0, 0, 0 };
		return compare(t.is_leap_year(), true);
	});

	MAXOS_CONDITIONAL_TEST(Time_IsLeapYear_False, TestType::COMMON)
	{
		Time t { 2019, 3, 1, 0, 0, 0 };
		return compare(t.is_leap_year(), false);
	});

	MAXOS_CONDITIONAL_TEST(Time_Epoch_1970_01_01, TestType::COMMON)
	{
		Time t { 1970, 1, 1, 0, 0, 0 };
		return compare(time_to_epoch(t), (uint64_t)0ULL);
	});

	MAXOS_CONDITIONAL_TEST(Time_Epoch_1970_01_02, TestType::COMMON)
	{
		Time t { 1970, 1, 2, 0, 0, 0 };
		return compare(time_to_epoch(t), (uint64_t)86400ULL);
	});

	MAXOS_CONDITIONAL_TEST(Time_Epoch_1971_01_01, TestType::COMMON)
	{
		Time t { 1971, 1, 1, 0, 0, 0 };
		return compare(time_to_epoch(t), (uint64_t)31536000ULL);
	});

	MAXOS_CONDITIONAL_TEST(Time_Epoch_1972_03_01_LeapYear, TestType::COMMON)
	{
		Time t { 1972, 3, 1, 0, 0, 0 };
		return compare(time_to_epoch(t), (uint64_t)68256000ULL);
	});

	MAXOS_CONDITIONAL_TEST(Time_MonthsArray_Bounds, TestType::COMMON)
	{
		return compare(string(Months[0]), string("January")) && compare(string(Months[11]), string("December"));
	});

}

/**
 * @brief Registers all vector tests
 */
void register_vector_tests() {

	MAXOS_CONDITIONAL_TEST(Vector_Default_Construct_Empty, TestType::COMMON)
	{
		Vector<int> v;
		return compare((int)v.size(), 0) && compare(v.empty(), true);
	});


	MAXOS_CONDITIONAL_TEST(Vector_Construct_Size_Fill, TestType::COMMON)
	{
		Vector<int> v(4, 7);
		return compare((size_t)v.size(), 4)
		       && compare(v[0], 7)
			   && compare(v[1], 7)
			   && compare(v[2], 7)
			   && compare(v[3], 7);
	});

	MAXOS_CONDITIONAL_TEST(Vector_PushBack_PopBack, TestType::COMMON)
	{
		Vector<int> v;
		v.push_back(1);
		v.push_back(2);
		v.push_back(3);

		if(!compare((size_t)v.size(), 3)) return false;

		int popped = v.pop_back();
		if(!compare(popped, 3)) return false;
		return compare((size_t)v.size(), 2);
	});

	MAXOS_CONDITIONAL_TEST(Vector_PushFront_PopFront, TestType::COMMON)
	{
		Vector<int> v;
		v.push_back(2);
		v.push_back(3);
		v.push_front(1);

		if(!compare((size_t)v.size(), 3)) return false;
		if(!compare(v[0], 1)) return false;

		int front = v.pop_front();
		if(!compare(front, 1)) return false;
		return compare((size_t)v.size(), 2);
	});

	MAXOS_CONDITIONAL_TEST(Vector_Reserve_IncreaseSize_Preserves, TestType::COMMON)
	{
		Vector<int> v;
		v.push_back(9);
		v.push_back(8);
		v.reserve(16);

		if(!compare((size_t)v.size(), 2)) return false;
		if(!compare(v[0], 9)) return false;
		if(!compare(v[1], 8)) return false;

		v.increase_size();
		return compare(v[0], 9);
	});

	MAXOS_CONDITIONAL_TEST(Vector_CopyConstructor_Independent, TestType::COMMON)
	{
		Vector<int> original;
		original.push_back(4);
		original.push_back(5);
		Vector<int> copy = original;

		original.pop_back();
		if(!compare((size_t)original.size(), 1)) return false;
		return compare((size_t)copy.size(), 2)
			&& compare(copy[1], 5);
	});


	MAXOS_CONDITIONAL_TEST(Vector_MoveConstructor_Transfers, TestType::COMMON)
	{
		Vector<int> src;
		src.push_back(10);
		src.push_back(11);
		Vector<int> moved = static_cast<Vector<int>&&>(src);

		if(!compare((size_t)moved.size(), 2)) return false;
		return compare((size_t)src.size(), 0);
	});

	MAXOS_CONDITIONAL_TEST(Vector_Assignment_Copy_And_Move, TestType::COMMON)
	{
		Vector<int> a;
		a.push_back(1);
		a.push_back(2);

		Vector<int> b;
		b = a;
		if(!compare((size_t)b.size(), 2)) return false;
		if(!compare(b[1], 2)) return false;

		Vector<int> c;
		c = static_cast<Vector<int>&&>(b); // move assign
		if(!compare((size_t)c.size(), 2)) return false;
		return compare((size_t)b.size(), 0);
	});

	MAXOS_CONDITIONAL_TEST(Vector_Erase_ByValue_RemovesAll, TestType::COMMON)
	{
		Vector<int> v;
		v.push_back(3);
		v.push_back(4);
		v.push_back(3);
		v.push_back(5);
		v.erase(3);

		if(!compare((size_t)v.size(), 2)) return false;
		return compare(v[0], 4)
			&& compare(v[1], 5);
	});

	MAXOS_CONDITIONAL_TEST(Vector_Erase_ByIterator_RemovesSingle, TestType::COMMON)
	{
		Vector<int> v;
		v.push_back(7);
		v.push_back(8);
		v.push_back(9);
		auto it = v.begin() + 1;
		v.erase(it);

		if(!compare((size_t)v.size(), 2)) return false;
		return compare(v[0], 7)
			&& compare(v[1], 9);
	});

	MAXOS_CONDITIONAL_TEST(Vector_Find_Present_And_Absent, TestType::COMMON)
	{
		Vector<int> v;
		v.push_back(21);
		v.push_back(22);
		v.push_back(23);

		bool found22 = (v.find(22) != v.end());
		bool found99 = (v.find(99) != v.end());

		if(!compare(found22, true)) return false;
		return compare(found99, false);
	});
}


/**
 * @brief Registers all common tests with the test runner
 */
void MaxOS::tests::register_tests_common() {
	register_buffer_tests();
	register_colour_tests();
	register_map_tests();
	register_rectangle_tests();
	register_string_tests();
	register_time_tests();
	register_vector_tests();
}