#ifndef _STACK_H_
#define _STACK_H_

#define MAX_STACK_SLOT	256
typedef struct ifstack
{
    int m_inum;						/*num of elems in the stack*/
    int m_data[MAX_STACK_SLOT];	/*the data*/
    
}ifstack;

int StackInit(ifstack *pstack);

int StackPush(ifstack *pstack, int idata);

int StackPop(ifstack *pstack, int *pidata);

int StackEmpty(ifstack *pstack);

#endif