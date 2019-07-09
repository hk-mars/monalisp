

#ifndef ML_LIST_H
#define ML_LIST_H


#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

#include "obj.h"

#include "token.h"


typedef struct s_lisp_list
{
    object_s obj;

    struct s_lisp_list *next;
    struct s_lisp_list *front;
    
} lisp_list_s;


bool list_add_token(lisp_list_s *list, token_s *token);

void list_show(lisp_list_s *list);


#endif /* ML_LIST_H */
