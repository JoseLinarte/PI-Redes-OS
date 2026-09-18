# SSLSocket - Servidor SSL (fork/hilos) + Cliente
#

CXX := g++
CXXFLAGS := -g -Wall -std=c++11
LDFLAGS := -lssl -lcrypto -lpthread
DEPS := Socket.h VSocket.h SSLSocket.h
TARGETS := ThreadSSLServer.out ForkSSLServer.out SSLClient.out

all: $(TARGETS)

%.o: %.cc $(DEPS)
	$(CXX) $(CXXFLAGS) -c $<

ThreadSSLServer.out: ThreadSSLServer.o SSLSocket.o VSocket.o Socket.o
	$(CXX) ThreadSSLServer.o SSLSocket.o VSocket.o Socket.o $(LDFLAGS) -o $@

ForkSSLServer.out: ForkSSLServer.o SSLSocket.o VSocket.o Socket.o
	$(CXX) ForkSSLServer.o SSLSocket.o VSocket.o Socket.o $(LDFLAGS) -o $@

SSLClient.out: SSLClient.o SSLSocket.o VSocket.o Socket.o
	$(CXX) SSLClient.o SSLSocket.o VSocket.o Socket.o $(LDFLAGS) -o $@

clean:
	rm -f *.o *.out