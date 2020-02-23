#ifndef __REFLECTIVE_CLASS_H__
#define __REFLECTIVE_CLASS_H__

#include <stdio.h>

class ReflectiveClass
{
private:
    int count = 0;

public:
    ReflectiveClass();
    ~ReflectiveClass();

    void increment();
    int get_counts();
    void set_count(int _count);
    void print_count();
    void help();
    void f_args(char *a, int b, int c, int d);
};


#endif // __REFLECTIVE_CLASS_H__