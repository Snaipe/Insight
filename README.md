# <img src="doc/insight-title.png" height="96" alt="Insight Logo" />

A reflection framework for C/C++

**WARNING: This library is still experimental. Beware of bugs and API changes!**

## Features

This library enables runtime reflection for C and C++ programs, providing
runtime information and manipulation of primitive, pointer, struct, union,
and class types including the name, storage size, members, methods, ...;
namespaces, and top level functions.

Insight works by building metadata on types from the debug symbols -- as such
it cannot work on binaries without any. Make sure to compile with `-g` and/or
not strip your debug symbols!

## Samples

Samples are available in the [samples directory][samples].

## Installation

### Prerequisites

* [libdwarf++][libdwarf++]
* A GNU C/C++ compiler like GCC or Clang

### Building from the source code

```bash
$ git clone https://github.com/Snaipe/Insight.git
$ cd Insight
$ mkdir build && cd $_
$ cmake .. && make
(become root)
# make install
```

## Documentation

[ TODO ]

## Contributing

You need to observe the following rules for pull requests:

* Your modifications must be working and tested.
* Be consistent with the project's style and formatting.
* Squash your commits into one if you can.
* If new files are added, please say so in the commit message, and add the license header.
* Keep your commit messages simple and concise. Good commit messages start with a verb ('Added', 'Fixed', ...).
* Your branch must be based off an up-to-date master, or at least must be able to be merged automatically.
* Sign off your pull request message by appending 'Signed-off-by: <name> <email>' to the message.

By submitting a pull request you accept to license your code under the Lesser GNU Public License version 3.

## Credits

Logo done by [Greehm](http://www.cargocollective.com/pbouigue)

[libdwarf++]: https://github.com/Snaipe/libdwarfplusplus
[samples]: ./samples/
