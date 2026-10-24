# C++ Default Guidelines

Collected from https://github.com/janwilmans/guidelines ([README](https://github.com/janwilmans/guidelines/blob/main/README.md)
and [guidelines_details.md](https://github.com/janwilmans/guidelines/blob/main/guidelines_details.md)), see there for
examples, memes and the full rationale.

## These are guidelines, not rules

Every "rule" below is a **guideline**: a sensible default, not a hard requirement. Exceptions are permitted whenever the
situation calls for it. Do not follow them blindly; think about the code, apply the default, and if it works well do not
complicate things further.

The motto is **comply or explain**: when deviating, be able to say why. Make the deviation explicit, for example by
moving it into a function whose name says **what** it does, with a comment explaining **why** (see [[I.30]](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#i30-encapsulate-rule-violations)).

## Scope

- Do not apply the guidelines to third-party code (see `.clang-format-ignore`), it makes updates harder to diff.
- Conform to the local style of the existing code; do not convert code to 'your' style function by function. Style
  changes are best made per project, in big chunks.
- Much of this repository is legacy code that predates these guidelines. Apply them to new and changed code; do not
  refactor unrelated code just to comply.

## Style

- Whitespace and layout are enforced by `.clang-format` and `.editorconfig`; one consistent style matters more than which style.
- Use nouns for class names and verbs for function names. Avoid abbreviations in names.

## Mottos

- KISS (Keep It Simple Stupid), followed by YAGNI (You Ain't Gonna Need It).
- DRY (Don't Repeat Yourself): do not copy-paste code to modify it slightly, write re-usable code.
- Fail early, use guard clauses: check pre-conditions first and bail out as soon as possible.
- Do not throw to catch: do not throw exceptions with the intent of catching them yourself.
- Design for debugging is a self-fulfilling prophecy.
- More explicit code is better code.
- Create a pit of success: make interfaces and types hard to use incorrectly.
- Comments are a code smell: names explain the **what**, comments explain the **why/how**.
- Prefer the most fitting tool for the job: the more specialised tool has fewer features and allows fewer mistakes
  (e.g. `std::lock_guard` over `std::unique_lock`, `std::unique_ptr` over `std::shared_ptr`).

## The Default Guidelines (highest level summary)

- [H.1] Turn on warnings and use sanitizers (`-fsanitize=address,undefined` or `-fsanitize=thread`, the latter cannot be combined).
- [H.2] No raw loops: prefer range-based for or a standard algorithm that expresses the intent.
- [H.3] Avoid global mutable state.
- [H.4] Keep the scope of variables and type declarations as limited as possible.
- [H.5] Initialize all variables at declaration.
- [H.6] Use `constexpr` or `const` whenever you can, but not for member variables and return types.
- [H.7] Use `[[nodiscard]]` on pure free functions and `const` member functions that only return a value.
- [H.8] Avoid returning values using arguments (out-parameters); return a value or a struct.
- [H.9] Use automatic resource management (RAII).
- [H.10] Follow the rule of 0 or the rule of 5, in that order: a class either deals exclusively with ownership (rule of 5)
  or not at all (rule of 0).
- [H.11] Avoid owning raw pointers (non-owning raw pointers are fine).
- [H.12] Avoid manual memory management using `new`, `delete`, `malloc`, `free`, etc. (`new` is allowed for Qt/CopperSpice objects).
- [H.13] Do not use C-style casts.
- [H.14] Do not add member variables to classes used as interfaces (pure virtual classes with a virtual `= default` destructor).
- [H.15] Do not use `protected` member variables.
- [H.16] Avoid `volatile`, `const_cast`, `reinterpret_cast`, `typedef`, `register`, `extern`, `protected` and `va_arg`.
- [H.17] Make all destructors of classes used in runtime polymorphism virtual.
- [H.18] Avoid references as data members of a class.

## Project structure and files

- Use a consistent directory structure (e.g. https://github.com/vector-of-bool/pitchfork).
- Files should not exceed 2000 lines; if exceeded, refactor.
- Keep unrelated code in separate files; related classes and functions may share a file.
- Use `#pragma once` as include guard in headers.

## Functions and lambdas

- Add `[[nodiscard]]` and `const` to all getters.
- Return by value; wrap (C-style) functions with output parameters in a function that returns by value.
- Keep functions limited to 42 lines; if exceeded, refactor, while keeping related logic together.
- Functions and lambdas should not have more than 3 arguments.
- Explicitly capture variables in lambdas.
- Do not use `inline` for performance reasons.
- Do not return `nullptr` to indicate an error.
- Pass fundamental types (e.g. `double`) and small objects by value.

## Classes

- Make RAII classes undiscardable using `[[nodiscard]]`.
- Put `[[nodiscard]]` on `const` member functions in the header, not repeated in the implementation.
- Mark all single-argument constructors `explicit`.
- Define interfaces as pure virtual classes with a default virtual destructor and no member variables.
- Initialize all members using in-class initialization or the member initializer list.
- Avoid `protected`, it usually indicates a design problem.
- Mark all destructors of classes in an inheritance hierarchy `virtual` or `override`.
- Declare `public`, `protected` and `private` in that order.
- Avoid assigning members in the constructor body; use the member initializer list or in-class initialization.
- Whenever you use `final`, add a comment explaining why and what could break if it were removed.

## Variables

- Keep the scope of variables as small as possible.
- Initialize variables at declaration.
- Declare variables on individual lines (no `int a, b, c;`).
- Use `const` whenever possible.
- Do not use `const` for member variables (makes the type unassignable and unmovable).
- Do not use references as member variables (same reason).
- Avoid globals; if really needed, encapsulate them in a static access function (Meyers singleton) and beware of the
  static initialization order fiasco.
- Prefer `auto` where it makes code **more** readable.
- Use `auto*` when receiving a pointer.
- Prefer `static_cast` or `dynamic_cast` over C-style casts; explicitly constructing a native type for a narrowing
  conversion is allowed (`int a(v);`).
- Do not use raw pointers for ownership; use RAII objects or smart pointers.
- Use `static constexpr` for scoped constants in implementation files, functions or classes.
- Use `inline constexpr` for globally scoped constants.
- Prefer enum classes over booleans (as arguments).
- Group related variables that are always passed together into structs.

## Exceptions

- Ensure thrown exceptions are handled at a higher level.
- Prefer standard exception types over custom exceptions; convert third-party exceptions that do not derive from
  `std::exception` into ones that do.
- Do not use exceptions for code-flow decisions; they are for cases where normal execution cannot continue.

## Misc

- List class data members in order big to small (reduces padding; note it affects the default `<=>`).
- Put switch statements in a separate function where each case only returns immediately.
- Do not use more than 2 nested levels of conditional statements; if exceeded, refactor.
- Prefer `nullptr` over `0` or `NULL`.
- Prefer functionality from the standard library over third-party libraries (e.g. Boost).
- Move `static_assert` out of headers, so they are checked only once.
- Empty lines between data members are a code smell (related members should be grouped).
- Do not return reference types, such as pointers or `std::string_view`, from a function.
- Use `std::string_view` for string arguments that do not need zero-termination.
- Use `const std::string&` for string arguments that **do** need zero-termination (e.g. functions wrapping C functions).
- Take `std::string` by value for constructor arguments that are sunk (moved) into a member.
- Use `const std::filesystem::path&` for strings that refer to file-like objects (files, devices).
- Avoid `requires requires`; give the concept a name.
