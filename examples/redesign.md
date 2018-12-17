# Mist Programming Language Redesign

I am having a redesign Mist to have cleaner and more consistent syntax. This is in the hopes of being allowing the semantics and syntax to
adhere to each other in a cohesive manner.

# Language Motivation

This language is intended to be a expression based systems language with focus on type safety and concurrency. Because of this dual focus, it is desired to have
type safety in concurrent applications. At first, this language will have shared memory parallelism as a language feature. This includes task and
data level parallelism.

Side note: I am intending to focus on core language features and defining the semantics before move onto the concurrency aspect of the language.

The language is inspired by C++, Rust, Jai, and Haskell. This language will have many features taken from functional languages that I believe would benefit the language. Some of these features include pattern matching, expression based, pure functions, and more as if decide they are needed. I am conflicted in what the primary paradigm should be for this language. I would like to make it object oriented that have features that combine object oriented with data oriented. But I do not know what that would look like or how that would work.

I am also looking at using Haskell and Rust as a large influences by have a more functional feel to the language and type system as a whole. This means that instead of inheriting from other class/structures a type would be an instance of a type class. A type class is a, more or less, an interface into the operation that a type can perform. This is similar to inheritance; however, the data theses types represent are not included. This is a form of sub-typing.

Generics will have the ability to be bounded. This means that a type class or base class is given as the type of the generic parameter which forces the compiler to require any type that is given as a type parameter to be a sub type of this type. This requires the generic type to have an expected interface and will allow for more and better type checking and errors.

# Language Syntax and Semantics

## Builtin Types

There will be build in support for many different primitive and aggregate types. These types are listed below. There will be a dynamic array; however, I am undesigned how it should be incorporated into the language. The options are to add special syntax to arrays to distinguish it from static arrays such as Odin's solution. The other option is to have is an external type that is included in the languages default namespace, similar to how Rust does it with Vec. I am partial to the later option because it will be easier to parse and makes the semantic analysis slightly simpler.

> Primitive Types
* i8
* i16
* i32
* i64
* u8
* u16
* u32
* u64
* char
* atom
* static arrays
* pointer
* references
* constants

> Builtin Aggregate Types
* string
* hash map
* dynamic array


## Expressions

Mist support standard infix notation for binary and unary expressions. Since everything in this language is an expression (event declarations) the standard control flow operations are expressions.

### Control Flow

#### If
```
if <expression> <expression>
[<elif <expression> <expression>>]*
[else <expression>]
```
#### While
Iterative conditional based loop.
```
while <expression> <expression>
```
#### For
Iterative range based loop.
```
while <pattern> in <expression> <expression>
```
#### Loop
Iterative unconditional loop.
```
loop <expression>
```
#### Match
```
match <expression> {
	[<pattern> >> <expression>]*
}
```

### Declarations

#### Variables

Variables are allowed to be declared in the global and local scopes. The declaration of these variables is as follows:

``` code
x: f32;
```
This declares a variable x with type 32 bit float without a value. The value is initialized to 0.0 as the default.

``` code
x := 0.0;
```

This declares a variable x with a value of 0.0. The type of x is inferred to be a 32 bit float (or f32) because of the initializing expression.

Variables can also be initialized in by a destruction pattern. Such as tuples, enums, or structures. Everything on the left hand side is considered a pattern, some of the patterns is a single variable. Meaning the entire result of the expression is bound the identifier.

The general structure of a variable declaration is as follows:

```
<pattern> ':' <type_pattern> '=' <expression>
```

This language has support for native multi return from functions. But this coupled with patterns adds some complexity to the variable declarations: difference between when a pattern is used or when there are multiple variables being declared.

e.g.

```
Enum(x, y) := function_returning_enum();

--or--

x, y := multi_return_function();

-- with type annotations --

Enum(x, y) : f32, string = function_returning_enum(); // i do not know if this is needed

--or--

x, y : f32, string = multi_return_function();
```

#### Function

Functions can be declared in global and function scope. Functions declared in function scope are only visible to the scope of the function. Function syntax is:

```
<identifier> '::' '(' <parameter_list> ')' ['->' <return_type_list>]? <expression>
```

An example functions for basic binary operators;

```
add :: (x: i32, y: i32) -> i32 {
	x + y
}

subtract :: (x: i32, y: i32) -> i32 {
	x - y
}

multiply :: (x: i32, y: i32) -> i32 {
	x * y
}

divide :: (x: i32, y: i32) -> i32 {
	x / y
}


--- another way to write this would be ---

binary_operator(op: (i32, i32)->i32, x: i32, y: i32) -> i32 {
	op(x, y)
}

add := binary_operator((+))
add := binary_operator((-))
add := binary_operator((*))
add := binary_operator((/))

```

This is an example of function currying. Where if at compile type a function is given the first parameters, it will generate a function with the given parameters filled.

```
binary_operator = ((i32, i32)-> i32, i32, i32) -> i32
-- a function that takes a binary function and two more parameters

add = (i32, i32) -> i32
-- this is because the first parameter has benen filled by the binary arithmetic operator (+) (notation could change, this is Haskells notation).

```


There are also anonymous functions not bound to a name. This is because function
functions are a first-class entity in the language. This means they have a value, to the users this value is irrelevant. The syntax for this is as follows:

```
fn := \x, y, z -> f32 => {
	x + y - z
};

assert fn(1.0, 3.0, 4.0) == 0.0
```

#### Structure/Classes

#### Type Class

#### Imports

#### Type Alias



## Generics
### Structure
### Function
