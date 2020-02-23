#include "Reflection.h"
#include "ReflectiveClass.h"

Reflection::Reflection(ElfReader *_elf, void *_main)
{
    this->elf = _elf;
    std::vector<Elf64_Sym *> *mains = this->elf->lookupSymbol("main");
    printf("[Reflection]::init\tmains.size() = %d\n", mains->size());
    // Elf64_Sym *main = mains->at(1);
    Elf64_Sym *main = this->promptForSymbolSelection(mains);
    this->base_address = (_main - main->st_value);
    free(mains);
}

Reflection::~Reflection()
{
}

Reflection* Reflection::FromCurrentProcess(void *_main)
{
    /**
     * Instantiates and returns a Reflection object for the current process
     * Linux maintains a reference to the process's executable ELF file in
     * /proc/<pid>/exec
     * This function uses that file path to load the process's ELF file
     */

    pid_t process_id = getpid();
    char filepath[64];
    sprintf(filepath, "/proc/%d/exe", process_id);

    printf("[Reflection.cpp]::FromCurrentProcess\tLoading ELF file from %s\n", filepath);

    ElfReader *elf = new ElfReader(filepath);
    return new Reflection(elf, _main);
}

typedef struct {
    void *data;
    bool parens;
} arg_t;

int Reflection::exec(std::string command, void *ret)
{
    // std::regex r("\\b([a-zA-Z_.]+)\\((.*)\\)$");
    printf("[Reflection]::exec(std::string, void*)\tTrying: %s\n", command.c_str());
    
    void *base_object = nullptr;
    const char *c_command = command.c_str();
    const char *period;

    char *func_name = (char *) malloc(32);
    memset(func_name, 0, 32);
    strcpy(func_name, command.c_str());

    char *args = (char *) malloc(32);
    memset(args, 0, 32);

    std::regex re("\\b([a-zA-Z_.]+)\\((.*)\\)$"); 
    std::smatch match;
    if (std::regex_search(command, match, re) == true) { 
        strcpy(args, match.str(2).c_str());
        printf("[Reflection]::exec\tArgs: '%s'\n", args);
        strcpy(func_name, match.str(1).c_str());
        printf("[Reflection]::exec\tFunc Name: '%s'\n", func_name);
    }

    if ((period = strstr(func_name, ".")) != NULL) {
        char *base_name = (char *) malloc((period - func_name) + 2);
        memset(base_name, 0, (period - func_name) + 2);
        strncpy(base_name, func_name, (period - func_name));
        printf("[Reflection]::exec\tLooking for base object: %s\n", base_name);

        std::vector<Elf64_Sym *> *base_symbols = this->elf->lookupSymbol(base_name);
        if (base_symbols->size() == 0) {
            printf("[Reflection]::exec\tCouldn't find base_symbol for: %s\n", base_name);
            free(base_name);
            free(base_symbols);
            return -2;
        }
        Elf64_Sym *base_symbol = base_symbols->at(0);
        char *name = this->elf->nameForSymbol(base_symbol);

        base_object = (this->base_address + base_symbol->st_value);
        size_t func_name_length = (strlen(func_name) - strlen(base_name));
        func_name = (char *) malloc(func_name_length + 2);
        memset(func_name, 0, func_name_length + 2);
        strncpy(func_name, period + 1, func_name_length);

        printf("[Reflection]::exec\tUsing %s as base @ %p\n", name, base_object);
        free(name);
        free(base_symbols);
        free(base_name);
    }

    std::vector<Elf64_Sym *> *matches = this->elf->lookupSymbol((char *) func_name);
    if (matches->size() == 0) {
        printf("[Reflection]::exec\tCouldn't find a symbol for: %s\n", func_name);
        free(matches);
        return -1;
    }
    Elf64_Sym *symbol = this->promptForSymbolSelection(matches);
    printf("Found Symbol! %s, 0x%X, 0x%X\n", this->elf->nameForSymbol(symbol), symbol->st_value, this->base_address + symbol->st_value);
    
    
    std::vector<std::string> a;
    if (strlen(args) > 0) {
        std::stringstream ss(args);
        while (ss.good()) {
            std::string arg;
            getline(ss, arg, ',');
            a.push_back(arg);
        }
    }

    int argc = a.size();
    arg_t* data_args[argc];
    for (uint8_t i = 0; i < a.size(); i++) {
        std::string arg_str = a.at(i);
        if (arg_str.find('"') != std::string::npos) {
            const char *arg_char = arg_str.c_str();
            char *data = (char *) malloc(strlen(arg_char));
            memset(data, 0, strlen(arg_char));
            strncpy(data, arg_char + 1, strlen(arg_char) - 2);
            arg_t *arg = (arg_t *) malloc(sizeof(arg_t));
            arg->data = data;
            arg->parens = false;
            data_args[i] = arg;
        } else {
            int arg_int = atoi(arg_str.c_str());
            int *data = (int *) malloc(sizeof(int));
            *data = arg_int;
            arg_t *arg = (arg_t *) malloc(sizeof(arg_t));
            arg->data = data;
            arg->parens = true;
            data_args[i] = arg;
        }
    }

    void (*func)(void) = (void (*)()) this->base_address + symbol->st_value;
    uint8_t switch_offset = (base_object == nullptr) ? 0 : 1;
    for (uint8_t i = 0; i < argc; i++) {
        switch(i + switch_offset) {
            case 0:
                if (data_args[i]->parens == true) asm("mov (%0),%%rdi" : : "r" (data_args[i]->data) : "%rdi");
                if (data_args[i]->parens == false) asm("mov %0,%%rdi" : : "r" (data_args[i]->data) : "%rdi");
                break;
            case 1:
                if (data_args[i]->parens == true) asm("mov (%0),%%rsi" : : "r" (data_args[i]->data) : "%rsi");
                if (data_args[i]->parens == false) asm("mov %0,%%rsi" : : "r" (data_args[i]->data) : "%rsi");
                break;
            case 2:
                if (data_args[i]->parens == true) asm("mov (%0),%%rdx" : : "r" (data_args[i]->data) : "%rdx");
                if (data_args[i]->parens == false) asm("mov %0,%%rdx" : : "r" (data_args[i]->data) : "%rdx");
                break;
            case 3:
                if (data_args[i]->parens == true) asm("mov (%0),%%rcx" : : "r" (data_args[i]->data) : "%rcx");
                if (data_args[i]->parens == false) asm("mov %0,%%rcx" : : "r" (data_args[i]->data) : "%rcx");
                break;
            case 4:
                if (data_args[i]->parens == true) asm("mov (%0),%%r8" : : "r" (data_args[i]->data) : "%r8");
                if (data_args[i]->parens == false) asm("mov %0,%%r8" : : "r" (data_args[i]->data) : "%r8");
                break;
            case 5:
                if (data_args[i]->parens == true) asm("mov (%0),%%r9" : : "r" (data_args[i]->data) : "%r9");
                if (data_args[i]->parens == false) asm("mov %0,%%r9" : : "r" (data_args[i]->data) : "%r9");
                break;
            default:
                break;
        }
    }
    if (base_object != nullptr) {
        asm("mov %0,%%rdi;" : : "r" (base_object) : "%rdi");
    }
    asm("call %0;": : "r" (func));

    for (uint8_t i = 0; i < argc; i++) {
        free(data_args[i]->data);
        free(data_args[i]);
    }
}

Elf64_Sym* Reflection::promptForSymbolSelection(std::vector<Elf64_Sym *> *symbols) {
    if (symbols->size() == 1) {
        return symbols->at(0);
    }
    printf("Found %d matching symbols. Please select the one you want to use: \n");
    for (uint8_t i = 0; i < symbols->size(); i++) {
        char *name = this->elf->nameForSymbol(symbols->at(i));
        printf("\t(%d) - %s\n", i, name);
        free(name);
    }
    printf("Enter choice: ");
    int index;
    std::cin >> index;
    while (index > symbols->size()) {
        printf("(%d) is out of range! Enter a new value: ");
        std::cin >> index;
    }
    return symbols->at(index);
}