
#include "lex.h"

#include "debug.h"

#include "chars.h"

#include "error.h"

#include "util.h"

#include "eval.h"

#include "mem.h"

#include "config.h"

#include "stack.h"

#include "token.h"

#include "variable.h"



#define next_code(code, code_sz) \
    (code)++;			 \
    (code_sz)--;


#define move_code(code, code_sz, sz_moved)	\
    int i = sz_moved;				\
    while ((code_sz) > 0 && i > 0) {		\
	next_code(code, code_sz);		\
	i--;					\
    }


#define ignore_end_chars(code, code_sz)		\
    while (1) {					\
	if (eq(**code, NEWLINE) ||		\
	    eq(**code, LINEFEED) ||		\
	    **code == SPACE) {			\
	    next_code(*code, *code_sz);		\
	    if (*code_sz <= 0) break;		\
	    continue;				\
	}					\
	break;					\
    }


typedef bool (*identify_f) (const char **code, size_t *code_sz, form_s *form);


static code_s
read_list(const char *code, size_t code_sz, form_s *form_head, form_s *form);


/* When a double-quote is encountered, characters are read from the input stream
 * and accumulated until another double-quote is encountered. If a single escape
 * character is seen, the single escape character is discarded, the next character 
 * is accumulated, and accumulation continues.
 */
static bool
read_string(const char **code, size_t *code_sz, form_s *form)
{
    token_s token;
    
    bool single_escape_flag;
    
    func_s();

    const char *s;
    
    if (!eq(**code, '\"')) return false;
    s = *code;

    single_escape_flag = false;
    while (*code_sz > 0) {

	next_code(*code, *code_sz);
	
	debug("0x%02x %c \n", **code, **code);

	if (is_escape_char(**code)) {
	    
	    single_escape_flag = true;
	    continue;
	}
	
	if (!single_escape_flag && eq(**code, '\"')) {

	    next_code(*code, *code_sz);

	    memset(&token, 0, sizeof(token_s));
	    token.value.symbol = ml_util_buf2str(s, *code - s);
	    if (token.value.symbol) {

		debug("symbol: %s \n", token.value.symbol);
		token.type = TOKEN_SYMBOL;
	    }
	    
	    if (token.type != TOKEN_UNKOWN) {

		//token_s *t = token_clone(&token);

		/* add the symbol token into the list form */
		list_add_token(form->list, &token);	
	
		func_ok();
		return true;
	    }
	    

	    func_ok();
	    return true;
	}

	if (single_escape_flag) single_escape_flag = false;
    }

    ml_err_signal(ML_ERR_ILLEGAL_CHAR);
    
    return false;
}


/* A single-quote introduces an expression to be "quoted". Single-quote followed 
 * by an expression exp is treated by the Lisp reader as an abbreviation for and is 
 * parsed identically to the expression (quote exp) or 'exp.
 */
static bool
read_expression_with_single_quote(const char **code, size_t *code_sz)
{
    func_s();

    if (!eq(**code, '\'')) return false;
    next_code(*code, *code_sz);

    /* TODO: reading expression 
     */
    while (*code_sz > 0) {

	debug("0x%02x %c \n", **code, **code);
	
	if (eq(**code, LINEFEED)) {
	    
	    next_code(*code, *code_sz);
 
	    func_ok();
	    return true;
	}

	next_code(*code, *code_sz);
    }

    ml_err_signal(ML_ERR_ILLEGAL_CHAR);
    
    return false;
}


/**
 * Semicolon is used to write comments.     
 * The semicolon and all characters up to and including the next newline are ignored. 
 * Thus a comment can be put at the end of any line without affecting the reader.
 */
static bool
read_comments(const char **code, size_t *code_sz)
{
    func_s();
    
    if (!eq(**code, ';')) return false;
    next_code(*code, *code_sz);

    while (*code_sz > 0) {

	if (eq(**code, NEWLINE) || eq(**code, LINEFEED)) {
	    
	    next_code(*code, *code_sz);

	    ignore_end_chars(code, code_sz);

	    func_ok();
	    return true;
	}

	next_code(*code, *code_sz);
    }

    ml_err_signal(ML_ERR_ILLEGAL_CHAR);
    
    return false;
}


