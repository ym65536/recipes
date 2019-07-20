LIB_SRC = event_loop.cc epoller.cc channel.cc timer/timer.cc timer/timer_queue.cc

BINARIES = test1 test2 test3 test4 test5

all: $(BINARIES)

include reactor.mk

test1: ./test/test1.cc 
test2: ./test/test2.cc
test3: ./test/test3.cc
test4: ./test/test4.cc
test5: ./test/test5.cc
