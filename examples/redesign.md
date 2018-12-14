# Mist Programming Language Redesign

I am having a redesign Mist to have cleaner and more consistent syntax. This is in the hopes of being allowing the semantics and syntax to
adhere to each other in a cohesive manner.

# Language Motivation

This language is intended to be a systems language with focus on type safety and concurrency. Because of this dual focus, it is desired to have
type safety in concurrent applications. At first, this language will have shared memory parallelism as a language feature. This includes task and
data level parallelism.

Side note: I am intending to focus on core language features and defining the semantics before move onto the concurrency aspect of the language.

The language is inspired by C++, Rust, Jai, and Haskell. This language will have many features taken from functional languages that I believe would benefit the language. Some of these features include pattern matching, expression based, pure functions, and more as if decide they are needed. I am conflicted in what the primary paradigm should be for this language. I would like to make it object oriented that have features that combine object oriented with data oriented. But I do not know what that would look like or how that would work.

I am also looking at using Haskell and Rust as a large influences by have a more functional feel to the language and type system as a whole. This means that instead of inheriting from other class/structures a type would be an instance of a type class. A type class is a, more or less, an interface into the operation that a type can perform. This is similar to inheritance; however, the data theses types represent are not included. This is a form of sub-typing.

Generics will have the ability to be bounded. This means that a type class or base class is given as the type of the generic parameter which forces the compiler to require any type that is given as a type parameter to be a sub type of this type. This requires the generic type to have an expected interface and will allow for more and better type checking and errors.

# Language Syntax and Semantics

## Primitive types

## Variables