static bool
read_symbol(const char **code, size_t *code_sz, form_s *form)
{
    bool found;
    token_s token;
     
    const char *buf;
    
    func_s();


    ignore_end_chars(code, code_sz);
    
    switch (**code) {

    case ';':

	/* A semicolon introduces characters to be ignored, such as comments. 
	 */
	
	found = read_comments(code, code_sz);
	if (!found) goto ERR;
	return true;
	break;
	
    case '\"':

        /* The double-quote is used to begin and end a string. 
	 */

	found = read_string(code, code_sz, form);
	if (!found) goto ERR;
	return true;
	break;

    default:
	break;

    }


    /* read the non-string symbol
     */
    buf = *code;  
    while (*code_sz > 0) {

	debug("0x%02x %c \n", **code, **code);
	
	if (eq(**code, SPACE) || eq(**code, ')')) {

	    memset(&token, 0, sizeof(token_s));
	    token.value.symbol = ml_util_buf2str(buf, *code - buf);
	    if (token.value.symbol) {

		debug("symbol: %s \n", token.value.symbol);
		token.type = TOKEN_SYMBOL;
	    }
	  
			    
	    if (eq(**code, SPACE)) {
		next_code(*code, *code_sz);
	    }


	    if (token.type != TOKEN_UNKOWN) {

		//token_s *t = token_clone(&token);

		/* add the symbol token into the list form */
		list_add_token(form->list, &token);	
	
		func_ok();
		return true;
	    }
	    
	    return false;
	}

	next_code(*code, *code_sz);
    }

  ERR:
    ml_err_signal(ML_ERR_ILLEGAL_CHAR);
    
    return false;
}


static bool
read_template(const char **code, size_t *code_sz, form_s *form_head, form_s *form)
{
    bool found;

    
    func_s();

    if (**code == '`') {
	
	next_code(*code, *code_sz);

	/* Add a template form
	 */
	list_add_char_obj(form->list, "`");
    }
    else if (**code == ',') {

	next_code(*code, *code_sz);

	//list_add_char_obj(form->list, ",");
	
	object_s obj;
	memset(&obj, 0, sizeof(object_s));
	obj.type = OBJ_TYPE;
	obj.subtype = OBJ_SUBTYPE_MACRO_COMMNA;
	list_add_object(form->list, &obj);
	goto DONE;
	
    }
    else {

	return false;	
    }
    
 		
    switch (**code) {
    case '(':

	/* parse the list subform in the template form
	 */

	next_code(*code, *code_sz);

	form_s *f_head = form_create();
	form->list->front->obj.sub = (void*)f_head;
	    
	form_s *f = form_create();
	if (!f) goto ERR;
	
	list_add_char_obj(f->list, "(");
		
	code_s icode = read_list(*code, *code_sz, f_head, f);
	if (!icode.code) goto ERR;

	list_add_char_obj(f->list, ")");
	
	*code = icode.code;
	*code_sz = icode.code_sz;
	break;

    default:
	goto ERR;
	break;
    }
    

  DONE:
    out(ok, true);

  ERR:
    ml_err_signal(ML_ERR_ILLEGAL_CHAR);

    out(fail, false);
}



static bool
like_arithmetic_func(const char *code)
{
    switch (*code) {
    case '+':
    case '-':
    case '*':
    case '/':
    case '<':
    case '>':
    case '=':
    case '!':
	return true;

    defualt:
	break;
    }
    
    return false;
}


static bool
like_list_func(const char *code)
{
    if (eq(*code, 'l') || !eq(*code, 'L')) return true;

    return false;
}


static bool
like_other_func(const char *code)
{
    if (eq(*code, 'c') || !eq(*code, 'C')) return true;

    if (eq(*code, 'e') || !eq(*code, 'E')) return true;
    
    return false;
}



/* A token is a potential number if it satisfies all of the following requirements:
 * 
 * 1. The token consists entirely of digits, signs, ratio markers, decimal points
 *    (.), extension characters (^ or _), and number markers. 
 * 2. The token contains at least one digit(letters may be considered to be digits).
 * 3. The token begins with a digit, sign, decimal point, or extension character, 
 *    but not a package marker.
 * 4. The token does not end with a sign.
 * 
 */
