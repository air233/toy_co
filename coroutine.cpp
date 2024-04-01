#include "coroutine.h"
#include <iostream>
#include <string.h>
#include <vector>

static std::vector<coCoroutine*> globalCo;

static void CoroutineFunc(coCoroutine* co, void* arg)
{
    if(co)
    {
        co->func(arg);
    }

    //让出
    co_yield();
}

void _co_contex_init(coCoroutine* co)
{
    if(co)
    {
        memset(co->coCtx.regs,0, sizeof(co->coCtx.regs));

        char* sp = (char*) co->stack_buff + co->stack_size - sizeof(void*);
        //字节对齐
        sp = (char*)((unsigned long)sp & -16LL);

        //eip地址设置为当前地址
        void** ret_addr = (void**)sp;
        *ret_addr = (void*)CoroutineFunc;
        //寄存器
        co->coCtx.regs[RSP] = sp;
        co->coCtx.regs[RET] = (void*)CoroutineFunc;
        //参数
        co->coCtx.regs[RDI] = (char*)co;
        co->coCtx.regs[RSI] = (char*)co->arg;
        co->ctxStart = 1;
    }
}

void _co_main_init()
{
    //构造一个mainCo mainCo只用来保存当前寄存器变量
    coCoroutine* mainCo = (coCoroutine*) malloc(sizeof(coCoroutine));
    memset(mainCo,0,sizeof(coCoroutine));
    memcpy(mainCo->coname,"mainco",6); 
    mainCo->func = NULL;
    mainCo->arg = NULL;
    mainCo->stack_size = 1024 * 1024 * 2;
    mainCo->stack_buff = malloc(mainCo->stack_size);
    memset(mainCo->stack_buff,0,mainCo->stack_size);
    _co_contex_init(mainCo);//保存当前寄存器的值
    globalCo.emplace_back(mainCo);
}

void _co_swap(coCoroutine* coCur,coCoroutine* coPending)
{
    if(coCur && coPending)
    {
        //切换进程
        co_contex_swap(&coCur->coCtx, &coPending->coCtx);
    }
}

int co_create(coCoroutine** pCo,void *(*routine)(void*),void *arg)
{
    if(globalCo.size() == 0)
    {
        _co_main_init();
    }

    coCoroutine* co = (coCoroutine*) malloc(sizeof(coCoroutine));
    memset(co,0,sizeof(coCoroutine));

    //函数
    co->func = routine;
    co->arg = arg;

    //内存
    co->stack_size = 1024 * 1024 * 2;
    co->stack_buff = malloc(co->stack_size);
    memset(co->stack_buff,0,co->stack_size);

    //ctx
    co->ctxStart = 0;

    *pCo = co;
    return 0;
}

void co_resume(coCoroutine* co)
{
    if(co)
    {
        if(co->ctxStart == 0)
        {
            _co_contex_init(co);

            co->ctxStart = 1;
        }
        
        coCoroutine* currCo = globalCo.back();
        globalCo.push_back(co);
        
        //跳转到co
        //std::cout << "cur:" << currCo->coname << std::endl;
        //std::cout << "peding:" << co->coname << std::endl;
        _co_swap(currCo,co);
    }
}

void co_yield()
{
    coCoroutine* currCo = globalCo.back();
    coCoroutine* lastCo = globalCo[globalCo.size()-2];
    globalCo.pop_back();

    //跳转lastCo
    _co_swap(currCo,lastCo);
}

void co_yield(coCoroutine* co)
{
    //TODO:
}

void co_release(coCoroutine* co)
{
    if(co && co->stack_buff)
    {
        free(co->stack_buff);
    }
}
