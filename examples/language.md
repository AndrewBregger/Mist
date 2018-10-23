# Mist Language Motivation and Semantics

This document is to allow me to organize my thoughts and ideas about this language. For the most part is will hold an almost
complete BNF of the language. The first part holds some ideas I had before starting to write the semantics and just some thoughts
about how things could work. This is a work in progress and no semantic or sytactic element is final.

## Motivation
Exploring the idea of developing a system level, data/object oriented language with a
strict type system (similar to Rust. Rust's type system is inspired by Haskell).
There will also be an semantic where everything is constant by default. There will only
be a mutable state when stated explicitly.
This will use the 'mut' keyword.

This has the implication that I can make structure/classes as
constant. This would mean that classes and structures declared as this type
would always be constant and cannot be casted to negate it. If I do this,
would this be the default semantic or would it need to be specified?

     mut struct Foo { <fields> }

this would be const
    struct Foo { <fields> }
or
    const struct Foo { <fields> }

this would be mutable


    struct Foo { <fields> }


To reiterate, if a structure is defined to be constant, every instance of it will be
constant.

Foo foo; // this will give an error, because it needs to be initialized with a value.
Foo foo = {...}; // this will be fine; however, the values set at initializion will be the only values.
                 // the methods for this structures will only allow constant operations of the object, however,
                 // it should define methods for tranforming the object into new version by creating new objects.
                 // This idea stems from functional langauges where everything is constant and there is no mutable state.

NOTE: syntax above is not actual syntax, is it for demonstration purposes.


Actual Syntax:

    Foo :: const struct {
    }

    Foo :: struct {
    }

    Vector :: struct[T, N] {
        data: [N]T
    } where T = Integral, N = i32 derive Read, Show, Debug, Index
    // Integral would be defined as a type class

    Array :: struct[T, Alloc = Context.DefaultAlloc] {
        data: *T,
        size: i32,
        alloc: Alloc
    } where T = Default
      derive Debug, Show, Iterator, Index


## Object Oriented Paradigm

Aside on object oriented paradigm, I am now thinking this language could be an optional object oriented lanague. this
Means that by a compiler flag or maybe it is always on, structs can 'inherit' from other structs. This will have limited oo features. The primary
oo features will come from type classes, which will allow for an expected set of data and methods. I am debating letting only
a single layer of inheritance amongst structs however, allow type classes to depend on other type classes (semi multi inheritance)


    Readable :?= trait | class | ... {
        <local_or_function_declaration>
    }

--or--

    Readable :: trait | class | ...[<template-parameters>] (derive <name-list>)? {
        <local_or_function_declaration>
    } (derive <name-list>)?

Function is this language will be implicitly pure depending on how it is used and the functions that are used in the body.
This will be done at compile time. A function can be marked as pure if it is intended for it to be, the compiler will then
check if all of the expression adhere to this constrait.

function syntax:

    name :: <function-attribute-list> (a A, b B = default) -> (c C) {
    }

    function-attribute-list = <function-attribute>, <function-attribute-list> | <function-attribute>

    function-attribute = pure | inline | <foriegn-function>

    //          foriegn library symbol, function name in library
    foriegn-fuction = foriegn <id> <string>

## Build in data types:
    [8-64] = 8, 16, 32, 64


    i[8-64],
    u[8-64],
    f32,
    f64,
    string,
    byte,
    atom // ? this would map to a unicode code point, it could default to u8 and a compiler flag should set it to u16 or u32
    static array, // I have not determined the notation appropriate for this.
    dynamic array, // I am wondering if this should be a language defined or library defined that is global.
    hash maps // there will be a collections library that will have the other major data structurs (queue, maps, stack, priority queue)
    tuples, // (T0, T1, ...TN)
    null, // this could be of type Null<T> where T is the type being assigned too and Null represents nothing for every type T.
    pointers, // *T, pointer to T
    <> // unit
    ref //


## User defined data type:

enum and struct, and of course alias

    <id> :: enum | struct [<template-params>] {

    } <modifiers>

    modifier = where <where-item> | derive <derive-item>

### Operator Overloading:

Operators can be overloaded; however, they must comply with the expected arity. IE. + is a binary operator so it will expect
an arity of 2, while - has an arity of 1 or 2 depending on usage. If there are more than expected then it will be an error.


    <op|op-pair> :: (...) -> Ret {

    }

    op-pair == [] | ()


## Function applications

A resent idea that comes from Haskell is the ability to build new function from applying some of
the parameters to the function. This will be defined as returning a new function that is of less arity 
then the applied function. I have not thought about how this can be applied to methods. For now it will
only be applied to pure functions.


    foo :: (x: i32, y: f32) -> f32 { ... } // this has type i32 -> f32 -> f32 || (i32, f32) -> f32


if the following funciton:

    foo(1);

is applied then the result is a new function.
The resulting type is f32 -> f32 || (f32) -> f32

I am considering allowing:

    foo(y = 2.0);

This will construct a different type: i32 -> f32 || (i32) -> f32
This will know that the f32 parameter is being bound and construct a different
function with the appropriate function bound.

I am wondering if this needs to have a different syntax to make it clear.

## Keywords:
* if
* else
* use
* while
* for
* loop
* continue
* break
* return
* yeild?
* where
* derive
* enum
* struct
* foriegn
* pure
* inline
* type
* pub
* mut

# Language Semantics

## Variable Declarations

     <variable_item> := <ident> : <type_spec> <init_expression>? ; |
                        "mut" <ident> : <type_spec> <init_expression>?;
                       
     <init_expression> := = <expression>
          
A variable declaration consists of an identifier, an optional type specification, and an optional
initialization expression. By default, variables are declared in a constant state. Meaning they
value of the variable cannot be changed by any external operation. To allow variables to be changed
an optional keyword mut is able to be added to the front which will declare the variable to be
mutable.
     I am considering moving the mutability to the type system. This would be more concise through
     out the language since the functionality will automatically be applied to function parameters.

Example:
     // without type annotations
     x := 1.0
     mut x := 1.0
     
     // with type annotations
     x : f32 = 1.0
     x : mut f32 = 1.0
     
Notice in the type annotated version the mut keyword has moved from before the identifier to the type
annotation. This allows for a cleaner semantic understanding since the same notation is used in
function parameters to show which ones are mutable within the functions scope; otherwise, they
can just be accessed.

## Primitive/Builtin Types

* u8
* u16
* u32
* u64
* i8
* i16
* i32
* i64
* byte
* char
* string
* Pointers
* References?
* static arrays
* dynamic arrays
* hash map
* Unit
* ref
* tuples

## Control Flow

There are multiple types of control in Mist. Most/All of them are common knowledge from other
languages. 

### If

     <if_expression> := 
     if <boolean_expression> <expression> <else_expression>
    
      <else_expression> :=
     else if <boolean_expression> <expression> <else_clause> |
     else <expression> |
     
This is an if expression (yes, this says expression, I will get to that later...similar to Rust).
This construct returns the result of the last expression evaluated. However, if the result of the
if is not used, then the result will be ignored. Otherwise, the resulting type of each path must be "similar".

Example:

       
     if x % 2 == 0 and x % 8 == 0 {
          /// some other stuff
          x ** 2
     }
     else if x % 2 == 0 and x % 6 == 0 {
          x += 5
          x ** 4
     }
     else x
     /// I think this condition should give a warning since the operations on x are returned to nothing.

Explanation of example or a better example.

## For Loop

     <for_expression> :=
     for <ident> in <expression>
          <expression_block>
          
For loops are used to iterate through a container. This can be any container that is iterable, this will be
defined by the Iterable type class. This type class will define the functions and types needed. I am considering
using the for loop expression result to function as list comprehension and each iteration the result will be added
to the new list. I think this should have a special syntax or if just assigning it to a value should be enough.

     x := <for_expression>
     
     /// or something like
     
     x := [] <for_expression>
     
## While Loop

     <while_expression> :=
     while <bool_expression> <expression>

While loops are the same as they are in other languages. The result of this is either the result of the last
expression of loop body or unit. The prior option would be interesting.

Another possible semantic is the combination of a while loop and do-while loop. The change would be the omission
of the bool expression as the condition.
     while <expression>
Here instead an implicit variable will be placed there and the result of the final expression of the body will be used there.
Therefor, this result must be a boolean or can be implicitly casted to one. This variable would be set to true for the first iteration.
making it equivalent to a do-while.
     
     while {
          can_continue()
     }
     /// I guess a question comes when it is consider a situation when the state of the loop is dependent on this variable.
     /// Should the programmer be allowed access to this variable? Or could they be given access through some context of the
     /// program, Context.conditional.

## Loop

     <loop_expression> :=
     loop <expression>
     
Loop construct is an infinate loop. It is equivalent to:
     
     while true <expression>

The result of the expression will always be the unit value.

## Defer expression

     <defer_expression> :=
     defer <expression> <if_else_conditional>?
     
     <if_else_condition> :=
     if <boolean_expression>
     <else_tail>?
     
     <else_tail> :=
     else <expression>
     
Defer expression are expression that occure at the end of the scope they were declared. There is an optional if else conditional
that allows for defers to have a conditional execution as well as a secondary option that can be executed instead. This expression
always results in unit.

## Match expression

     <match_expression> := match <expression> { <match_clause_list> }
     
     <match_clause_list> := <match_clause> | <match_clause>, <match_clause_list> |
     
     <match_clause> := <match_pattern> = <expression> // i am considering :, =
     
     <match_pattern> := <ident> | <path> | <tuple_pattern> | <path> <match_pattern> | <literal>
     
     <path> := <ident> | <ident> . <path>
     
     <tuple_pattern> := ( <ident_list> )
     
     <ident_list> := <ident> | <ident> , <ident_list>
     
Match expressions replace switchs in functionality as well as explands on what it is able to match against. At a basic
level it can match against literals. An extension to switches, matches are able to match to the types of enums and dispatch their content to another pattern that follows. Some of this functionality will be fleshed out when I figure out how enums, unions, and
structs should work.

## Built threads?
I am thinging about how this could work and be useful to a developer. Right now I am thinking it could be a simple detached thread that the a library can give more functionality to. I dont know yet.

# User Defined Types

## Functions

This needs to be looked at. I do not think it is complete.

     <function_declaration> := <function_specification> = <expression>
     
     <function_specification> := <ident> :: [<generic_parameters>] (<parameter_list>) <return_type> <where_clause>
     
     <parameter_list> := <parameter_item> | <paramter_item>, <paramter_list>
     
     <parameter_item> := <ident> <type_annotation>?
     
     <generic_parameters> := <generic_type_and_bounds> | <generic_type_and_bounds> , <generic_parameters>
     
     <generic_type_and_bounds> := <ident> <type_bounds>?
     
     <type_bounds> := : <type_bounds_list>
     
     <bounds_list> := <type_spec>
     
     <type_annotation> := : <type_spec>
     
     <where_clause> := <where_item> | <where_item> , <where_clause>
     
     <where_item> := <ident> : <type_bounds_list>

Mist have a different syntax then those in a similar class (ie. Rust, D, C++, C#). The generics come before the
parameter list and are enclosed in square brackets followed by the parameter list. The parameter list is following
many functional languages which have parameter type deduction. Some or all of the parameters are allowed to have the
their type omitted. This is forcing the compiler to determine the type bounded from usage of the variable. For example, 
if an un-typed parameter is used in an addition and is then index, then the type will be given the type bounds that define
addition and indexing.

## Struct

     <struct_declaration> := <ident> :: [<generic_parameters>] struct { <field_list> } <where_derive_clause>
     
     <field_list> := <field> | <field> , <field_list> |
     
     <field> := <ident> : <type_spec> <init_expression>?
     
     <where_derive_clause> := <where_clause> <derive_clause> | <derive_clause> <where_clause> |
     
     <derive_clause> := derive <type_spec_list>

Structures is a collection of types. These types can be declared as generic and the entire structure can derive from
type classes that expand its functionality and all it to take advantage of other language features. 

## Methods

Methods are defined in an impl block for the receiving struct or enum.

     <impl_declaration> :: [<generics>] impl { <function_list> }

The only syntatic difference between a function and method is the first paramter must be 'self' or 'mut self'. Otherwise,
it is a static function of that type.

## Type Classes

     <type_class_declaration> := <ident> :: <generics> class { <member_list> }
     
     <member_list> := <function_declaration> | <function_specification>

Type classes are similar to Haskell's. They can be thought of as interfaces to language functionality. They provide
a way of defining similarities between functionality and behavior amongst types. TODO: Learn more about Haskell's type classes.
## Enum

     <enum_declaration> := <ident> :: <generics> enum { <enum_member_list> }
     
     <enum_member_list> := <enum_member> | <enum_memeber, <enum_member_list> | 
     
     <enum_member> := <ident> = <const_expr>
                    | <ident> ( <type_list> )

Enum are a combination of C++ enums and a tagged union. The elements can be either a name associated to a value or it can be
a union of where the name is a tag.


## Unions?
