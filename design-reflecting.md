
### language spec 
I think a programming language should be mostly defined in itself. A language spec should be divided into two parts, a small core of operators that play the role of axioms, 
and the rest of the language, which, like theorems, are defined in terms of the axioms.

### terminal editor

@__

e.g.:  quit monalisp

@ (q)

@: is also good.


Consider below codes in editor of shell-terminal:

   **@** (eql "foo" "foo")

   **>** (eql "foo" "foo")

   \* (eql "foo" "foo")

I think "@" is more elegant than others, although ">" and "*" is good.
"@" and ">" all have a forward direction.


### design-hackable

Any ideas that make designing more easy to hack, try them in monalisp.

- layout design.
- interface design.
- architecture design.
- system design (complex systems)
- ...

Consider more deeply about this.



### less init-forms and more optional brief main-forms

Too many optional init-forms in the loop makes it is hard to program and the code is not brief and elegant.

### implement the core form of a common lisp form as a minimize prototype


## circle-like is more elegant than non-cricle
"@" is more elegant than ">", "*", ">>", ">>>" and "$";

"*" is more elegant than ">", ">>", ">>>" and "$";

"$" is more elegant than ">", ">>" and ">>>".

## guide-like is more useful than non-guide

">" is more useful than "*" and "@";

"@" is more useful than "*".

## the balance of elegant design and useful design.

"@" is more designable than "*", ">", ">>", ">>>", "#" and "$".

So, "@" is choose.


## more clear and brief keywords

"return" should be replaced by "out", because:

> out: move away from a place
return: got back to a place
"out" is more clear and brief than "return".


## use more words like "i" to show a free style

i: (out (+ 1 2 3))

6

@ (out (+ 1 2 3))

"@" is like "i" including a nut as the foundation.


## db

- (add @list to @db)
- (search @list in @db)
- (del @db as @list)
- (update @db as @list)
- (show @db)
- (pull @db from @where)
- (push @db to @where)
- (merge @db-list)



## arch overview

V1:
![Image](doc/arch-draft-v1.jpg "arch-draft")


## distributed codes in servers as list object

The lisp loader is in the local application.
But most other codes can be distributed in the servers, and can be changed as we expect.


## the statistics, search engine and QA system
  
Statistics like search engine is used to construct the world of questions and to provide 
a list guides for the question’s answer.


## language name

monalisp

ml

mona

mn

m.


## create a mini parser first, then extend it based on the design for special products


## easy-to-type-on-keyboard

- object.type

"list.int" is better than "list <int>" and it's more easy to type on keyboard.














