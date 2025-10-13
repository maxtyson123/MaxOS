# Freestanding C++ Guidelines for MaxOS

This document provides guidelines for writing C++ code in MaxOS's freestanding environment.

## What is Freestanding C++?

MaxOS is a freestanding C++ environment, meaning:
- No hosted C++ standard library (no `std::string`, `std::vector`, etc.)
- No exceptions (compiled with `-fno-exceptions`)
- No RTTI (compiled with `-fno-rtti`)
- No standard library runtime (compiled with `-nostdlib`)
- Must provide own implementations of essential functionality

## Allowed Standard Headers

### C++ Freestanding Headers (Allowed)

The following standard library headers are part of the freestanding implementation and MAY be used:

#### Basic Types and Limits
- `<cstddef>` or `<stddef.h>` - `size_t`, `ptrdiff_t`, `nullptr_t`, `NULL`
- `<cstdint>` or `<stdint.h>` - Fixed-width integer types (`uint8_t`, `int32_t`, etc.)
- `<limits>` - Numeric limits (C++20 freestanding)
- `<climits>` or `<limits.h>` - Integer limit macros (`INT_MAX`, etc.)
- `<cfloat>` or `<float.h>` - Floating-point limit macros

#### Type Traits and Metaprogramming (C++20 Freestanding)
- `<type_traits>` - Type traits for compile-time type information
- `<ratio>` - Compile-time rational arithmetic
- `<version>` - Feature-test macros

#### Utilities
- `<cstdarg>` or `<stdarg.h>` - Variable argument lists
- `<initializer_list>` - Support for `{}`-initialization
- `<new>` - Placement new, alignment, `std::nothrow`

#### Atomic Operations (Partial, Hardware Dependent)
- `<atomic>` - Lock-free atomic operations (if hardware supports)

#### Concepts and Coroutines (C++20)
- `<concepts>` - Standard concepts
- `<coroutine>` - Coroutine support

### Hosted Headers (FORBIDDEN)

The following headers require a hosted environment and MUST NOT be used:

- `<string>`, `<string_view>` - Use custom `common/string.h` instead
- `<vector>`, `<array>`, `<deque>` - Use custom `common/vector.h` instead
- `<map>`, `<unordered_map>`, `<set>` - Use custom `common/map.h` instead
- `<algorithm>`, `<functional>` - Implement needed algorithms manually
- `<iostream>`, `<fstream>`, `<sstream>` - Use custom stream classes
- `<memory>` - Implement custom memory management
- `<exception>`, `<stdexcept>` - Exceptions are disabled
- `<thread>`, `<mutex>`, `<condition_variable>` - Implement custom threading
- Any other STL container or algorithm header

## Custom Implementations

MaxOS provides custom implementations of common data structures:

### Data Structures
- `common/string.h` - String class with dynamic and small-string optimization
- `common/vector.h` - Dynamic array template
- `common/map.h` - Key-value map template
- `common/pair.h` - Pair template
- `common/buffer.h` - Buffer management

### Streams
- `common/inputStream.h` - Input stream abstraction
- `common/outputStream.h` - Output stream abstraction

### Memory Management
- `memory/memorymanagement.h` - Custom memory allocator
- Placement new operators defined in kernel

## Best Practices

### 1. Use Freestanding Headers for Type Safety

**GOOD:**
```cpp
#include <cstdint>
#include <type_traits>

template<typename T>
void process_value(T value) {
    static_assert(std::is_integral<T>::value, "T must be integral");
    // ...
}
```

**AVOID:**
```cpp
// Don't redefine standard types
typedef unsigned long long uint64_t;  // Already in <cstdint>
```

### 2. Implement Only What You Need

Don't try to replicate the entire STL. Implement features as needed:

**GOOD:**
```cpp
// Simple, focused implementation
class String {
    char* m_data;
    size_t m_length;
public:
    // Only essential methods
    const char* c_str() const;
    size_t length() const;
    // ...
};
```

**AVOID:**
```cpp
// Don't implement every possible std::string method
class String {
    // Hundreds of methods trying to match std::string exactly
    // Many will never be used in kernel code
};
```

### 3. Use Modern C++ Features

C++20 freestanding includes many modern features:

**GOOD:**
```cpp
// Use constexpr for compile-time computation
constexpr size_t page_size() {
    return 4096;
}

// Use concepts if available
template<typename T>
concept Integral = std::is_integral_v<T>;

template<Integral T>
void process(T value) { /* ... */ }

// Use [[nodiscard]] to prevent bugs
[[nodiscard]] bool allocate_memory(size_t size);
```

### 4. Avoid Hidden Allocations

In kernel code, all allocations should be explicit:

**GOOD:**
```cpp
// Explicit allocation
auto* buffer = new uint8_t[size];
// ... use buffer ...
delete[] buffer;

// Or use custom allocator
String str;
str.reserve(100);  // Explicit capacity
```

**AVOID:**
```cpp
// Hidden allocations in constructors
String str1 = str2 + str3;  // OK if documented
// But be aware this allocates
```

### 5. Handle Allocation Failures

Without exceptions, handle allocation failures explicitly:

