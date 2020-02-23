# C++ Runtime Reflection Engine

This project provides files that can be included in any C++ project running on Linux that provide the ability for the program to identify and utilize data and function objects from just a string, without having to manually register each attribute.

![gif of it working](/docs/reflection.gif)

From [Wikipedia](https://en.wikipedia.org/wiki/Reflection_(computer_programming)),

> Reflection is the ability of a process to examine, introspect, and modify its own structure and behavior.

This feature is available in most modern languages, including Python and Javascript. Reflection allows the developer/user to gain information about how the program is structured, ie what functions are available, where they're located, what arguments they take, etc. However, it's distincly lacking from C++. Since C++ programs are compiled and linked before execution, the function names in your code ger replaced with the virtual address of where the machine can find them. While this greatly improves the performace of C++, there are some scenarios where it may be beneficial to know about how the program is structured. Consider an IoT device operating in a remote location that needs maintenance; rather than spending time writing diagnostic tools, a service member could remotely access the device and execute any of the code already used to control the device to identify and resolve the issue.

## How To Use It

Using this project is as easy as including `ElfReader.cpp`, `ElfReader.h`, `Reflection.cpp`, and `Reflection.h` in your project. At some point in your code, instantiate the `Reflection` Class as shown below, and it will automatically process the available symbols and make them available to the application.

```c++
Reflection *r = Reflection::FromCurrentProcess((void *)main);
```

## Technical Details

There are two classes that power this project; `Reflection` and `ElfReader`. `ElfReader` reads the ELF file describing the running process, and provides the symbols to `Reflection`. `Reflection` accepts arguments from the developer/user, and makes meaning from the symbols provided by `ElfReader`.

### ElfReader

Currently, the ElfReader gets the current process's ELF file by opening `/proc/<pid>/exe`. This is really just a symlink to the executable elsewhere in the filesystem, but using the `pid` is an easy way to get an absolute path, which is why we use it.

Once the ELF file is opened, `ElfReader` parses the Section Header Table to identify the `SHT_DYNSYM` entry, and makes a list of the `SHT_STRTAB` and `SHT_SYMTAB` entries.

`ElfReader` also provides a function to lookup a symbol by a string, `std::vector<Elf64_Sym*>* ElfReader::lookupSymbol(char *)`. This method goes through every entry in `SHT_DYNSYM` and resolves its name by looking it up in one of the `SHT_STRTAB`s, and de-mangling it. If the name that we're looking for exists in the resolved name, the symbol is added to a list of matches, which is returned when all of the entries have been analyzed.

### Reflection

The `Reflection` Class should be instantiated by the `Reflection::FromCurrentProcess(void *main)` method, which automatically identifies the process's `pid` and load the `ElfReader`. Passing the location of `int main(int,char**)` allow the class to map the value of a symbol to a location in memory; `Reflection` will lookup the `main` symbol and calculate the difference between its value and the address passed to it. This offset is then added to any other symbol's value to determine it's location in memory.

The main method to note in `Reflection` is `int Reflection::exec(char*,void*)`. This currently allows the user to specify a function that they want to execute via the string method, and supply up to 6 arguments that are either strings or numbers. Due to the early stage of the project, only commands with one `.` seperator work. The fvalue of the command before `.` is used to identify an instance of a class, and the value after is used to determine the function to run. If no `.` is needed, it will try to find a global function.

Passing the arguments to a function is done through inline assembly code. The value is pulled from the string passed to the method and stored in the appropriate register. When all registers have populated, inline assembly code is used to call the function.

Calling functions on an object instantiated from a class is also possible. The identified object is simply passed 
as the first argument to the function in compliance with the machine's ABI.

## Future Development

### Investigate Support For MCUs

See if there's a way to also make this work for microcontrollers, where the code is flash directly and the ELF file is not available at runtime. May be limited to applications that have peripheral storage or implement an OS that loads the file.

### Reading Debug Symbols

Read debug symbols as well to provide more data and options to the user/developer. This could include access to local variables as well as source code and type matching.

### Attach To Another Process

Use the `ElfReader` to read a different process's ELF file, and use a system call like `strace` to take control of it.

### Load and Link Object Files

Use the `ElfReader` to read another ELF file. Load that program's code and link it to the existing process, therefore providing methods and functionality from the other object.