static bool
like_num_token(const char *code, size_t code_sz)
{
    if (is_sign(*code)) goto FIND_ONE_DIGIT;

    if (is_digit(*code)) goto CHECK_MORE;
    
    if (eq(*code, '.')) goto FIND_ONE_DIGIT;

    if (eq(*code, '^')) goto FIND_ONE_DIGIT;
    if (eq(*code, '_')) goto FIND_ONE_DIGIT;

    
  FIND_ONE_DIGIT:

    next_code(code, code_sz);
    while (code_sz > 0) {

	if (eq(*code, SPACE) || eq(*code, ')')) return false;

	if (is_digit(*code)) goto CHECK_MORE;

        next_code(code, code_sz);
    }

    return false;

  CHECK_MORE:

    next_code(code, code_sz);
    while (code_sz > 0) {

	if (eq(*code, SPACE) || eq(*code, ')')) {

	    /* The token does not end with a sign. */
	    if (is_sign(*(code-1))) return false;

	    return true;
	}

	if (is_digit(*code) || is_sign(*code) ||
	    eq(*code, '^') || eq(*code, '_') ||
	    eq(*code, '.') || is_ratio_marker(*code) ||
	    is_exponent_maker(*code)) {

	    next_code(code, code_sz);

	    continue;
	}

	return false;
    }

    
    return false;
}


static int
identify_digits(const char **code, size_t *code_sz)
{
    int k = 0;

    func_s();
    char *c = (char*)*code;
    
    while (*code_sz > 0) {

	if (!is_digit(**code)) {

	    debug("\n");
	    if (k > 0) {
		debug("digits len: %d \n", k);

		unsigned int x = ml_util_arr2int(c, k);
		debug("%d \n", x);
	    }
	    else {

		ml_err_signal(ML_ERR_ILLEGAL_CHAR);
	    }
	    
	    return k;
	}

	debug("%c ", **code);
	
	k++;
	next_code(*code, *code_sz);
    }

    return k;
}


static bool
identify_number_token(const char **code, size_t *code_sz, form_s *form)
{
    bool is_negative = false;
    char *s = (char*)*code;

    token_s token;
    memset(&token, 0, sizeof(token_s));

    if (*code_sz <= 0) return false;
    
    if (is_sign(**code)) {

	is_negative = (**code == '-');
	next_code(*code, *code_sz);
    }
    
    char c;
    int k = 0;
    

    while (*code_sz > 0) {

	c = **code;
	if (!is_digit(c)) break;

	//debug("%c ", c);
	
	k++;
	next_code(*code, *code_sz);
    }
    //debug("\n");

    
    if (k > 0) {
	debug("digits len: %d \n", k + is_negative);

	/* [sign] {digit}+ decimal-point
	 */
	if (eq(c, '.')) {
	    next_code(*code, *code_sz);

	    if (is_digit(c)) {

		/* float */
		token.type = TOKEN_NUM_FLOAT;
	    }
	    else if (is_exponent_maker(c)) {

		/* float */
		token.type = TOKEN_NUM_FLOAT;
	    }
	    else {

		/* integer */
		token.type = TOKEN_NUM_INT;
	    }
	}
	else if (is_ratio_marker(c)) {  
	    next_code(*code, *code_sz);

	    if (is_digit(c)) {

		/* ratio */
		token.type = TOKEN_NUM_RATIO;
			
	    }
	    else {
			
		return false;
	    }
			
	}		
	else {

	    if (!is_whitespace_char(c) &&
		!eq(c, SPACE) &&
		!eq(c, ')')) {

		return false;
	    }
		
	    /* integer */
	    token.type = TOKEN_NUM_INT;
	    
	    token.value.num_int = ml_util_arr2int(s, k + is_negative);
	    debug("%d \n", token.value.num_int);
	}
	       		
    }
    else {

	/* float */
	token.type = TOKEN_NUM_FLOAT;
    }

	    
    if (token.type != TOKEN_UNKOWN) {

	//token_s *t = token_clone(&token);

	/* add the number token into the list form */
	list_add_token(form->list, &token);	
	
	func_ok();
	return true;
    }
    
    return false;
}


