CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -pthread
LDFLAGS  := -pthread

BUILDDIR := build
BIN      := intermediario

FUENTES := main.cpp \
           Servidor.cpp Manejadorconexion.cpp Router.cpp \
           Httprequest.cpp Httpresponse.cpp Parserproforma.cpp \
           ServicioProductos.cpp PaginasHtml.cpp \
           Vsocket.cpp Socket.cpp SSLSocket.cpp \
           HttpCliente.cpp RegistroServidores.cpp ServicioProductosDistribuido.cpp Descubrimiento.cpp \
           ProtocoloIslas.cpp RegistroIslas.cpp DescubrimientoIslas.cpp \
           ServicioIslas.cpp ServidorIslas.cpp ConsultaIslas.cpp

OBJETOS := $(patsubst %.cpp,$(BUILDDIR)/%.o,$(FUENTES))

.PHONY: all clean run

all: $(BIN)

$(BIN): $(OBJETOS)
	$(CXX) $(OBJETOS) -o $@ $(LDFLAGS) -lssl -lcrypto

$(BUILDDIR)/%.o: %.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

run: $(BIN)
	./$(BIN) $(or $(PUERTO),8080)

clean:
	rm -rf $(BUILDDIR) $(BIN)

-include $(OBJETOS:.o=.d)