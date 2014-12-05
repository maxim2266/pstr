###A string class

####Motivation

In the modern C++ STL implementations the class `std::string` does not deal quite accurately with constant strings.
Each time someone writes
```C++
std::string s("xyz");`
```
the constant string literal gets copied to a heap allocated memory. Some implementations can handle
short strings without memory allocations, but still some unnecessary processing is involved, thus wasting
memory and CPU cycles, also making a contribution to the global warming. :)

The aim of the project is to find a way of improving this situation. The code provided is just a sketch,
not a production quality library, but it gives an idea about one possible way of solving the problem.

####Technical details

In the above example the actual `std::string` object creation happens in the `std::string(const char*)` constructor, which generally does the following:

1. Calculates the length of the string by invoking `strlen()` or an equivalent function. The length of the literal is known at the compile time, so this step is not needed. It should be noted that modern compilers can replace a
call to `strlen("xyz")` with constant `3`, but here we are at the mercy of the optimiser.
2. Allocates the memory for the string. May be not the case for short strings, but this is implementation-dependant.
3. Copies the literal to the allocated memory. Totally unnecessary.
4. At the end of the object life-time the allocated memory gets free'd in the destructor. Removing the memory allocation will naturally remove this step too.

The solution suggested in this project attempts to solve the above problems using the following techniques:

1. The string class itself is a pointer to implementation.
2. There are two implementation classes: one for the constant literals and one for the heap-allocated strings. The
heap-allocated strings are handled in the same way many usual string implementations do, while constant literals
are never copied, modified or free'd, which allows them to reside in the constant section of the binary (usually called `.rodata`). Naturally, both implementations have their string lengths stored inside the object.
3. Constant literals have their lengths calculated at compile time and stored at the beginning of the string,
the way similar to Pascal strings.
4. In C language, all string literals are subject for string pooling which makes sure no duplicate literals are put into the binary. In this implementation a similar effect is achieved by keeping all const string implementations in mergeable sections of the binary, thus allowing for the linker to keep only one instance of each const string.

The latter technique has turned out to be the trickiest to implement. Current solution uses a macro that generates
an assembly code which puts the literal and its length into the appropriate mergeable section. The code uses gcc's extended assembly, with the obvious portability issues, but the code actually generates no assembler instructions at all, only some data definitions, which can make porting a bit easier. Using those tricks is by far not a brilliant idea, but it is the only way I have found so far to achieve the goal, so please let me know if there is a better solution to this problem.

Another issue with the macro is that in gcc the extended assembly cannot be used outside a function, so at the moment there is no way to define a static const literal. One possible solution here could be to introduce another macro to generate a C++11 lambda function definition and then call it in-place, but this is not implemented yet.

Tested on Linux:

```bash
$ g++ --version
g++ (Ubuntu 4.8.2-19ubuntu1) 4.8.2
```

**Licence: BSD**