static bool
identify_arithmetic_operator(const char **code, size_t *code_sz, form_s *form)
{
    token_s *t, tk;
    char *s;
    int len;

    switch (**code) {

    case '+':
	s = "+";
	len = 1;
	break;
	
    case '-':
	s = "-";
	len = 1;
	break;
	
    case '*':
	s = "*";
	len = 1;
	break;
	
    case '/':
	if (*code_sz-1 == 0) return false;
	
	if (*(*code+1) == '=') {
	    s = "/=";
	    len = 2;
	}
	else {
	    return false;	    
	}
	break;
	
    case '<':
	if (*code_sz-1 == 0) return false;
	
	if (*(*code+1) == '=') {
	    s = "<=";
	    len = 2;
	}
	else {
	    s = "<";
	    len = 1;	    
	}
	break;
	
    case '>':
	if (*code_sz-1 == 0) return false;
	
	if (*(*code+1) == '=') {
	    s = ">=";
	    len = 2;
	}
	else {
	    s = ">";
	    len = 1;	    
	}
	break;

    case '!':
	if (*code_sz-1 == 0) return false;
	
	if (*(*code+1) == '=') {
	    s = "!=";
	    len = 2;
	}
	else {
	    return false;	    
	}
	break;
	
    defualt:
	return false;
	break;
    }


    move_code(*code, *code_sz, len);
    if (*code_sz == 0) goto DONE;

    if (eq(**code, SPACE)) {
	
	next_code(*code, *code_sz);
	goto DONE;
    }
    
    if (is_whitespace_char(**code)) {

	next_code(*code, *code_sz);
	goto DONE;
    }
 
    if (**code == ')') {
	
	next_code(*code, *code_sz);
	goto DONE;
    }    
     
    return false;
    
  DONE:
    
    memset(&tk, 0, sizeof(token_s));
    t = &tk;
    //t = token_create();
    //if (!t) return false;
    
    
    t->type = TOKEN_SYMBOL;
    t->value.symbol = s;

        
    if (form_is_unkown(form)) {
	form_set_type(form, COMPOUND_FUNCTION_FORM);
    }
    
    list_add_token(form->list, t);
  
    func_ok();
    return true;
}


static bool
identify_list_func(const char **code, size_t *code_sz, form_s *form)
{
    token_s *t, tk;
    
    if (!ml_util_strbufcmp("list", *code, *code_sz)) return false;
    
    move_code(*code, *code_sz, 4);
    
    if (*code_sz == 0) goto DONE;

    if (eq(**code, SPACE)) {
	
	next_code(*code, *code_sz);
	goto DONE;
    }
    
    if (is_whitespace_char(**code)) {

	next_code(*code, *code_sz);
	goto DONE;
    }
 
    if (**code == ')') {
	
	next_code(*code, *code_sz);
	goto DONE;
    }    
     
    return false;
    
  DONE:
    memset(&tk, 0, sizeof(token_s));
    t = &tk;
    //t = token_create();
    //if (!t) return false;
    
    t->type = TOKEN_SYMBOL;
    t->value.symbol = "list";

        
    if (form_is_unkown(form)) {
	form_set_type(form, COMPOUND_FUNCTION_FORM);
    }
    
    list_add_token(form->list, t);
  
    func_ok();
    return true;
}


static bool
identify_other_func(const char **code, size_t *code_sz, form_s *form)
{
    token_s *t, tk;
    const char *str;
    int len;
    
    if (ml_util_strbufcmp("car", *code, *code_sz)) {

	str = "car";
    }
    else if (ml_util_strbufcmp("cdr", *code, *code_sz)) {

	str = "cdr";
    }
    else if (ml_util_strbufcmp("cons", *code, *code_sz)) {

	str = "cons";
    }
    else if (ml_util_strbufcmp("eq", *code, *code_sz)) {

	str = "eq";	
    }       
    else {

	return false;
    }

    len = strlen(str);
    debug("%s \n", str);
    
    move_code(*code, *code_sz, len);   
    if (*code_sz == 0) goto DONE;

    if (eq(**code, SPACE)) {
	
	next_code(*code, *code_sz);
	goto DONE;
    }
    
    if (is_whitespace_char(**code)) {

	next_code(*code, *code_sz);
	goto DONE;
    }
 
    if (**code == ')') {
	
	next_code(*code, *code_sz);
	goto DONE;
    }    
     
    return false;
    
  DONE:
    memset(&tk, 0, sizeof(token_s));
    t = &tk;
    //t = token_create();
    //if (!t) return false;
    
    t->type = TOKEN_SYMBOL;
    t->value.symbol = str;
        
    if (form_is_unkown(form)) {
	form_set_type(form, COMPOUND_FUNCTION_FORM);
    }
    
    list_add_token(form->list, t);
  
    func_ok();
    return true;
}


