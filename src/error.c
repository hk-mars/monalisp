

#include "error.h"

#include "debug.h"


void
ml_err_proc_mem_full(void)
{
    debug_err("err: oops, heap memory is full");

    
    /* TODO: process for out of memory */
    while (1);
}


static void
proc_illegal_char(void)
{
    func_s();

    /* TODO: process for illegal lex char */
    while (1);
}


static void
proc_stack_overflow(void)
{
    func_s();
    
    /* TODO: process for stack overflow */
    while (1);
}


static void
proc_buf_overflow(void)
{
    func_s();
    
    /* TODO: process for buffer overflow */
    while (1);
}


static void
proc_divide_zero(void)
{
    func_s();
    
    /* TODO: process for deviding zero */
    while (1);
}


void
ml_err_signal(ml_err_t err)
{
    debug("%s, err:%d \n", __func__, err);

    switch (err) {
    case ML_ERR_ILLEGAL_CHAR:

	proc_illegal_char();
	break;

    case ML_ERR_MEM_FULL:

	ml_err_proc_mem_full();
	break;

    case ML_ERR_STACK_OVERFLOW:

	proc_stack_overflow();
	break;


    case ML_ERR_BUF_OVERFLOW:

	proc_buf_overflow();
	break;

    case ML_ERR_EVAL_DIVIDE_ZERO:

	proc_divide_zero();
	break;

    default:
	debug_err("unkown error \n");
	break;
    }

}