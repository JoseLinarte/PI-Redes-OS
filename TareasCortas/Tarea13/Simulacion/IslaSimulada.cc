#include "IslaSimulada.h"
#include "Mensaje.h"
#include "ProtocoloGrupal.h"
#include "Utilidades.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <map>

static int aEntero(const std::string &texto) {
  try { return std::stoi(texto); } catch (...) { return 0; }
}

static double aDecimal(const std::string &texto) {
  try { return std::stod(texto); } catch (...) { return 0.0; }
}

static std::string dosDecimales(double valor) {
  char bufer[32];
  std::snprintf(bufer, sizeof(bufer), "%.2f", valor);
  return bufer;
}

static std::string error(const std::string &codigo) { return "02/" + codigo; }

IslaSimulada::IslaSimulada(const std::string &id, const std::string &ip, int puerto, const std::string &ruta, Bitacora &bitacora) :
    idIsla(id), ipIsla(ip), puertoIsla(puerto), bitacora(bitacora), contadorAnuncios(0) {
  this->sistemaArchivos.crear_archivo(ruta);
  this->sistemaArchivos.crear_bodega("principal");
}

IslaSimulada::~IslaSimulada() { detener(); }

void IslaSimulada::agregarProducto(const std::string &categoria, const std::string &producto, int cantidad, const std::string &precio) {
  this->sistemaArchivos.insertar_producto("principal", categoria, producto, std::to_string(cantidad), precio);
}

void IslaSimulada::iniciar() {
  this->bitacora.registrarAccion(this->idIsla, "inicialización");
  this->hilo = std::thread(&IslaSimulada::ejecutar, this);
}

void IslaSimulada::detener() {
  if (!this->hilo.joinable()) return;
  Mensaje fin;
  fin.idCliente = FIN;
  fin.comando = Comando::Fin;
  std::strcpy(fin.contenido, "TERMINAR");
  this->buzonEntrada.encolar(fin);
  this->hilo.join();
  this->sistemaArchivos.cerrar_archivo();
  this->bitacora.registrarAccion(this->idIsla, "detención");
}

std::string IslaSimulada::crearAnuncio() {
  char idMsg[8];
  std::snprintf(idMsg, sizeof(idMsg), "%04d", this->contadorAnuncios++ % 10000);
  // ANNOUNCE: 60/ID_MSG/id_isla/ip/puerto
  return "60/" + std::string(idMsg) + "/" + this->idIsla + "/" + this->ipIsla + "/" + std::to_string(this->puertoIsla);
}

std::string IslaSimulada::crearDespedida() const {
  // ANNOUNCE_BYE: 61/id_isla
  return "61/" + this->idIsla;
}

std::string IslaSimulada::atender(const std::string &solicitud) {
  std::lock_guard<std::mutex> bloquear(this->candadoConexion);
  Mensaje entrada;
  entrada.idCliente = 0;
  entrada.comando = Comando::Enviar;
  copiarContenido(entrada.contenido, TAMANIO_MAXIMO, solicitud);
  this->buzonEntrada.encolar(entrada);
  return this->buzonSalida.desencolar().contenido;
}

void IslaSimulada::ejecutar() {
  while (true) {
    Mensaje solicitud = this->buzonEntrada.desencolar();
    if (solicitud.idCliente == FIN) break;
    this->bitacora.registrarAccion(this->idIsla, "solicitud recibida [" + std::string(solicitud.contenido) + "]");
    Mensaje respuesta;
    respuesta.idCliente = solicitud.idCliente;
    respuesta.comando = Comando::Enviar;
    copiarContenido(respuesta.contenido, TAMANIO_MAXIMO, this->procesar(solicitud.contenido));
    this->buzonSalida.encolar(respuesta);
    this->bitacora.registrarAccion(this->idIsla, "respuesta enviada");
  }
}

std::string IslaSimulada::procesar(const std::string &solicitud) {
  std::vector<std::string> campos = Grupal::dividir(solicitud, '/');
  if (!Grupal::esTipo(campos[0])) return error(Grupal::ERR_MAL_FORMADO);
  const std::string &tipo = campos[0];

  if (tipo == "10") {
    if (campos.size() != 1) return error(Grupal::ERR_MAL_FORMADO);
    return this->atenderCategorias();
  }
  if (tipo == "20") {
    if (campos.size() != 2) return error(Grupal::ERR_MAL_FORMADO);
    if (!Grupal::esCategoria(campos[1])) return error(Grupal::ERR_VALOR_INVALIDO);
    return this->atenderProductos(aMinusculas(campos[1]));
  }
  if (tipo == "50") {
    if (campos.size() != 3) return error(Grupal::ERR_MAL_FORMADO);
    if (!Grupal::esProducto(campos[1]) || !Grupal::esCantidad(campos[2])) return error(Grupal::ERR_VALOR_INVALIDO);
    return this->atenderReserva(aMinusculas(campos[1]), campos[2]);
  }
  if (tipo == "40") {
    if (campos.size() != 3) return error(Grupal::ERR_MAL_FORMADO);
    return this->atenderCompra(campos[1], campos[2]);
  }
  return error(Grupal::ERR_TIPO_NO_SOPORTADO);
}

std::string IslaSimulada::atenderCategorias() {
  std::vector<std::string> categorias;
  for (const std::string &bodega : this->sistemaArchivos.listar_bodegas()) {
    for (const ProductoTexto &p : this->sistemaArchivos.listar_productos(bodega)) {
      std::string cat = aMinusculas(p.categoria);
      if (std::find(categorias.begin(), categorias.end(), cat) == categorias.end()) categorias.push_back(cat);
    }
  }
  std::string respuesta = "01/";
  for (size_t i = 0; i < categorias.size(); i++) {
    if (i > 0) respuesta += ",";
    respuesta += categorias[i];
  }
  return respuesta;
}