static bool
identify_code_as_func(const char **code, size_t *code_sz,
		      identify_f func, form_s *form)
{
    bool found;
    
    code_s icode = { *code, *code_sz };
    
    stack_push(&icode, sizeof(code_s));

    found = func(code, code_sz, form);
	
    stack_pop(&icode);

    if (!found) {
	
	*code = icode.code;
	*code_sz = icode.code_sz;
	//debug("0x%02x %c \n", *icode.code, *icode.code);
    }

    return found;
}



static char*
read_character(const char **code, size_t *code_sz)
{
    char *s, *ss;
    
    if (**code != '\\') return false;
    next_code(*code, *code_sz);

    func_s();
    
    if (*code_sz == 0) goto FAIL;

    /* if the x is more than one character long, the x must be a symbol 
     * with no embedded package markers. 
     * the character name is (string-upcase x).
     * if the name is invalid, then signaling a syntax error.
     */
    size_t len = 0;
    ss = *code;
    while (*code_sz > 1) {

	if (**code == SPACE || **code == NEWLINE ||  **code == LINEFEED) {
	    
	    next_code(*code, *code_sz);
	    break;
	}

	if (**code == ')') {
	    
	    break;
	}	

	next_code(*code, *code_sz);
	len++;
    }

    if (len == 0) {

	debug_err("character name is null \n");
	goto FAIL;
	
    }
    
    s = (char*)ml_malloc(2);
    if (!s) return NULL;

    if (len == 1) {
	
	s[0] = *ss;
	s[1] = 0;
    }
    else {

	char *name = ml_util_buf2str(ss, len);	
	debug("character name: %s \n", name);
	
	char c = char_get(name);
	if (c == CHAR_UNKNOWN) {

	    debug_err("unknown character name: %s \n", name);
	    ml_free(name);
	    goto FAIL;
	}
	
	ml_free(name);
	
	s[0] = c;
	s[1] = 0;
    }

    debug("character: %s\n", s);
    
    out(ok, s);
    
  FAIL:
    ml_err_signal(ML_ERR_ILLEGAL_CHAR);
    
    return NULL;
}


static bool
identify_special_form(const char **code, size_t *code_sz, form_s *form)
{
    token_s *t, tk;
    int len;
    char *name;

    
    if (ml_util_strbufcmp("if", *code, *code_sz)) {

	len = 2;
	name = "if";
    }
    else {

	return false;
    }
        
    move_code(*code, *code_sz, len);
    
    if (*code_sz == 0) goto DONE;

    if (eq(**code, SPACE)) {
	
	next_code(*code, *code_sz);
	goto DONE;
    }
    
    if (is_whitespace_char(**code)) {

	next_code(*code, *code_sz);
	goto DONE;
    }
 
    if (**code == ')') {
	
	next_code(*code, *code_sz);
	goto DONE;
    }    
     
    return false;
    
  DONE:
    memset(&tk, 0, sizeof(token_s));
    t = &tk;
   
    
    t->type = TOKEN_SYMBOL;
    t->value.symbol = name;

        
    if (form_is_unkown(form)) {
	form_set_type(form, COMPOUND_SPECIAL_FORM);
    }
    
    list_add_token(form->list, t);
  
    func_ok();
    return true;
}


static bool
identify_macro_form(const char **code, size_t *code_sz, form_s *form)
{
    token_s *t, tk;
    int len;
    char *name;

    
    if (ml_util_strbufcmp("loop", *code, *code_sz)) {

	len = 4;
	name = "loop";
    }
    else if (ml_util_strbufcmp("return", *code, *code_sz)) {

	len = 6;
	name = "return";
    }
    else if (ml_util_strbufcmp("defun", *code, *code_sz)) {

	len = 5;
	name = "defun";
    }
    else if (ml_util_strbufcmp("defmacro", *code, *code_sz)) {

	len = 8;
	name = "defmacro";
    }    
    else {

	return false;
    }
        
    move_code(*code, *code_sz, len);
    
    if (*code_sz == 0) goto DONE;

    if (eq(**code, SPACE)) {
	
	next_code(*code, *code_sz);
	goto DONE;
    }
    
    if (is_whitespace_char(**code)) {

	next_code(*code, *code_sz);
	goto DONE;
    }
 
    if (**code == ')') {
	
	next_code(*code, *code_sz);
	goto DONE;
    }    

    func_fail();
    return false;
    
  DONE:
    memset(&tk, 0, sizeof(token_s));
    t = &tk;
   
    
    t->type = TOKEN_SYMBOL;
    t->value.symbol = name;

    debug("%s \n", name);
        
    if (form_is_unkown(form)) {
	form_set_type(form, COMPOUND_MACRO_FORM);
    }
    
    list_add_token(form->list, t);
  
    func_ok();
    return true;
}



