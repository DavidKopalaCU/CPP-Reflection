#include "ReflectiveClass.h"

ReflectiveClass::ReflectiveClass()
{
}

ReflectiveClass::~ReflectiveClass()
{
}

void ReflectiveClass::increment()
{
    printf("[ReflectiveClass]::increment\tthis = %p\n", this);
    this->count += 1;
}

int ReflectiveClass::get_counts()
{
    return this->count;
}

void ReflectiveClass::set_count(int _count)
{
    this->count = _count;
}

void ReflectiveClass::print_count()
{
    printf("[ReflectiveClass]::print_count()\tCount: %d\n", this->count);
}

void ReflectiveClass::help()
{
    printf("THIS SHOULD WORK\n");
}

void ReflectiveClass::f_args(char *a, int b, int c, int d)
{
    printf("[ReflectiveClass]::f_args(%s,%d,%d,%d)\n", a, b, c, d);
}