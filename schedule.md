
## Aug-w3-w4
- [Hack sbcl manual](http://sbcl.org/manual/sbcl.pdf)
- [Hack ecl manual](https://common-lisp.net/project/ecl/static/manual/)
- [Hack arc guide](http://paulgraham.com/arc.html)

## Sep-2018-to-Feb-2019
- Hack sbcl
- Hack ecl
- Hack clisp(again)
- Hack arc
- Consider more about how to extend common lisp for monalisp features.
- Create interesting codes for systemic understanding of common lisp.

holiday time (Feb-2019)

## Feb-2019-to-2020

- Desgin monalisp
- Implement monalisp parser (lisping in this summer 6~9)

### Events

- June-15-2019-to-June-21-2019

hack lexer of parser

- June 14, 2019

hack parser

- June 9, 2019

hack parser

- two weeks, from August 1 2019 to August 13 2019

hack parser (syntax AST tree has been constructed and verified, a demo has been shown)

- two weeks util the end of August 2019

[**goals**]

@ hack the core parts of parser (show a demo)  **#1**

@ improve codes and warnings  **#2**

[**#1**]

@ implementing for the scope and extent

@ implementing for below features:

when + do

if + do

return + return-from

macro feature

cond

loop + as + in

loop + for + in

out + out-from

dolist


- Sep 2019

	call user's macro
	
	gc
	
	load several lisp files
	
	create the name list of the syntax object with const type as to reduce the use of 
	dynamical memory(pratical via the BNF file and the AST tree).
	
			
- 202x

	test cases and bug fixing
	
	call user API in C (a demo)
		
	load more lisp codes and fix the bugs

	think deeply about the features
	 
	a debugger

	db object

	create/search/update db (a demo)
	
	more+
	
## 2019

Releases monalisp v1.0 (baby parser)

### 2018-12-22, monalisp v1.0, demo version

**Forms implemented and outputted with test-cases:**

- car, cdr, cons, eq, atom, list, print, number operations

- setq, if

- loop, return, defun

See: all *.lisp in the "test-cases" folder


**Forms implemented but needed more tests:**

- defmacro

- quote

- defconstant

- defstruct

See: [demo.lisp](src/demo.lisp)





	


   
   





