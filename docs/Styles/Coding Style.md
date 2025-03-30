<!-- Modified from https://github.com/SerenityOS/serenity/blob/m_is_master/Documentation/CodingStyle.md -->
# MaxOS C++ coding style


This document describes the coding style used for C++ code in the Max Operating System project. All new code should conform to this style.

### Names

A combination of CamelCase, snake\_case, and SCREAMING\_CASE:

- Use CamelCase (Capitalize the m_first_memory_chunk letter, including all letters in an acronym) in a class, struct, or namespace name
- Use snake\_case (all lowercase, with underscores separating words) for variable and function names
- Use SCREAMING\_CASE for constants (both global and static member variables)

###### Right:

```cpp
struct Entry;
size_t buffer_size;
class FileDescriptor;
String absolute_path();
```

###### Wrong:

```cpp
struct data;
size_t bufferSize;
class Filedescriptor;
String MIME_Type();
```

Use full words, except in the rare case where an abbreviation would be more canonical and easier to understand.

###### Right:

```cpp
size_t character_size;
size_t length;
short tab_index; // More canonical.
```

###### Wrong:

```cpp
size_t char_size;
size_t len;
short tabulation_index; // Goofy.
```

Data members in C++ classes should be private. Static data members should be prefixed by "s\_". Other data members should be prefixed by "m\_". Global variables should be prefixed by "g\_".

###### Right:

```cpp
class String {
public:
    ...

private:
    int m_length = 0;
};
```

###### Wrong:

```cpp
class String {
public:
    ...

    int length = 0 ;
};
```

Precede setters with the word "set". Use bare words for getters. Setter and getter names should match the names of the variables being set/gotten.

###### Right:

```cpp
void set_count(int); // Sets m_count.
int count() const; // Returns m_count.
```

###### Wrong:

```cpp
void set_count(int); // Sets m_the_count.
int get_count() const; // Returns m_the_count.
```

Precede getters that return values throughout arguments with the word "get".

###### Right:

```cpp
void get_filename_and_inode_id(String&, InodeIdentifier&) const;
```

###### Wrong:

```cpp
void filename_and_inode_id(String&, InodeIdentifier&) const;
```

Use descriptive verbs in function names.

###### Right:

```cpp
bool convert_to_ascii(short*, size_t);
```

###### Wrong:

```cpp
bool to_ascii(short*, size_t);
```


Leave meaningless variable names out of function declarations. A good rule of thumb is if the parameter type name contains the parameter name (without trailing numbers or pluralization), then the parameter name isn't needed. Usually, there should be a parameter name for bools, strings, and numerical types.

###### Right:

```cpp
void set_count(int);

void do_something(Context*);
```

###### Wrong:

```cpp
void set_count(int count);

void do_something(Context* context);
```

Enum members should use InterCaps with an initial capital letter.

Prefer `const` to `#define`. Prefer inline functions to macros.

`#defined` constants should use all uppercase names with words separated by underscores.


### Other Punctuation

Constructors for C++ classes should initialize their members using C++ initializer syntax. Each member (and superclass) should be indented on a separate line, with the colon or comma preceding the member on that line. Prefer initialization at member definition whenever possible.

###### Right:

```cpp
class MyClass {
    ...
    Document* m_document = nullptr;
    int m_my_member = 0;
};

MyClass::MyClass(Document* document)
    : MySuperClass()
    , m_document(document)
{
}

MyOtherClass::MyOtherClass()
    : MySuperClass()
{
}
```

###### Wrong:

```cpp
MyClass::MyClass(Document* document) : MySuperClass()
{
    m_myMember = 0;
    m_document = document;
}

MyClass::MyClass(Document* document) : MySuperClass()
    : m_my_member(0) // This should be in the header.
{
    m_document = document;
}

MyOtherClass::MyOtherClass() : MySuperClass() {}
```

Prefer index or range-for over iterators in Vector iterations for terse, easier-to-read code.

###### Right:

```cpp
for (auto& child : m_children)
    child->do_child_thing();
```


#### OK:

```cpp
for (int i = 0; i < m_children.size(); ++i)
    m_children[i]->do_child_thing();
```

###### Wrong:

```cpp
for (auto it = m_children.begin(); it != m_children.end(); ++it)
    (*it)->do_child_thing();
```

### Pointers and References

Both pointer types and reference types should be written with no space between the type name and the `*` or `&`.

An out argument of a function should be passed by reference except rare cases where it is optional in which case it should be passed by pointer.

###### Right:

```cpp
void MyClass::get_some_value(OutArgumentType& out_argument) const
{
    out_argument = m_value;
}

void MyClass::do_something(OutArgumentType* out_argument) const
{
    do_the_thing();
    if (out_argument)
        *out_argument = m_value;
}
```

###### Wrong:

```cpp
void MyClass::get_some_value(OutArgumentType* outArgument) const
{
    *out_argument = m_value;
}
```

### Classes

For types with methods, prefer `class` over `struct`.

* For classes, make public getters and setters, keep members private with `m_` prefix.
* For structs, let everything be public and skip the `m_` prefix.

###### Right:

```cpp
struct Thingy {
    String name;
    int frob_count = 0;
};

class Doohickey {
public:
    String const& name() const { return m_name; }
    int frob_count() const { return m_frob_count; }

    void jam();

private:
    String m_name;
    int m_frob_count = 0;
}
```

###### Wrong:

```cpp
struct Thingy {
public:
    String m_name;
    int frob_count() const { return m_frob_count; }

private:
    int m_frob_count = 0;
}

class Doohickey {
public:
    String const& name() const { return this->name; }

    void jam();

    String name;
    int frob_count = 0;
};
```


### Comments

Comments should be the line above the code they are describing. They should not be inline

Make comments look like sentences by starting with a capital letter and ending with a period (punctuation). One exception may be end of line comments like this `if (x == y) // false for NaN`.

Use TODO for things that need to be done, FIXME for things that need to be fixed, and HACK for things that are ugly but work. Use FIXME when you think there is a better way to do something, but you don't know what it is yet.


###### Right:

```cpp
// FIXME: Make this code handle jpg in addition to the png support.
draw_jpg(); 
```

###### Wrong:

```cpp
draw_jpg(); // FIXME(joe): Make this code handle jpg in addition to the png support.
```

```cpp
// TODO: Make this code handle jpg in addition to the png support.
draw_jpg(); 
```

Explain *why* the code does something. The code itself should already say what is happening.

###### Right:

```cpp
// Go to the next page.
i++; 
```

```cpp
// Let users toggle the advice functionality by clicking on catdog.
catdog_widget.on_click = [&] {
    if (advice_timer->is_active())
        advice_timer->stop();
    else
        advice_timer->start();
};
```

###### Even better:

```cpp
page_index++;
```

###### Wrong:

```cpp
i++; // Increment i.
```

```cpp
// If the user clicks, toggle the timer state.
catdog_widget.on_click = [&] {
    if (advice_timer->is_active())
        advice_timer->stop();
    else
        advice_timer->start();
};
```


### Doxygen

Functions and classes should be documented with Doxygen c

To document a function, use the following format:

```cpp
/**
 * @brief Brief description of the function.
 *
 * Longer description of the function.
 *
 * @param param_name Description of the parameter.
 * @return Description of the return value.
 */
```

To document a class, use the following format:
```cpp
/**
 * @class Class name
 * @brief Brief description of the class.
 *
 * Longer description of the class.
 */
```

###### Right:

```cpp
/**
 * @brief Returns the length of the string.
 *
 * @return The length of the string.
 */
size_t length() const;
```

###### Wrong:

```cpp
/**
 * @brief Returns the length of the string.
 */
size_t length() const;
```

```cpp
/**
 * @brief Returns the length of the string.
 *
 * @return The length of the string.
 */
size_t length(string const& str) const;
```


### Overriding Virtual Methods

The declaration of a virtual method inside a class must be declared with the `virtual` keyword. All subclasses of that class must also specify either the `override` keyword when overriding the virtual method, or the `final` keyword when overriding the virtual method and requiring that no further subclasses can override it.

###### Right:

```cpp
class Person {
public:
    virtual String description() { ... };
}

class Student : public Person {
public:
    virtual String description() override { ... }; // This is correct because it contains both the "virtual" and "override" keywords to indicate that the method is overridden.
}

```

```cpp
class Person {
public:
    virtual String description() { ... };
}

class Student : public Person {
public:
    virtual String description() final { ... }; // This is correct because it contains both the "virtual" and "final" keywords to indicate that the method is overridden and that no subclasses of "Student" can override "description".
}

```

###### Wrong:

```cpp
class Person {
public:
    virtual String description() { ... };
}

class Student : public Person {
public:
    String description() override { ... }; // This is incorrect because it uses only the "override" keyword to indicate that the method is virtual. Instead, it should use both the "virtual" and "override" keywords.
}
```

```cpp
class Person {
public:
    virtual String description() { ... };
}

class Student : public Person {
public:
    String description() final { ... }; // This is incorrect because it uses only the "final" keyword to indicate that the method is virtual and final. Instead, it should use both the "virtual" and "final" keywords.
}
```

```cpp
class Person {
public:
    virtual String description() { ... };
}

class Student : public Person {
public:
    virtual String description() { ... }; // This is incorrect because it uses only the "virtual" keyword to indicate that the method is overridden.
}
```

### Omission of curly braces from statement blocks

Curly braces may only be omitted from `if`/`else`/`for`/`while`/etc. statement blocks if the body is a single line.

Additionally, if anybody of a connected if/else statement requires curly braces according to this rule, all of them do.

###### Right:
```cpp
if (condition)
    foo();
```

```cpp
if (condition) {
    foo();
    bar();
}
```

```cpp
if (condition) {
    foo();
} else if (condition) {
    bar();
    baz();
} else {
    qux();
}
```

```cpp
for (size_t i = i; condition; ++i) {
    if (other_condition)
        foo();
}
```

##### OK:

```cpp
if (condition) {
    foo();
}
```

###### Wrong:

```cpp
if (condition)
    // There is a comment here.
    foo();
```

```cpp
if (condition)
    foo();
else {
    bar();
    baz();
} else
    qux();
```

```cpp
for (size_t i = i; condition; ++i)
    if (other_condition)
        foo();
```