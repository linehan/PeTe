# PeTe

This is a permutation testing library written in C. The object is to
provide an easy-to-use API and a scalable resource model that can allow
experimental mathematicians to easily measure statistics on sets of
permutations.

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