std::string IslaSimulada::atenderProductos(const std::string &categoria) {
  std::string items;
  int cantidad = 0;
  for (const std::string &bodega : this->sistemaArchivos.listar_bodegas()) {
    for (const ProductoTexto &p : this->sistemaArchivos.listar_productos(bodega)) {
      if (aMinusculas(p.categoria) != categoria) continue;
      if (cantidad > 0) items += ";";
      items += aMinusculas(p.producto) + "," + p.precio + "," + p.cantidad;
      cantidad++;
    }
  }
  if (cantidad == 0) return "03/" + categoria;
  return "01/" + std::to_string(cantidad) + "/" + items;
}

bool IslaSimulada::buscarProducto(const std::string &nombre, ProductoTexto &encontrado) {
  for (const std::string &bodega : this->sistemaArchivos.listar_bodegas()) {
    for (const ProductoTexto &p : this->sistemaArchivos.listar_productos(bodega)) {
      if (aMinusculas(p.producto) == nombre) { encontrado = p; return true; }
    }
  }
  return false;
}

void IslaSimulada::purgarReservas() {
  auto ahora = std::chrono::steady_clock::now();
  this->reservas.erase(
    std::remove_if(this->reservas.begin(), this->reservas.end(),
                   [&](const Reserva &r) { return r.expira <= ahora; }),
    this->reservas.end());
}

int IslaSimulada::cantidadReservada(const std::string &producto) {
  int total = 0;
  for (const Reserva &r : this->reservas) if (r.producto == producto) total += r.cantidad;
  return total;
}

std::string IslaSimulada::atenderReserva(const std::string &producto, const std::string &cantidadTexto) {
  int cantidad = aEntero(cantidadTexto);
  if (cantidad <= 0) return error(Grupal::ERR_VALOR_INVALIDO);

  this->purgarReservas();
  ProductoTexto p;
  if (!this->buscarProducto(producto, p)) return "03/" + producto;
  // Lo disponible es el stock menos lo ya reservado
  int disponible = aEntero(p.cantidad) - this->cantidadReservada(producto);
  if (cantidad > disponible) return error(Grupal::ERR_STOCK_INSUFICIENTE);

  this->reservas.push_back({producto, cantidad,
    std::chrono::steady_clock::now() + std::chrono::seconds(SEGUNDOS_RESERVA)});
  return "01";
}

std::string IslaSimulada::atenderCompra(const std::string &n, const std::string &itemsTexto) {
  if (!Grupal::esCount(n)) return error(Grupal::ERR_VALOR_INVALIDO);

  // Interpreta la lista "producto,cantidad;producto,cantidad;..."
  struct Item { std::string producto; int cantidad; };
  std::vector<Item> items;
  std::map<std::string, int> pedido; // total pedido por producto
  for (const std::string &texto : Grupal::dividir(itemsTexto, ';')) {
    std::vector<std::string> c = Grupal::dividir(texto, ',');
    if (c.size() != 2) return error(Grupal::ERR_MAL_FORMADO);
    if (!Grupal::esProducto(c[0]) || !Grupal::esCantidad(c[1]) || aEntero(c[1]) <= 0) return error(Grupal::ERR_VALOR_INVALIDO);
    items.push_back({aMinusculas(c[0]), aEntero(c[1])});
    pedido[items.back().producto] += items.back().cantidad;
  }
  if (static_cast<int>(items.size()) != aEntero(n)) return error(Grupal::ERR_MAL_FORMADO);

  this->purgarReservas();

  std::map<std::string, ProductoTexto> datos;
  for (const auto &par : pedido) {
    ProductoTexto p;
    if (!this->buscarProducto(par.first, p)) return "03/" + par.first;
    if (this->cantidadReservada(par.first) < par.second) return error(Grupal::ERR_SIN_RESERVA);
    if (aEntero(p.cantidad) < par.second) return error(Grupal::ERR_STOCK_INSUFICIENTE);
    datos[par.first] = p;
  }

  for (const auto &par : pedido) {
    int restante = par.second;
    for (auto it = this->reservas.begin(); it != this->reservas.end() && restante > 0;) {
      if (it->producto != par.first) { ++it; continue; }
      int tomado = std::min(restante, it->cantidad);
      it->cantidad -= tomado;
      restante -= tomado;
      if (it->cantidad == 0) it = this->reservas.erase(it); else ++it;
    }
    const ProductoTexto &p = datos[par.first];
    int nuevoStock = aEntero(p.cantidad) - par.second;
    this->sistemaArchivos.extraer_producto(p.bodega, p.producto);
    this->sistemaArchivos.insertar_producto(p.bodega, p.categoria, p.producto, std::to_string(nuevoStock), p.precio);
  }

  double total = 0;
  std::string detalle;
  for (size_t i = 0; i < items.size(); i++) {
    double subtotal = aDecimal(datos[items[i].producto].precio) * items[i].cantidad;
    total += subtotal;
    if (i > 0) detalle += ";";
    detalle += items[i].producto + "," + std::to_string(items[i].cantidad) + "," + dosDecimales(subtotal);
  }
  return "01/" + dosDecimales(total) + "/" + std::to_string(items.size()) + "/" + detalle;
}