
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread
TARGET   = tienda

SRCS = $(wildcard *.cc)
OBJS = $(SRCS:.cc=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Crea las carpetas que necesita el programa
dirs:
	mkdir -p bodegas bitacora

# Compila y ejecuta en modo interactivo
run: $(TARGET) dirs
	./$(TARGET)

# Compila y ejecuta los 17 casos automáticos
auto: $(TARGET) dirs
	./$(TARGET) --auto

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all dirs run auto clean