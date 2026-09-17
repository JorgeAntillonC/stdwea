# stdwea

### My own libc for Windows

stdwea is a collection of C libraries written for Windows systems programming.

The project provides my own implementations and utilities for common tasks that I want direct control over instead of relying entirely on the standard C library or higher level abstractions.

## Purpose

stdwea is mainly built as a personal systems programming library and as a foundation for other low level projects.

The goal is to provide small and reusable components while keeping the implementation close to the underlying system.

## Libraries

The project currently includes

* `gdefwea.h` General definitions and common types
* `memwea.h` Memory related utilities
* `strwea.h` String handling
* `mathwea.h` Mathematical utilities
* `iowea.h` Input and output utilities
* `dynbuffwea.h` Dynamic buffer utilities
* `phashtablewea.h` Hash table implementation
* `glwea.h` General library utilities
* `wormbuff.h` Buffer utilities
* `stdwea.h` Main library header

## Platform

Currently Windows only.

The library makes use of Windows APIs where appropriate and is intended primarily for low level Windows development.

## Design

stdwea is written in C with a focus on

* Low level control
* Small reusable components
* Minimal abstraction
* Direct interaction with the operating system
* Predictable behavior
* Reuse across systems programming projects

## Usage

The headers can be included directly in C projects.

```c
#include "stdwea.h"
```

Individual libraries can also be included when only specific functionality is needed.

## Development

stdwea is actively developed alongside my other systems programming projects.

The API and implementations may change as the library evolves.

## License

No open source license has been granted at this time.

All rights reserved.
