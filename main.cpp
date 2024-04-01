#include<iostream>
#include<stdio.h>
#include<string.h>
#include"coroutine.h"

using namespace std;

void* cofun(void* arg)
{
    int a = *(int*)arg;

    cout << "cofun - 1:" << a << endl;
    co_yield();
    cout << "cofun - 2:" << a << endl;
}

int main()
{
    coCoroutine *co1,*co2;
    int a = 10;
    int b = 20;
    co_create(&co1,cofun,&a);
    cout << "co_create-co1" << endl;
    co_create(&co2,cofun,&b);
     cout << "co_create-co2" << endl;

    co_resume(co1);
    cout << "co_resume co1 - 1" << endl;
    co_resume(co2);
    cout << "co_resume co2 - 1" << endl;

    co_resume(co1);
    cout << "co_resume co1 - 2" << endl;
    co_resume(co2);
    cout << "co_resume co2 - 2" << endl;
    return 0;
}