**GOOD:**
```cpp
void* operator new(size_t size) {
    void* ptr = allocate(size);
    if (!ptr) {
        // Handle failure - perhaps panic or return null
        PANIC("Out of memory");
    }
    return ptr;
}

// Or use nothrow version
void* ptr = operator new(size, std::nothrow);
if (!ptr) {
    // Handle failure
}
```

### 6. Use Compile-Time Checks

Leverage `static_assert` and `constexpr`:

**GOOD:**
```cpp
#include <type_traits>

template<typename T>
class Array {
    static_assert(std::is_trivially_copyable<T>::value,
                  "Array elements must be trivially copyable");
    // ...
};

// Verify freestanding mode
#if __STDC_HOSTED__ != 0
#error "Must compile in freestanding mode"
#endif
```

### 7. Document Custom Implementations

Make it clear when using custom implementations:

**GOOD:**
```cpp
/**
 * @brief Custom string implementation for freestanding environment.
 * 
 * This class provides basic string functionality without relying on
 * the standard library. Uses small-string optimization for strings
 * under MAX_STRING_SMALL_STORAGE bytes.
 * 
 * @note This is NOT std::string. API may differ.
 */
class String {
    // ...
};
```

## Common Patterns

### Iteration Without STL Algorithms

**Instead of `std::for_each`:**
```cpp
// Range-based for loop
for (auto& item : container) {
    process(item);
}

// Or callback-based iteration
template<typename Callback>
void iterate(Callback callback) {
    for (size_t i = 0; i < size(); ++i) {
        callback(m_elements[i]);
    }
}
```

### Sorting Without `std::sort`

Implement needed sorting algorithms:
```cpp
// Simple bubble sort for small arrays
template<typename T>
void sort(T* array, size_t size) {
    for (size_t i = 0; i < size - 1; ++i) {
        for (size_t j = 0; j < size - i - 1; ++j) {
            if (array[j] > array[j + 1]) {
                T temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
        }
    }
}
```

### Move Semantics Without std::move

```cpp
// Implement move constructors/assignment
template<typename T>
class Vector {
    Vector(Vector&& other) noexcept
        : m_data(other.m_data)
        , m_size(other.m_size)
    {
        other.m_data = nullptr;
        other.m_size = 0;
    }
};

// Manual move using placement new
template<typename T>
T&& move(T& value) noexcept {
    return static_cast<T&&>(value);
}
```

## Memory Management

### New/Delete Operators

MaxOS must provide its own `operator new` and `operator delete`:

```cpp
void* operator new(size_t size);
void* operator new[](size_t size);
void operator delete(void* ptr) noexcept;
void operator delete[](void* ptr) noexcept;

// Placement new (from <new>)
void* operator new(size_t, void* ptr) noexcept { return ptr; }
```

### Alignment

Use `alignof` and `alignas` for proper alignment:

```cpp
// Ensure cache-line alignment
alignas(64) struct CacheAlignedData {
    uint64_t values[8];
};

// Check alignment at compile time
static_assert(alignof(CacheAlignedData) == 64);
```

## Testing Freestanding Code

### Verify Freestanding Compliance

```cpp
// In a common header
#ifndef __STDC_HOSTED__
#error "Compiler does not define __STDC_HOSTED__"
#endif

#if __STDC_HOSTED__ != 0
#error "Code must be compiled in freestanding mode (-ffreestanding)"
#endif

// Verify essential features
static_assert(__cplusplus >= 202002L, "Requires C++20 or later");
```

### Check for Accidental Standard Library Use

```bash
# Find any std:: usage
grep -r "std::" --include="*.cpp" --include="*.h" .

# Find hosted headers
grep -r "#include <string>" --include="*.cpp" --include="*.h" .
grep -r "#include <vector>" --include="*.cpp" --include="*.h" .
```

## Migration from Hosted to Freestanding

If porting code from a hosted environment:

1. **Replace STL containers:**
   - `std::string` → `MaxOS::String`
   - `std::vector` → `MaxOS::common::Vector`
   - `std::map` → `MaxOS::common::Map`

2. **Replace algorithms:**
   - Implement needed algorithms manually
   - Use range-based for loops

3. **Handle errors without exceptions:**
   - Return error codes
   - Use nullable pointers
   - Use `[[nodiscard]]` on error-returning functions

4. **Replace iostream:**
   - Use custom `InputStream`/`OutputStream`
   - Use `Logger` for diagnostic output

## References

- C++20 Standard, Section 16.5.1.3 "Freestanding implementations"
- GCC documentation on `-ffreestanding`
- [cppreference.com - Freestanding implementations](https://en.cppreference.com/w/cpp/freestanding)

## Summary

**DO:**
- ✅ Use `<cstdint>`, `<cstddef>`, `<type_traits>`, `<limits>`
- ✅ Implement custom data structures (String, Vector, Map)
- ✅ Use modern C++ features (constexpr, concepts, [[nodiscard]])
- ✅ Handle allocations explicitly
- ✅ Use compile-time checks

**DON'T:**
- ❌ Use hosted STL headers (`<string>`, `<vector>`, `<algorithm>`)
- ❌ Assume exceptions work (they're disabled)
- ❌ Use RTTI (`dynamic_cast`, `typeid`)
- ❌ Rely on global constructors/destructors (use explicit initialization)
- ❌ Use standard library runtime functions without reimplementing them

---

*Last Updated: 2025-01-13*  
*For MaxOS Kernel Development*
