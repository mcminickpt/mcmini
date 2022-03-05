# DPOR
Dynamic Partial Order Reduction model checker for C programs


## High-Level Overview of Functionality

## API Naming Reference:

The C APIs are inspired by that of Core Foundation (a low-level framework written in C for iOS/macOS). In Core Foundation, the notion of ownership over memory
is implied through the names of the functions operating on the data you work with. The goal is to alert you when you should remember to free and destroy "object"
(structs) you receive from callers, and it helps to keep everyone consistent throughout the codebase regarding memory to prevent leaks.

There are a couple of naming rules that have thus far been observed (and ideally should be kept up to keep us consistent):

1. Pointers to any defined structs are followed by `_ref` to remind you that the data you are working with has _reference semantics_; that is,
	it reminds you that you may be modifying data referenced elsewhere. This differs from types that have _value semantics_, where values are
	copied around.

2. Functions that return memory that you do **not** own are followed by *`_get`*. You should not free the memory that you receive from
such functions, as the results would be undefined since the memory is managed elsewhere (think use-after-free bugs, double free, nasty stuff).

3. Functions that return memory that you **do** own are followed either by *`_copy`*, *`_create`*, or *`_alloc`* It is your responsibility
to clean up any memory created after invoking these functions to ensure you don't leak memory

Core Foundation has more advanced notions of ownership *count*, giving some struct types automatic reference counting-like semantics. This project
doesn't require such detail and does not implement the more complicated features




