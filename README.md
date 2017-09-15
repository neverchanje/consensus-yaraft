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

On Ubuntu 14.04

```sh
sudo apt install zlib1g-dev libboost-all-dev libsnappy-dev
bash install_dependencies.sh
bash compile_proto.sh
bash run.sh build
```

Once the building becomes success, the library would be installed in the directory `output/`.

## Test on it

There is a simple http server that exposes the `ReplciatedLog` methods through http APIs, which can be found in `output/bin/raft_server`.
To set up a local cluster of 3 nodes, you need only to run the command:

```sh
bash run.sh start_onebox --server_count 3
```

After that, there will be 3 raftserver processes running in the background, each in one of the "127.0.0.1:12321", "127.0.0.1:12322", "127.0.0.1:12323".

If you want to know the status of the RSM node in "127.0.0.1:12321", just send GET request to this address:

```
http "127.0.0.1:12321/info" # using httpie
```

This will get result like:

```
Access-Control-Allow-Origin: *
Content-Length: 71
Content-Type: text/html; charset=UTF-8

{
    "commitIndex": 1,
    "currentLeader": 3,
    "currentTerm": 1,
    "logIndex": 1
}
```

## License

consensus-yaraft is under the Apache 2.0 license. See the LICENSE file for details.
