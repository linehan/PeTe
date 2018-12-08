# PeTe

PeTe is **pe**rmutation **te**sting application and library designed to
support mathematcal experiments on large sets of permutations. There are
*n!* permutations of length *n*, so working with them is always going to be
a chore, and intractable in general.

PeTe is written in C and heavily multithreaded for performance, and work is
in progress to allow scale-out in a distributed cluster to further increase
its capabilities.

The goal of this project is to provide an easy-to-use API and a scalable
resource model that can allow experimental mathematicians to easily measure
statistics on sets of permutations.

## Setup

The ``pete`` package can be built as either

- standalone binary program
- static C library

### Building as binary

When built as a binary, ``pete`` can be invoked like any other
program.

```
make
```

### Building as static library

When built as a static library, ``pete`` can be linked with other
C code and used to support your application.

```
make library
```

### Linking the library

