CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

all: servidor_productos servidor_intermediario

servidor_productos: servidor_productos.cpp protocolo.h
	$(CXX) $(CXXFLAGS) -o servidor_productos servidor_productos.cpp

servidor_intermediario: servidor_intermediario.cpp protocolo.h
	$(CXX) $(CXXFLAGS) -o servidor_intermediario servidor_intermediario.cpp

clean:
	rm -f servidor_productos servidor_intermediario

listar-colas:
	ipcs -q
