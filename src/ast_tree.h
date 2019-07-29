


#ifndef _AST_TREE_H_
#define _AST_TREE_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "config.h"


#include "tree.h"
#include "hsearch.h"


typedef struct S_TOKEN
{
  char *key;
  char *value;
} token;


typedef struct S_TOKEN_LIST
{
  token tk;
  struct S_TOKEN_LIST *next;
} token_list;


int dbg_lex(void);
int is_like_keyword(char *s);
void set_lex_tree(tr_node_s *root);
tr_node_s* get_lex_tree(void);
void set_kw_htab(hash_table_s *htab);
int is_token(char *key);
int check_lex(char *sql_str, token_list *tk_lst) ;
void free_tokens(token_list *tk_lst);


#endif /* _AST_TREE_H_ */
