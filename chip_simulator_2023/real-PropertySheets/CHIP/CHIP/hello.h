//hello.h
#ifndef  _HELLO_H
#define _HELLO_H
#include "systemc.h"

SC_MODULE(hello)
{
    SC_CTOR(hello)
    {
        cout << "Genshin, start! " << endl;
    }
};
#endif