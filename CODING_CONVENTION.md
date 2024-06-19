# Trace Coding Standard

Trace Engine has a few simple coding standards and conventions. This document reflects the state of Trace Engine current coding standards. Following the coding standards is mandatory.

# Context is Important

The rules and guidelines here derive from the tools we use, the architectures we work on, and the kind of problems we have to solve.


## Namespaces

The public API for core (and non-core) services the is related to the engine core features should be in the `trace`namespace and others should be wrapped in an appropriate namespace.

Do not use the using keyword. All namespaces should be explicitly referenced, everywhere.

Namespaces must not be hierarchical, they should be one level deep only.

Functions in namespace scope in source files should be fully qualified, and not rely on being enclosed in a namespace.

```cpp
// System.h
namespace System
{
  void Function();
}
```

```cpp
// RIGHT

// System.cpp
#include "System.h"

void System::Function()
{
}
```

```cpp
// WRONG

// System.cpp
#include "System.h"

namespace System
{
  void Function()
  {
  }
}
```

## Naming Conventions

When choosing a name, prefer long names to abbreviated names. Prefer descriptive names to arbitrary ones.

We generally use a PascalCase convention for types, identifiers.While camelCase is used for namespaces. In cases when a word is composed of acronyms, it is acceptable to either use all initials for the acronym portion, or PascalCase.

```cpp
namespace DDL // okay
{
  namespace threadLocal { ... }
}

namespace ddl // also okay
{
  namespace threadLocal { ... }
}

namespace ddl
{
  namespace ThreadLocal { ... } // WRONG
}

namespace Ddl
{
  namespace THREADLOCAL { ... } // WRONG
}
```

Variables have a different naming convention depending on their scope:
- Type and variable names are nouns.
- Method names are verbs that either describe the method's effect, or the return value of a method without an effect.
- Macro names should be fully capitalized with words separated by underscores, and prefixed with `TRC_`.

```cpp
#define TRC_SHADER_ID

```

Variable, method, and class names should be:

- Clear
- Unambiguous
- Descriptive

All variables should be declared on their own line so that you can provide comment on the meaning of each variable.

We encourage you to prefix function parameter names with "out_" if:

- The function parameters are passed by reference.
- The function is expected to write to that value.

This makes it obvious that the value passed in this argument is replaced by the function.

If an In or Out parameter is also a boolean, put in/out prefix, such as `out_result`.

Functions that return a value should describe the return value. The name should make clear what value the function returns. This is particularly important for boolean functions. Consider the following two example methods:

```cpp
// what does true mean?
bool CheckBox(Box box);
 
// name makes it clear true means tea is fresh
bool IsBoxFull(Box box);
 
float boxWeight;
int32 boxCount;
```

### Local variables

- No trailing underscores

Note: this applies to function parameters as well.

`lower_case_underscores`


### Member variables

- Initial m_
- Use CamelCase
- No other underscores

`m_camelCase`

### Type names (struct, class, enum)


- PascalCase
- No underscores.

`ModelInstance`

### Portable C++ code
The int and unsigned int types vary in size across platforms. They are guaranteed to be at least 32 bits in width and are acceptable in code when the integer width is unimportant. Explicitly-sized types are used in serialized or replicated formats.

Below is a list of common types:

- ``bool`` for boolean values (NEVER assume the size of bool). BOOL will not compile.
- `uint8_t` for unsigned bytes (1 byte).
- `int8_t` for signed bytes (1 byte).
- `uint16_t` for unsigned shorts (2 bytes).
- `int16_t` for signed shorts (2 bytes).
- `uint32_t` for unsigned ints (4 bytes).
- `int32_t` for signed ints (4 bytes).
- `uint64_t` for unsigned quad words (8 bytes).
- `int64_t` for signed quad words (8 bytes).
- `float` for single precision floating point (4 bytes).
- `double` for double precision floating point (8 bytes).

### Guidelines

- Write self-documenting code. For example:

```cpp
// Bad:
t = s + l - b;
 
// Good:
total_leaves = small_leaves + large_leaves - small_and_large_leaves;

```

- Write useful comments. For example:

```cpp
// Bad:
// increment leaves
++leaves;
 
// Good:
// we know there is another leaf
++leaves;

```

- Do not over comment bad code — rewrite it instead. For example:

```cpp
// Bad:
// total number of leaves is sum of
// small and large leaves less the
// number of leaves that are both
t = s + l - b;
 
// Good:
total_leaves = small_leaves + large_leaves - small_and_large_leaves;


```


### Nullptr

You should use `nullptr` instead of the C-style NULL macro in all cases.

### Auto

You shouldn't use ``auto`` in C++ code, except for the few exceptions listed below. Always be explicit about the type you're initializing. This means that the type must be plainly visible to the reader.

C++17's structured binding feature should also not be used, as it is effectively a variadic ``auto``.

Acceptable use of ``auto``:

- When you need to bind a lambda to a variable, as lambda types are not expressible in code.
- For iterator variables, but only where the iterator's type is verbose and would impair readability.
- In template code, where the type of an expression cannot easily be discerned. This is an advanced case.

It's very important that types are clearly visible to someone who is reading the code. Even though some IDEs are able to infer the type, doing so relies on the code being in a compilable state. It also won't assist users of merge/diff tools, or when viewing individual source files in isolation, such as on GitHub.

If you're sure you are using ``auto`` in an acceptable way, always remember to correctly use ``const``, ``&``, or ``*`` just like you would with the type name. With ``auto``, this will coerce the inferred type to be what you want.

### Code Formatting

#### Braces

Always include braces in single-statement blocks. For example.:

