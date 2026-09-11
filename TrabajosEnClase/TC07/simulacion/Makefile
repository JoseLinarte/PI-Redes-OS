# Bodegas - Servidor de productos (HTTP, hilos) + Cliente
#

CXX := g++
CXXFLAGS := -g -Wall -std=c++11
LDFLAGS := -lpthread
DEPS := Socket.h VSocket.h FileSystem.hpp
TARGETS := ServidorProductos.out Cliente.out

all: $(TARGETS)

%.o: %.cc $(DEPS)
	$(CXX) $(CXXFLAGS) -c $<

%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c $<

ServidorProductos.out: ServidorProductos.o VSocket.o Socket.o FileSystem.o
	$(CXX) ServidorProductos.o VSocket.o Socket.o FileSystem.o $(LDFLAGS) -o $@

Cliente.out: Cliente.o VSocket.o Socket.o
	$(CXX) Cliente.o VSocket.o Socket.o $(LDFLAGS) -o $@

clean:
	rm -f *.o *.out *.dat
