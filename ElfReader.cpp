#include "ElfReader.h"

ElfReader::ElfReader(char *filepath)
{
    FILE *elf = fopen(filepath, "r");
    if (elf == nullptr) {
        printf("[ElfReader.cpp]::init(char *)\tCould not read ELF file! %s\n", filepath);
        perror("ELFReader");
    }

    // Measure the size of the ELF File
    if (fseek(elf, 0, SEEK_END)) {
        printf("[ElfReader.cpp]::init(char*)\tCould seek to the end of the ELF file!\n");
        fclose(elf);
        return;

    }
    this->elf_size = ftell(elf);

    // Copy the ELF File into program space
    this->elf_data = mmap(nullptr, this->elf_size, PROT_READ, MAP_PRIVATE, fileno(elf), 0);
    if (this->elf_data == nullptr) {
        perror("mmap");
        fclose(elf);;
        return;
    }

    // Load the ELF Header
    this->elf_header = (Elf64_Ehdr *) this->elf_data;
    const uint8_t magic_header[] = { ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3 };
    if (memcmp(this->elf_header, magic_header, sizeof(magic_header)) != 0) {
        printf("[ElfHeader.cpp]::init(char*)\tThe file specified is not an ELF file!\n");
        munmap(elf_data, elf_size);
        return;
    }

    // Locate symbol table
    for (uint16_t i = 0; i < this->elf_header->e_shnum; i++) {
        size_t offset = this->elf_header->e_shoff + (i * this->elf_header->e_shentsize);
        Elf64_Shdr *section_header = (Elf64_Shdr *) (elf_data + offset);   

        switch (section_header->sh_type)
        {
            case SHT_DYNSYM:
                this->symbol_table = elf_data + section_header->sh_offset;
                this->symbol_table_size = section_header->sh_size;
                break;
            
            case SHT_STRTAB:
                // printf("[ElfReader.cpp]::init(char*)\tSHT_STRTAB\n");
                // this->string_tables.push_back(section_header);
                // break;
            case SHT_SYMTAB:
                // printf("[ElfReader.cpp]::init(char*)\tSHT_SYMTAB\n");
                if (this->string_tables.size() == 0) {
                    this->string_tables.push_back(section_header);
                }
                break;

            default:
                break;
        }     
    }
}

ElfReader::~ElfReader()
{
}

std::vector<Elf64_Sym *>* ElfReader::getSymbols()
{
    // std::vector<Elf64_Sym *> *vector = new std::vector<Elf64_Sym *>();

    // for (uint16_t i = 0; i*sizeof(Elf64_Sym) < this->symbol_table_size; i++) {
    //     vector->push_back((Elf64_Sym *) this->symbol_table + (i*sizeof(Elf64_Sym)));
    // }

    // return vector;

    std::vector<Elf64_Sym *> *vector = new std::vector<Elf64_Sym *>();

    for (uint16_t i = 0; i*sizeof(Elf64_Sym) < symbol_table_size; i++) {
        size_t offset = i * sizeof(Elf64_Sym);
        vector->push_back((Elf64_Sym *) (symbol_table + offset));
    }

    return vector;
}

void ElfReader::printSectionHeader() {
    std::cout << std::left 
        << std::setw(5)  << std::setfill(' ') << ""
        << std::setw(20) << std::setfill(' ') << "Type"
        << std::setw(10) << std::setfill(' ') << "Flags"
        << std::setw(12) << std::setfill(' ') << "Address"
        << std::setw(12) << std::setfill(' ') << "Offset"
        << std::setw(12) << std::setfill(' ') << "Size"
        // << std::setw(10) << std::setfill(' ') << "Link"
        // << std::setw(10) << std::setfill(' ') << "Info"
        << std::setw(7) << std::setfill(' ') << "Align"
        << std::setw(10) << std::setfill(' ') << "entsize"
        << "\n";

    for (uint16_t i = 0; i < this->elf_header->e_shnum; i++) {
        std::cout << std::setw(5) << std::setfill(' ') << std::dec << i;
        size_t offset = this->elf_header->e_shoff + (i * this->elf_header->e_shentsize);
        Elf64_Shdr *section_header = (Elf64_Shdr *) (elf_data + offset);   

        std::cout << std::setw(20) << std::setfill(' ');
        switch (section_header->sh_type)
        {
            case SHT_NULL: std::cout << "SHT_NULL"; break;
            case SHT_PROGBITS: std::cout << "SHT_PROGBITS"; break;
            case SHT_SYMTAB: std::cout << "SHT_SYMTAB"; break;
            case SHT_STRTAB: std::cout << "SHT_STRTAB"; break;
            case SHT_RELA: std::cout << "SHT_RELA"; break;
            case SHT_HASH: std::cout << "SHT_HASH"; break;
            case SHT_DYNAMIC: std::cout << "SHT_DYNAMIC"; break;
            case SHT_NOTE: std::cout << "SHT_NOTE"; break;
            case SHT_NOBITS: std::cout << "SHT_NOBITS"; break;
            case SHT_REL: std::cout << "SHT_REL"; break;
            case SHT_SHLIB: std::cout << "SHT_SHLIB"; break;
            case SHT_DYNSYM: std::cout << "SHT_DYNSYM"; break;
            case SHT_GNU_ATTRIBUTES: std::cout << "SHT_GNU_ATTRIBUTES"; break;
            case SHT_GNU_HASH: std::cout << "SHT_GNU_HASH"; break;
            case SHT_GNU_LIBLIST: std::cout << "SHT_GNU_LIBLIST"; break;
            case SHT_CHECKSUM: std::cout << "SHT_CHECKSUM"; break;
            case SHT_GNU_verdef: std::cout << "SHT_GNU_verdef"; break;
            case SHT_GNU_verneed: std::cout << "SHT_GNU_verneed"; break;
            case SHT_GNU_versym: std::cout << "SHT_GNU_versym"; break;
            default:
                std::cout << section_header->sh_type;
                break;
        }
        std::cout << std::setw(10) << std::setfill(' ');
        char flags[64] = {0, };
        if (section_header->sh_flags & SHF_WRITE) sprintf(flags, "w,");
        if (section_header->sh_flags & SHF_ALLOC) sprintf(flags, "%sa,", flags);
        if (section_header->sh_flags & SHF_EXECINSTR) sprintf(flags, "%se,", flags);
        if (section_header->sh_flags & SHF_MASKPROC) sprintf(flags, "%sproc", flags);
        std::cout << flags;
        std::cout << "0x" << std::setw(10) << std::setfill(' ') << std::hex << section_header->sh_addr;
        std::cout << "0x" << std::setw(10) << std::setfill(' ') << std::hex << section_header->sh_offset;
        std::cout << "0x" << std::setw(10) << std::setfill(' ') << std::hex << section_header->sh_size;
        std::cout << std::setw(7) << std::setfill(' ') << std::dec << section_header->sh_addralign;
        std::cout << "0x" << std::setw(8) << std::setfill(' ') << std::hex << section_header->sh_entsize;
        std::cout << "\n";
    }
}