```cpp
if (thing)
{
    return;
}
```

#### If - Else

Each block of execution in an if-else statement should be in braces. This helps prevent editing mistakes. When braces are not used, someone could unwittingly add another line to an if block. The extra line wouldn't be controlled by the if expression, which would be bad. It's also bad when conditionally compiled items cause if/else statements to break. So always use braces.

```cpp
if (game_exists)
{
    BuildGame();
}
else
{
    RunGame();
}
```

### Variable Declaration

- Do not use the comma operator to declare multiple variables on the same line. Declare them on separate lines.

```cpp
// WRONG
float* a, b;

// RIGHT
float* a;
float b;
```

### switch statements

```cpp
switch ()
{
  case x:
  {        <- Always use brackets. Even one liners.
    stuff
    break; <- With break before brackets.
  }
  <- If break is not desired, comment goes here.
}
```

# Coding Standard: Nomenclature

## Spelling

We use American English spelling throughout. So it is "Color" and not "Colour", "Serialize" and not "Serialise".

## Accessors

Accessors should start with "Set" or "Get". Exception: if an accessor reads the value of a bool, it may begin with "Is", or "Does", whichever reads more like natural English.

```cpp
void SetSpeed (float speed);
float GetSpeed() const;

void SetActive (bool active);
bool IsActive() const;

void SetSnapToGrid (bool snap);
bool DoesSnapToGrid() const;
```

## Opposites

### Create/Destroy

Of an object, where a C++ constructor is not appropriate. This is the case if an object can be re-created multiple times. Use this instead of placement new if possible. It makes the code clearer.

Create must return a success flag.

### Init/Shutdown

Used for systems and managers.

### Start/Stop

Start() is used to start something running, either synchronously or asynchronously, until Stop() is called. For example: a particle effect, a music track, a streaming system, or a timer.

### Suspend/Resume

For use between Start/Stop, to temporarily suspend that what is running.

### Begin/End

Used to bracket some kind of aggregation or definition, such as a triangle strip, command buffer, goal list.

### Enter/Exit

Of a state or mode.

### Add/Remove

An element in a collection.

### Open/Close

Of a data stream. Input/output, or between processes.

### Acquire/Release

Exclusive ownership of a resource. Acquire must return an error code.

# Coding Standards

## Use a struct for passive data carriers, a class for everything else

A struct may have associated constants, but lacks any functionality other than access to the data members. Field access is direct rather than through accessors. Methods should not provide behavior, they should only be used to set up the data members, e.g. constructor, destructor, Reset(), or Validate().

_Rationale_
This is to make a distinction between objects in the C++ sense (class) and data structures that aren't expected to be used in OOP fashion

## Data members of a struct must be public

A struct is used to conveniently tie together strongly related data, and it has no "moving parts". The need for accessors is minimal and it would reduce the convenience.

## Data members of a class must be private

All data members of a class must be accessed through accessors.

_Rationale_
Accessors allow a place for breakpoints, instrumentation, validation. They make it possible to change implementation without changing the interface. For example, you may want to replace a bool with a bit flag. They also allow you to run extra code when a value is changed. For example, you may want to set a flag to indicate that a change has occurred. Although you may not need any of these things at the time of initial implementation, you may need it later, or someone else may need it.

Note that this rule implies that class data members must not be protected.

The rationale for the use of accessors is not limited to the public interface, but also applies to inheritance. There should be exactly one point where a data member is accessed, and that is the accessors in the class where the data member is defined. If access to a data member must be restricted to derived classes, you can declare the accessors protected.

_Exceptions_
Member constants may be public.

Aggregates where a specific memory layout is required and instances are intended to be worked with in an aliased way may have public data members. For example, four-float vector objects should have public x, y, z, w.

# General Style Issues

- Minimize dependency distance.
  - When code depends on a variable having a certain value, try to set that variable's value right before using it. Initializing a variable at the top of an execution block, and not using it for a hundred lines of code, gives lots of space for someone to accidentally change the value without realizing the dependency. Having it on the next line makes it clear why the variable is initialized the way it is and where it is used.

- Split methods into sub-methods where possible.
  - It is easier for someone to look at a big picture, and then drill down to the interesting details, than it is to start with the details and reconstruct the big picture from them. In the same way, it is easier to understand a simple method, that calls a sequence of several well-named sub-methods, than it is to understand an equivalent method that simply contains all the code in those sub-methods.

- In function declarations or function call sites, do not add a space between the function's name and the parentheses that precede the argument list.

- Pointers and references should only have one space to the right of the pointer or reference.
  - This makes it easy to quickly use Find in Files for all pointers or references to a certain type. For example:

```cpp
// Use this
ActionType* ptr;
 
// Do not use these:
ActionType *ptr;
ActionType * ptr;
```

- Shadowed variables are not allowed.
  - C++ allows variables to be shadowed from an outer scope, but this makes usage ambiguous to a reader. For example, there are three usable Count variables in this member function:

```cpp

class SomeClass
{
public:
    void Func(const int32_t count)
    {
        for (int32 count = 0; count != 10; ++count)
        {
            // Use count
        }
    }
 
private:
    int32_t m_count;
}
```

- Avoid using anonymous literals in function calls.
  - Prefer named constants which describe their meaning. This makes intent more obvious to a casual reader as it avoids the need to look up the function declaration to understand it.

```cpp
// Wrong
RunToPosition("Soldier", 5, true);.
 
// Right
const std::string object_name = "Soldier";
const float cooldown_in_seconds = 5;
const bool vulnerable_during_cooldown  = true;
RunToPosition(object_name, cooldown_in_seconds, vulnerable_during_cooldown);

```