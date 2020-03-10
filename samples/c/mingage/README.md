# Mingage

Mingage is a minimal Engage-powered application that serves to demonstrate some of the basics of the Engage API and use of the Engine.

## Building

Mingage is built using a simple make file so all you should need to do it to run the make command:

```shell
$ make
g++ -c -Wno-psabi -Wall -std=c++11 -fPIC  -DNDEBUG -O3 -I. -I../../../api/c/include Mingage.cpp  -o Mingage.o
warning: unknown warning option '-Wno-psabi' [-Wunknown-warning-option]
1 warning generated.
g++ -c -Wno-psabi -Wall -std=c++11 -fPIC  -DNDEBUG -O3 -I. -I../../../api/c/include WorkQueue.cpp  -o WorkQueue.o
warning: unknown warning option '-Wno-psabi' [-Wunknown-warning-option]
1 warning generated.
g++ -Wno-psabi -Wall -std=c++11 -fPIC  -DNDEBUG -O3 -I. -I../../../api/c/include -o mingage Mingage.o WorkQueue.o -L../../../bin/1.121.8880/darwin_x64 -lengage-shared -lpthread -lstdc++
Building mingage with Engage version latest
```

>Your compiler might produce warnings for the "***-Wno-psabi***" option.  This can be expected when compiling for non-ARM platforms and can generally be ignored.

The current version of Engage to link with is "***latest***" which is a subdirectory of ***bin*** in this repository.  This value is defined in the make file as:

```shell
ENGAGE_VER ?= latest
```

## Running

Once mingage is built, execute it by running:

```shell
$ export LD_LIBRARY_PATH=<path to libengage-shared.so / libengage-shared.dylib>
$ ./mingage
```

Once launched, mingage loads its configurations from JSON files in the "***cfg***" directory.  The files there are for the active engine policy, active mission, and active identity.  If mingage is operating in Rallypoint-connection mode, it will also load the active rallypoint file.  (The default, though, it to run in multicast mode).

## User Interface

Mingage has a very simple command-line interface.  All user interaction is via the keyboard with all commands consisting of a single character.  Help can be obtained by entering "***?***" or "***h***".

```shell
mingage > ?
quit/q    ......................... quit
status/s  ......................... show status
next/n    ......................... next channel
txon/t    ......................... transmit on
txoff/x   ......................... transmit off
rp/r      ......................... switch to rallypoint connection
mc/m      ......................... switch to multicast connection
```

## Architecture

Mingage is VERY simple.  It does very little error-checking and is certainly not tuned for performance.  However, this simplicity should make it easy for you to get going on writing C++ apps for Engage.  Some things to consider though ...

### Threading and WorkQueue

Inside the Engage Engine, we do all our work on a variety of worker threads - resulting in a mostly asynchronous, non-blocking setup.  This setup results in event notifications coming up from Engage on a different thread than calls made into the Engine.  While Engage guarantees that all event callbacks will always be on the same thread, just the mere fact that these callbacks are coming "up" on a different thread can be problematic for the application.

So, we've added the "***WorkQueue***" C++ class to this project that allows mingage to perform its operations on a single, main thread.  This means that there's no need to protect data shared by threads with critical sections and mutexes as all operations happen on one thread (and they're queued behind each other).  

While ***WorkQueue*** is a stripped-down version of the more robust and feature-rich TaskExecutor we use inside Engage; it is still very efficient, works great for most purposes on most plaforms and is free for you to use in your own code.


## Logging

Engage produces a LOT of debug output and, while great to look at sometimes, can become quite annoying.  To reduce the amount of output, you can use the "***engageSetLogLevel***" API or, even more simply, set an environment variable to the required level.  

Levels are as follows:
|Level|Name|Description|
|-|-|-|
|0|FATAL|Big problems have happened|
|1|ERROR|(Mostly) recoverable errors|
|2|WARNING|Something's not right|
|3|INFORMATIONAL|You might find this genuinely interesting|
|4|DEBUG|TMI|


For example to set in the environment to errors and fatals only:

```shell
$ export ENGAGE_LOG_LEVEL=1
```
