# Makefile del intermediario - TicAmazon
CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -pthread
LDFLAGS  := -pthread

BUILDDIR := build

BIN_INTERMEDIARIO := intermediario

FUENTES_INTERMEDIARIO := main.cpp \
                         ManejadorIntermediario.cpp \
                         Httprequest.cpp \
                         Vsocket.cpp Socket.cpp

OBJETOS_INTERMEDIARIO := $(patsubst %.cpp,$(BUILDDIR)/%.o,$(FUENTES_INTERMEDIARIO))

.PHONY: all objetos clean run

all: $(BIN_INTERMEDIARIO)

$(BIN_INTERMEDIARIO): $(OBJETOS_INTERMEDIARIO)
	$(CXX) $(OBJETOS_INTERMEDIARIO) -o $@ $(LDFLAGS)

objetos: $(OBJETOS_INTERMEDIARIO)
	@echo "El intermediario compila sin errores."

$(BUILDDIR)/%.o: %.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

run: $(BIN_INTERMEDIARIO)
	./$(BIN_INTERMEDIARIO) $(or $(PUERTO),8081) $(or $(HOST_PRODUCTOS),127.0.0.1) $(or $(PUERTO_PRODUCTOS),8080)

clean:
	rm -rf $(BUILDDIR) $(BIN_INTERMEDIARIO)