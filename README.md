# consensus-yaraft
[![Build Status](https://travis-ci.org/neverchanje/consensus-yaraft.svg)](https://travis-ci.org/neverchanje/consensus-yaraft)

consensus-yaraft is an embeddable distributed log storage library that provides strong consistency based
on the Raft algorithm.

## Features

- **Fault tolerance & Strong consistency**: every single log will be synchronously replicated through the [Raft](raft) state machine. Failure of minority doesn't impede progress.
- **Multi Raft**: A process creates 100 raft nodes doesn't have to create 100 threads. The background timer, the disk io thread pool (`ReadyFlusher`), even the FSM task queue, can be shared between raft nodes. 
- **Simple**: the basic operation for writing a slice of log includes only `ReplicatedLog::Write`.

[raft]: https://raft.github.io/

## Limitation

- The library doesn't provide with client-interaction support.
- Writes can only be applied to the leader. Any write operations to a non-leader will get rejected.

## Installation

consensus-yaraft is written in C++11, please ensure a compiler with C++11 support is installed.

Ensure you have cmake, unzip, libtool, autoconf installed on your system.

Ensure you have leveldb, zlib, openssl installed, because 
[brpc relies on them](https://github.com/brpc/brpc/blob/master/docs/cn/getting_started.md).

On Ubuntu 14.04

```sh
sudo apt install zlib1g-dev libboost-all-dev libsnappy-dev
bash install_dependencies.sh
bash compile_proto.sh
bash run.sh build
```

Once the building becomes success, the library would be installed in the directory `output/`.

## Internals

For more details about the architecture and design of consensus-yaraft, please read 
[this article](https://github.com/neverchanje/consensus-yaraft/wiki).
Currently we relies on [brpc](brpc) to implement network communication.

## MemKV

[apps/memkv](apps/memkv) is a prototype of using consensus-yaraft to implement a raft-based in-memory key-value store.

## License

consensus-yaraft is under the Apache 2.0 license. See the LICENSE file for details.

[brpc]: https://github.com/brpc/brpc
[raft]: https://raft.github.io