/** 
 * The function read is called recursively to read successive objects until a right
 * parenthesis is found to be next in the code. A list of the objects read is returned.
 */     
static code_s
read_list(const char *code, size_t code_sz, form_s *form_head, form_s *form)
{
    bool found;
    bool is_nil_list;
    
    func_s();

    is_nil_list = true;
    while (code_sz > 0) {

	debug("0x%02x %c \n", *code, *code);

	if (is_whitespace_char(*code)) {

	    next_code(code, code_sz);
	    continue;
	}

	if (*code == SPACE) {

	    /* next token */
	    next_code(code, code_sz);
	    continue;
	}
	
	if (*code == ')') {

	    /* nil list: () 
	     */
	    if (form->list->next->next == form->list) {
		form->list->is_nil = true;
		form_add_front(form_head, form);		
		form_set_type(form, SELF_EVALUATING_FORM);
		form->subtype = NIL_LIST_FORM;
	    }
	    
	    next_code(code, code_sz);
	    code_s cd = { code, code_sz };
	    return cd;
	}

	is_nil_list = false;

	if (*code == '`' || *code == ',') {

	    found = read_template(&code, &code_sz, form_head, form);
	    if (found) continue;
	    
	}

	if (like_arithmetic_func(code)) {

	    found = identify_code_as_func(&code, &code_sz,
					  identify_arithmetic_operator, form);
	    if (found) {

		form_add_front(form_head, form);
		continue;
	    }
	}

	if (like_list_func(code)) {

	    found = identify_code_as_func(&code, &code_sz,
					  identify_list_func, form);
	    if (found) {

		form_add_front(form_head, form);
		continue;
	    }
	}

	if (like_other_func(code)) {

	    found = identify_code_as_func(&code, &code_sz,
					  identify_other_func, form);
	    if (found) {

		form_add_front(form_head, form);
		
		continue;
	    }
	}

	if (tolower(*code) == 'e' ||
	    tolower(*code) == 'i') {

	    found = identify_special_form(&code, &code_sz, form);
	    
	    if (found) {

		form_add_front(form_head, form);
		
		continue;
	    }
	    
	}

	if (tolower(*code) == 'l' || tolower(*code) == 'r' ||
	    tolower(*code) == 'd'){

	    found = identify_macro_form(&code, &code_sz, form);
	    
	    if (found) {

		form_add_front(form_head, form);
		
		continue;
	    }
	    
	}	
	

	if (like_num_token(code, code_sz)) {

	    found = identify_code_as_func(&code, &code_sz,
					  identify_number_token, form);
	    if (found) {
		
		continue;
	    }
	}


	if (*code == '#') {

	    next_code(code, code_sz);
	    
	    char *character = read_character(&code, &code_sz);
	    if (character) {

		list_add_char_obj(form->list, character);
		
		continue;
	    }
	    
	}

	if (*code == '(') {

	    //token_s *t = token_create();
	    token_s tk;
	    memset(&tk, 0, sizeof(token_s));
	    list_add_token(form->list, &tk);
	    form->list->front->obj.type = OBJ_LIST;
	 
	    form_s *f_head = form_create();
	    form->list->front->obj.sub = (void*)f_head;

	    form_s *f = form_create();
	    
	    list_add_char_obj(f->list, "(");
	    
	    code_s cd = read_list(++code, --code_sz, f_head, f);
	    if (!cd.code) return cd;

	    list_add_char_obj(f->list, ")");
	    
	    code = cd.code;
	    code_sz = cd.code_sz;
	}
	else {

	    found = read_symbol(&code, &code_sz, form);
	    if (!found) break;

	    
	    char *sym = obj_get_symbol(&form->list->front->obj);
	    if (var_match_binder(sym)) {
		
		if (form_is_unkown(form)) {
		    
		    form_set_type(form, COMPOUND_SPECIAL_FORM);
		    form_add_front(form_head, form);
		}
	    }
	    else {

		if (func_get(sym)) {

		    if (form_is_unkown(form)) {
		    
			form_set_type(form, COMPOUND_FUNCTION_FORM);
			form_add_front(form_head, form);
		    }		    
		}		
		else if (macro_get(sym)) {

		    //debug("macro call: %s \n", sym);
		    
		    if (form_is_unkown(form)) {
		    
			form_set_type(form, COMPOUND_MACRO_FORM);
			form_add_front(form_head, form);
		    }		    
		}
		else {
		
		    if (form_is_unkown(form)) {
		    
			form_set_type(form, SYMBOL_FORM);
			form_add_front(form_head, form);
		    }
		}
	    }
	    
	}
	
    }

    ml_err_signal(ML_ERR_ILLEGAL_CHAR);

    code_s cd = { NULL, 0 };
    return cd;
}


