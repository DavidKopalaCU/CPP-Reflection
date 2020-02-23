#ifndef __ELFREADER_H__
#define __ELFREADER_H__

#include <stdio.h>
#include <string.h>
#include <elf.h>
#include <sys/mman.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include <cxxabi.h>     // demangling

class ElfReader
{
private:

public:
    ElfReader(char *filepath);
    ~ElfReader();

    std::vector<Elf64_Shdr *> string_tables;
    size_t elf_size = 0;
    void *elf_data = nullptr;
    Elf64_Ehdr *elf_header = nullptr;
    void *symbol_table = nullptr;
    size_t symbol_table_size = 0;

    std::vector<Elf64_Sym *>* getSymbols();
    void printSymbol(Elf64_Sym *);
    char* nameForSymbol(Elf64_Sym *);

    void printSectionHeader();
    void printSymbolTable();

    std::vector<Elf64_Sym *>* lookupSymbol(char *symbol);
};


#endif // __ELFREADER_H__