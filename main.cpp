#include <stdio.h>
#include <iostream>
#include <iomanip>

#include "Reflection.h"
#include "ReflectiveClass.h"

ReflectiveClass ref_a;

char *repl_prompt = ">>>";

void test_function() {
    printf("Test Function!\n");
}

void test_echo(char *str) {
    printf("Called test_echo! str = %p\n", str);
    printf(str);
}

void four_args(int a, char *b, int c, int d) {
    printf("[main.cpp]::four_args(%d,%s,%d,%d)\n", a, b, c, d);
}

int main(int argc, char **argv) {
    printf("[main.cpp]\tIt runs!\n");

    Reflection *r = Reflection::FromCurrentProcess((void *)main);

    printf("[main]\t&ReflectiveClass::increment = %p\n", &ReflectiveClass::increment);
    printf("[main]\t&ReflectiveClass::help = %p\n", &ReflectiveClass::help);
    printf("[main]\ttest_function = %p\n", test_function);
    printf("[main]\tref_a = %p\n", &ref_a);

    std::string line;
    while (true) {
        std::cout << repl_prompt;
        std::getline(std::cin, line, '\n');
        if (line.length() == 0) continue;
        r->exec(line, nullptr);
    }
}