static bool
read_macro(code_s *cd, lex_s *lex)
{
    bool found = false;

    if (!cd || !cd->code || !cd->code_sz) return false;
    
    func_s();

    char c = *cd->code;
    debug("0x%02x %c \n", c, c);

    switch (c) {

    case '(':

	/* The left-parenthesis character initiates reading of a list. 
	 */

	next_code(cd->code, cd->code_sz);

	form_s *form = form_create();
	if (!form) return false;

	
	list_add_char_obj(form->list, "(");
		
	code_s icode = read_list(cd->code, cd->code_sz, &lex->forms, form);
	if (!icode.code) return false;

	list_add_char_obj(form->list, ")");

	
	cd->code = icode.code;
	cd->code_sz = icode.code_sz;

	found = true;
	break;

    case '\'': 

	/* A single-quote introduces an expression to be "quoted" 
	 */
	
	found = read_expression_with_single_quote(&cd->code, &cd->code_sz);
	break;
	    
    case ';':

	/* A semicolon introduces characters to be ignored, such as comments. 
	 */
	
	found = read_comments(&cd->code, &cd->code_sz);
	break;

    case '\"':

        /* The double-quote is used to begin and end a string. 
	 */

	found = read_string(&cd->code, &cd->code_sz, NULL);
	break;

    case '`':

        /* The backquote introduces a template of a data structure to be built. 
	 */

	/* TODO: backquote syntax 
	 */

	found = read_template(&cd->code, &cd->code_sz, &lex->forms, NULL);	
	
	break;


    case '#':

	/* Sharpsign is a non-terminating dispatching macro character. It uses 
	 * a character to select a function to run as a reader macro function.
	 * 1. Sharpsign Backslash
	 *    syntax: #\<x>
	 * 2. TODO
	 */

	next_code(cd->code, cd->code_sz);

	char *character = read_character(&cd->code, &cd->code_sz);
	found = !!character;
	
	break;

    default:
	break;

    }

    
    func_e();
    return found;
}



/**
 * Initialize lexer 
 */
lex_rt_t
ml_lex_init(void)
{
    func_s();

    

    func_ok();
    
    return LEX_OK;
}



/**
 * Lexical analyzing for the code
 */
