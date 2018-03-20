#include "stack.h"
int StackInit(ifstack *pstack)
{
    pstack->m_inum = 0;
    return 1;
}

int StackPush(ifstack *pstack, int idata)
{
    if (pstack->m_inum==MAX_STACK_SLOT)
        return 0;							/*stack overflow*/
    pstack->m_data[pstack->m_inum++] = idata;
    return 1;
}

int StackPop(ifstack *pstack, int *pidata)
{
    if (pstack->m_inum==0)
        return 0;			/* It's Empty*/
    *pidata = pstack->m_data[--pstack->m_inum];
    return 1;
}

int StackEmpty(ifstack *pstack)
{
    if (pstack->m_inum >0)
        return 0;
    else
        return 1;
}


