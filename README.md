# `billionaire`

A C implementation of the [Billionaire card game](https://ourpastimes.com/billionarie-card-game-rules-5970604.html).

`billionaire` is played between two or more clients connecting to a
central server. Messages are passed between client and server as
outlined in the [`billionaire` protocol](message_protocol.md).
The server is written in C, and clients may be written in any language
provided they correctly implement the aforementioned message passing
protocol. Some boilerplate code has been provided for a Python 3 client,
however this is just one of many different ways of approaching a client
implementation.

## Requirements

`billionaire` requires the following external libraries to compile and
run:
- [libevent](https://github.com/libevent/libevent)
- [json-c](https://github.com/json-c/json-c/)
- [xxHash](https://github.com/Cyan4973/xxHash)

## Getting Started

`billionaire` can be cloned and compiled by simply running
```bash
$ git clone https://github.com/hdp1213/billionaire.git
$ cd billionaire/
$ make
```

This creates the billionaire server. By default, the server's port is
`5555`. Consult the help output of `billionaire-server` for
game-specific options.

A dummy Python 3 client can be run alongside the server by running
```bash
$ ./python/client.py
```
when a `billionaire-server` is already running. The client will simply
offer cards until it no longer has any valid offers to give.

## Contributing

It is recommended before contributing to install the following libraries
for testing:
- [libcheck](https://libcheck.github.io/check/)
- [cppcheck](https://github.com/danmar/cppcheck/)

For Python development, the following are also recommended:
- [pytest](https://docs.pytest.org/en/latest/index.html)
- [pycodestyle](https://github.com/PyCQA/pycodestyle)

A pre-commit hook is also provided which can be installed locally by
running the following in the root `billionaire/` directory:
```bash
$ ./hooks/create-hook-symlinks
```

These hooks will check any staged code for syntax errors and the like.