lex_rt_t
ml_lex(lex_s *lex, code_s *cd)
{
    const char *code;
    size_t code_sz;
    
    func_s();

    if (!lex || !cd || !cd->code) return LEX_ERR_NULL;
    
    if (cd->code_sz <= 0) return LEX_ERR_ARGU;

    ml_util_show_buf((char*)cd->code, cd->code_sz);
    

    code = cd->code;
    code_sz = cd->code_sz;
    
    
    /* identify the tokens
     *
     * rules:
     * 1. A single escape character never stands for itself but always serves to cause the 
     *    following character to be treated as a simple alphabetic character. A single escape 
     *    character can be included in a token only if preceded by another single escape 
     *    character.
     * 2. A multiple escape character also never stands for itself. The characters between a 
     *    pair of multiple escape characters are all treated as simple alphabetic characters, 
     *    except that single escape and multiple escape characters must nevertheless be preceded
     *    by a single escape character to be included.
     */
    char x, y, z;
    while (1) {
    
	/* step 1: if at end of code, break.
	 */
	if (code_sz == 0) break;

	x = *code; /* read one character */

	debug("0x%02x %c \n", x, x);
	
	/* step 2: if x is an illegal character, signal an error. 
	 */
	if (is_illegal_char(x)) {

	    show_func_line();
	    ml_err_signal(ML_ERR_ILLEGAL_CHAR);
	    return LEX_ERR;
	}

	
	/* step 3: if x is a whitespace character, then discard it and go back to step 1. 
	 */
	if (is_whitespace_char(x)) {

	    debug("whitespace char \n");
	    
	    next_code(code, code_sz);
	    continue;
	}

	
	/* step 4: if x is a macro character, then execute the function associated with that
	 * character. The function may return zero values or one value.
	 */
	if (is_macro_char(x)) {

	    debug("macro char \n");

	    cd->code = code;
	    cd->code_sz = code_sz;	    
        
	    if (!read_macro(cd, lex)) return LEX_ERR;

	    form_show(&lex->forms);
	    
	    debug("remain code_sz:%d \n",cd->code_sz);
	    if (cd->code_sz == 0) break;

	    code = cd->code;
	    code_sz = cd->code_sz;   
	    continue;
	}


	/* step 5: if x is a single escape character, then read the next character and call it y.
	 */
	if (is_escape_char(x)) {

	    debug("escape char \n");

	    next_code(code, code_sz);
	    y = *code;

	    if (code_sz == 0) {

		debug_err("end of code \n");
		ml_err_signal(ML_ERR_ILLEGAL_CHAR);
		return LEX_ERR;
	    }

	    goto STEP_8;
	}


	/* step 6: if x is a multiple escape character, then begin a token and go to step 9.
	 */
	if (is_multiple_escape_char(x)) {

	    show("multiple escape char \n");
	    goto STEP_9;
	}


	/* step 7: if x is a constituent character, then it begins an extended token.
	 * After the entire token is read in, it will be interpreted either as representing 
	 * a Lisp object such as a symbol or number, or as being of illegal syntax.
	 */
	if (is_constituent_char(x)) {

	    debug("constituent char \n");
	    
	    /* use x to begin a token, and go on to step 8.
	     */
	    goto STEP_8;
	}
	

	/* step 8: at this point a token is being accumulated, and an even number of multiple 
	 * escape characters have been encountered.
	 * If at end of code, go to step 10, otherwise, read a character (call it y), 
	 * and perform one of the following actions according to its syntactic type:
	 *
	 * 1. If y is a constituent or non-terminating macro, then do the following:
	 *    Append y to the token being built, and repeat step 8.
	 * 2. If y is a single escape character, then read the next character and call it z(but 
	 *    if at end of code, signal an error instead). Ignore the usual syntax of z and
	 *    pretend it is a constituent whose only attribute is alphabetic.
	 *    Append z to the token being built, and repeat step 8.
	 * 3. If y is a multiple escape character, then go to step 9.
	 * 4. If y is an illegal character, signal an error.
	 * 5. If y is a terminating macro character, it terminates the token. First ``unread'' 
	 *    the character y, then go to step 10.
	 * 6. If y is a whitespace character, it terminates the token. First ``unread'' y if 
	 *    appropriate, then go to step 10.
	 */
      STEP_8:
	  ;


	/* step 9: at this point a token is being accumulated, and an odd number of multiple 
	 * escape characters have been encountered.
	 * If at end of file, signal an error. Otherwise, read a character (call it y), and
	 * perform one of the following actions according to its syntactic type:
	 *
	 * 1. If y is a constituent, macro or whitespace character, then ignore the usual syntax 
	 *    of that character and pretend it is a constituent whose attribute is alphabetic.
	 *    Append y to the token being built, and repeat step 9.
	 * 2. If y is a single escape character, then read the next character and call it z(but 
	 *    if at end of file, signal an error instead). Ignore the usual syntax of z and
	 *    pretend it is a constituent whose only attribute is alphabetic.
	 *    Append z to the token being built, and repeat step 9.
	 * 3. If y is a multiple escape character, then go to step 8.
	 * 4. If y is an illegal character, signal an error.
	 */
      STEP_9:
	  ;


	/* step 10: an entire token has been accumulated.
	 * Interpret the token as representing a Lisp object and return that object as the result 
	 * of the read operation, or signal an error if the token is not of legal syntax.
	 */
      STEP_10:
	  ;


	  
	  next_code(code, code_sz);
	
    } /* end of while */

    
    
    func_ok();
    
    return LEX_OK;
}
