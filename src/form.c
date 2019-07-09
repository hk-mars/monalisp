

#include "form.h"

#include "debug.h"

#include "mem.h"

#include "util.h"

#include "error.h"



form_s*
form_create(void)
{
    return ml_malloc(sizeof(form_s));
}


void
form_free(form_s *form)
{
    ml_free(form);
}


form_s*
form_create_list(void)
{
    form_s *f;

    f = form_create();
    if (!f) return NULL;

    f->list = ml_malloc(sizeof(lisp_list_s));
    if (!f->list) return NULL;

    f->list->obj.type = OBJ_LIST;
    
    return f;
}


form_s*
form_create_symbol(void)
{
    form_s *f;

    f = form_create();
    if (!f) return NULL;

    f->symbol = ml_malloc(sizeof(symbol_s));
    if (!f->symbol) return NULL;

    return f;
}


bool
form_add_front(form_s *forms, form_s *new_form)
{
    if (!forms || !new_form) return false;

    if (!forms->front) {

	forms->next = new_form;
	forms->front = new_form; 
	new_form->front = forms;
	new_form->next = forms;
    }
    else {
	
	forms->front->next = new_form;
	new_form->front = forms->front;
	new_form->next = forms;
	forms->front = new_form;
    }

    func_ok();
    return true;
}


void
form_set_type(form_s *form, form_t type)
{
    debug("%s, type: %d \n", __func__, type);
    
    form->type = type;
}


bool
form_is_unkown(form_s *form)
{
    return (form->type == UNKOWN_FORM);
}


void
form_show(form_s *form)
{
    if (!form) return;

    func_s();

    form_s *f = form->next;
    
    while (f != form) {

	switch (f->type) {

	case SYMBOL_FORM:
	    debug_err("SYMBOL_FORM \n");
	    
	    break;

	case COMPOUND_FUNCTION_FORM:
	    debug_err("COMPOUND_FUNCTION_FORM \n");
	    
	    break;

	    
	default:
	    debug_err("unkown form \n");
	    break;

	}
	
	
	f = f->next;
    }

    func_ok();
}
