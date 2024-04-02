#include "coroutine.h"
#include <iostream>
#include <string.h>
#include <vector>

coCoroutineEvn* co_get_evn();
void _co_contex_init(coCoroutine* co);

static coCoroutineEvn globalCoEvn;

bool coCoroutineEvn::IsInit()
{
    return m_init == 1;
}

bool coCoroutineEvn::Init()
{
    //构造一个mainCo mainCo只用来保存当前寄存器变量
    coCoroutine* mainCo = (coCoroutine*) malloc(sizeof(coCoroutine));
    memset(mainCo,0,sizeof(coCoroutine));
    memcpy(mainCo->m_coname,"mainco",6); 
    mainCo->m_func = NULL;
    mainCo->m_arg = NULL;
    mainCo->m_stack_size = 1024 * 1024 * 2;
    mainCo->m_stack_buff = malloc(mainCo->m_stack_size);
    memset(mainCo->m_stack_buff,0,mainCo->m_stack_size);
    _co_contex_init(mainCo);//保存当前寄存器的值

    m_gloablCo.emplace_back(mainCo);
    m_init = 1;
    return true;    
}

//最后一个co始终为当前协程
coCoroutine* coCoroutineEvn::GetCurCo()
{
    if(false == IsInit())
    {
        return nullptr;
    }

    if(m_gloablCo.size() == 0)
    {
        return nullptr;
    }

    return m_gloablCo.back();
}

//前一个co
 coCoroutine* coCoroutineEvn::GetLastCo()
 {
    if(false == IsInit())
    {
        return nullptr;
    }

    if(m_gloablCo.size() < 1)
    {
        return nullptr;
    }

    return m_gloablCo[m_gloablCo.size()-2];
 }

void coCoroutineEvn::PushCo(coCoroutine* co)
{
    if(false == globalCoEvn.IsInit())
    {
        globalCoEvn.Init();
    }

    m_gloablCo.emplace_back(co);
}

void coCoroutineEvn::PopCurCo()
{
    if(m_gloablCo.size() > 1)
    {
        m_gloablCo.pop_back();
    }
}



//==============================================


static void CoroutineFunc(coCoroutine* co)
{
    if(co)
    {
        co->m_func(co->m_arg);
    }

    co_yield();
}

void _co_contex_init(coCoroutine* co)
{
    if(co)
    {
        memset(co->m_coCtx.regs,0, sizeof(co->m_coCtx.regs));

        char* sp = (char*) co->m_stack_buff + co->m_stack_size - sizeof(void*);
        //字节对齐
        sp = (char*)((unsigned long)sp & -16LL);

        //eip地址设置为CoroutineFunc 初次执行时需要设置
        void** ret_addr = (void**)sp;
        *ret_addr = (void*)CoroutineFunc;

        //寄存器
        co->m_coCtx.regs[RSP] = sp;
        co->m_coCtx.regs[RET] = (void*)CoroutineFunc;

        //参数
        co->m_coCtx.regs[RDI] = (char*)co;
        //co->coCtx.regs[RSI] = (char*)co->arg;
        co->m_ctxStart = 1;
    }
}

void _co_swap(coCoroutine* coCur,coCoroutine* coPending)
{
    if(coCur && coPending)
    {
        //切换进程
        co_contex_swap(&coCur->m_coCtx, &coPending->m_coCtx);
    }
}

int co_create(coCoroutine** pCo, void *(*routine)(void*), void *arg)
{
    coCoroutineEvn* coGlobalEvn = co_get_evn();

    coCoroutine* co = (coCoroutine*) malloc(sizeof(coCoroutine));
    memset(co,0,sizeof(coCoroutine));

    //evn
    co->m_evn = coGlobalEvn;

    //函数
    co->m_func = routine;
    co->m_arg = arg;

    //内存
    co->m_stack_size = 1024 * 1024 * 2;
    co->m_stack_buff = malloc(co->m_stack_size);
    memset(co->m_stack_buff,0,co->m_stack_size);

    //ctx
    co->m_ctxStart = 0;

    *pCo = co;
    return 0;
}

void co_resume(coCoroutine* co)
{
    if(co)
    {
        if(co->m_ctxStart == 0)
        {
            _co_contex_init(co);

            co->m_ctxStart = 1;
        }
        
        coCoroutine* currCo = co->m_evn->GetCurCo();
        co->m_evn->PushCo(co);
        _co_swap(currCo,co);
    }
}

void co_yield()
{
    coCoroutineEvn* coEvn = co_get_evn();

    if(coEvn)
    {
        coCoroutine* currCo = coEvn->GetCurCo();
        coCoroutine* lastCo = coEvn->GetLastCo();
        coEvn->PopCurCo();
            
        if(currCo && lastCo)
        {
            //跳转lastCo
            _co_swap(currCo,lastCo);
        }
    }
}

void co_yield(coCoroutine* co)
{
    (void)co;

    co_yield();
}

void co_release(coCoroutine* co)
{
    if(co && co->m_stack_buff)
    {
        free(co->m_stack_buff);
    }
}

coCoroutineEvn* co_get_evn()
{
    if(false == globalCoEvn.IsInit())
    {
        globalCoEvn.Init();
    }

    return &globalCoEvn;
}
