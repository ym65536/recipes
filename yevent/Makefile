LIB_SRC = event_loop.cc epoller.cc channel.cc timer/timer.cc timer/timer_queue.cc \
					event_loop_thread.cc inet_address.cc socket.cc acceptor.cc \
					tcp_connection.cc tcp_server.cc buffer.cc

BINARIES = test1 test2 test3 test4 test5 test6 test7 test8

all: $(BINARIES)

include reactor.mk

test1: ./test/test1.cc 
test2: ./test/test2.cc
test3: ./test/test3.cc
test4: ./test/test4.cc
test5: ./test/test5.cc
test6: ./test/test6.cc
test7: ./test/test7.cc
test8: ./test/test8.cc
