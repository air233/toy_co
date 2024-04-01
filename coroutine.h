
#define RDI 7
#define RSI 8
#define RET 9
#define RSP 13

struct coContex
{
    void* regs[14];
    // 64 bit
//low | regs[0]: r15 |
//    | regs[1]: r14 |
//    | regs[2]: r13 |
//    | regs[3]: r12 |
//    | regs[4]: r9  |
//    | regs[5]: r8  |
//    | regs[6]: rbp |
//    | regs[7]: rdi |
//    | regs[8]: rsi |
//    | regs[9]: ret |  //ret func addr
//    | regs[10]: rdx |
//    | regs[11]: rcx |
//    | regs[12]: rbx |
//hig | regs[13]: rsp |
    char* stack_sp;
    int   size_stack;
};

extern "C"
{
	extern void co_contex_swap(coContex* a, coContex* b) asm("co_swap");
};

typedef void *(*pfCoWorkFunction)(void* arg);


struct coCoroutine
{
    coContex coCtx;
    pfCoWorkFunction func;
    void* arg;
    void* stack_buff;
    int stack_size;

    char ctxStart;

    char coname[64];
};

int co_create(coCoroutine** pCo,void *(*routine)(void*),void *arg);
void co_resume(coCoroutine* co);
void co_yield();
void co_yield(coCoroutine* co);
void co_release(coCoroutine* co);
void co_loop();

void _co_swap(coCoroutine* coCur,coCoroutine* coLast);