char* ElfReader::nameForSymbol(Elf64_Sym *symbol) {
    if (symbol->st_name != 0) {
        char *name = nullptr;
        for (uint8_t j = 0; j < this->string_tables.size(); j++) {
            void *table_start = this->elf_data + this->string_tables.at(j)->sh_offset;
            size_t table_size = this->string_tables.at(j)->sh_size;

            if (symbol->st_name < table_size) {
                name = (char *) table_start + symbol->st_name;
                break;
            }
        }
        if (name != nullptr) {
            // std::cout << name;
            char *demangled;
            int status;
            demangled = abi::__cxa_demangle(name, 0, 0, &status);
            // std::cout << status;
            if (status == 0) {
                return demangled;
            } else {
                char *mallocd = (char *) malloc(strlen(name) + 4);
                strcpy(mallocd, name);
                return mallocd;
            }
        }
    }
    return nullptr;
}

void ElfReader::printSymbolTable() {
    std::vector<Elf64_Sym *> * symbols = this->getSymbols();
    printf("[ElfReader.cpp]::printSymbolTable()\tFound %d symbols\n", symbols->size());
    std::cout << std::left 
        << std::setw(5)  << std::setfill(' ') << ""
        << std::setw(10) << std::setfill(' ') << "SH Index"
        << std::setw(20) << std::setfill(' ') << "value"
        << std::setw(20) << std::setfill(' ') << "size"
        << std::setw(15) << std::setfill(' ') << "info"
        << std::setw(15) << std::setfill(' ') << "other"
        << std::setw(10) << std::setfill(' ') << "name"
        << "\n";

    for (uint16_t i = 0; i < symbols->size(); i++) {
        Elf64_Sym *symbol = symbols->at(i);

        std::cout << std::setw(5) << std::setfill(' ') << std::dec << i;
        std::cout << std::setw(10) << std::setfill(' ') << std::dec << symbol->st_shndx;
        std::cout << "0x" << std::setw(18) << std::setfill(' ') << std::hex << symbol->st_value;
        std::cout << "0x" << std::setw(18) << std::setfill(' ') << std::hex << symbol->st_size;
        std::cout << std::setw(15) << std::setfill(' ');
        switch (ELF64_ST_TYPE(symbol->st_info))
        {
        case STT_NOTYPE: std::cout << "STT_NOTYPE"; break;
        case STT_OBJECT: std::cout << "STT_OBJECT"; break;
        case STT_FUNC: std::cout << "STT_FUNC"; break;
        case STT_SECTION: std::cout << "STT_SECTION"; break;
        case STT_FILE: std::cout << "STT_FILE"; break;
        case STT_COMMON: std::cout << "STT_COMMON"; break;
        case STT_TLS: std::cout << "STT_TLS"; break;
        case STT_NUM: std::cout << "STT_NUM"; break;
        case STT_LOOS: std::cout << "STT_LOOS"; break;
        case STT_HIOS: std::cout << "STT_HIOS"; break;
        case STT_LOPROC: std::cout << "STT_LOPROC"; break;
        case STT_HIPROC: std::cout << "STT_HIPROC"; break;
        default:
            std::cout << std::dec << ELF64_ST_TYPE(symbol->st_info);
        }
        std::cout << std::setw(15) << std::setfill(' ');
        switch(ELF64_ST_VISIBILITY(symbol->st_other))
        {
            case STV_DEFAULT: std::cout << "STV_DEFAULT"; break;
            case STV_INTERNAL: std::cout << "STV_INTERNAL"; break;
            case STV_HIDDEN: std::cout << "STV_HIDDEN"; break;
            case STV_PROTECTED: std::cout << "STV_PROTECTED"; break;
        }
        char *name = this->nameForSymbol(symbol);
        if (name != nullptr) {
            std::cout << name;
            free(name);
        }

        std::cout << "\n";
    }
}

std::vector<Elf64_Sym *>* ElfReader::lookupSymbol(char *lookup)
{
    std::vector<Elf64_Sym *>* finds = new std::vector<Elf64_Sym *>();
    std::vector<Elf64_Sym *>* symbols = this->getSymbols();
    for (uint16_t i = 0; i < symbols->size(); i++) {
        Elf64_Sym *symbol = symbols->at(i);
        char *name = this->nameForSymbol(symbol);
        if (name == nullptr) {
            continue;
        }
        if (strstr(name, lookup) != NULL) {
            finds->push_back(symbol);
        }
    }
    return finds;
}