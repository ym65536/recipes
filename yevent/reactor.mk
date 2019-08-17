CXXFLAGS = -O0 -g  -Wall -Werror -I. -I.. -pthread -I./base/ -I./timer/
LDFLAGS = -lpthread
BASE_SRC = ./base/logging.cc ./base/log_stream.cc ./base/thread.cc ./base/timestamp.cc

$(BINARIES):
	g++ -std=c++11 $(CXXFLAGS) -o $@ $(LIB_SRC) $(BASE_SRC) $(filter %.cc,$^) $(LDFLAGS)

clean:
	rm -f $(BINARIES) core

