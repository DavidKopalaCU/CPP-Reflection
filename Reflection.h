#ifndef __REFLECTION_H___
#define __REFLECTION_H__

#include <unistd.h>     // getpid
#include <sys/types.h>  // pid_t
#include <stdio.h>
#include <regex>

#include "ElfReader.h"

class Reflection
{
private:
    void *base_address = nullptr;
public:
    Reflection(ElfReader *_elf, void *_main);
    ~Reflection();

    static Reflection* FromCurrentProcess(void *_main);
    int exec(std::string, void *);

    ElfReader *elf;

    Elf64_Sym* promptForSymbolSelection(std::vector<Elf64_Sym *> *symbols);
};


#endif // __REFLECTION_H__