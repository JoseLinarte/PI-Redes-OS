#include "Cliente.h"
#include "Intermediario.h"
#include "Servidor.h"
#include "Buzon.h"
#include "CentralBuzones.h"
#include "SistemaArchivos.h"
#include "Utilidades.h"
#include "Mensaje.h"
#include "Bitacora.h"
#include "IslaSimulada.h"
#include "RedSimulada.h"
#include "RegistroIslas.h"
#include "ProtocoloGrupal.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

// Lista de islas remotas simuladas
using ListaIslas = std::vector<std::unique_ptr<IslaSimulada>>;

// Método que inicializa las bodegas y productos provisionales para las pruebas
void inicializarBodegas(SistemaArchivos &sistemaArchivos) {
  // Crea la bodega provisional para las pruebas
  sistemaArchivos.crear_bodega("principal");
  // Carga algunos productos de prueba, organizados por categoría
  sistemaArchivos.insertar_producto("principal", "granos", "garbanzos", "50", "600");
  sistemaArchivos.insertar_producto("principal", "granos", "arroz", "50", "1000");
  sistemaArchivos.insertar_producto("principal", "condimentos", "aceite_vegetal", "25", "1000");
  sistemaArchivos.insertar_producto("principal", "condimentos", "sal", "25", "200");
}

// Crea las islas simuladas con datos de prueba y las conecta a la red
static void crearIslasSimuladas(ListaIslas &islas, RedSimulada &red, Bitacora &bitacora) {
  islas.push_back(std::make_unique<IslaSimulada>("INT_01", "172.16.123.17", PUERTO_GRUPAL, "bodegas/Isla01.data", bitacora));
  islas[0]->agregarProducto("ropa", "camisa", 10, "15.50");
  islas[0]->agregarProducto("ropa", "pantalon", 5, "25.00");
  islas[0]->agregarProducto("calzado", "tenis", 8, "40.00");

  islas.push_back(std::make_unique<IslaSimulada>("INT_04", "172.16.123.68", PUERTO_GRUPAL, "bodegas/Isla04.data", bitacora));
  islas[1]->agregarProducto("reposteria", "pastel_choco", 5, "8.50");
  islas[1]->agregarProducto("reposteria", "flan", 10, "3.20");
  islas[1]->agregarProducto("bebidas", "jugo_naranja", 20, "2.00");

  // Conecta cada isla a la red simulada y la pone a ejecutar
  for (auto &isla : islas) {
    red.conectar(isla.get());
    isla->iniciar();
  }
}

// Busca una isla simulada por su id; retorna nullptr si no existe
static IslaSimulada *buscarIsla(ListaIslas &islas, const std::string &id) {
  for (auto &isla : islas) {
    if (isla->id() == id) return isla.get();
  }
  return nullptr;
}

// Casos de prueba automáticos
void casosAutomaticos(Cliente &cliente, Mensaje &mensaje) {
  // Caso 1: BuscarProducto - existe
  mensaje.comando = Comando::BuscarProducto;
  strcpy(mensaje.contenido, "garbanzos");
  cliente.iniciar(mensaje);
  cliente.detener();

  // Caso 2: BuscarProducto - no existe
  mensaje.comando = Comando::BuscarProducto;
  strcpy(mensaje.contenido, "cantonés");
  cliente.iniciar(mensaje);
  cliente.detener();

  // Caso 3: Categorias - hay categorías registradas
  mensaje.comando = Comando::Categorias;
  strcpy(mensaje.contenido, "");
  cliente.iniciar(mensaje);
  cliente.detener();

  // Caso 4: Productos - categoría con productos, en una bodega específica
  mensaje.comando = Comando::Productos;
  strcpy(mensaje.contenido, "condimentos:principal");
  cliente.iniciar(mensaje);
  cliente.detener();

  // Caso 5: Productos - categoría inexistente
  mensaje.comando = Comando::Productos;
  strcpy(mensaje.contenido, "higiene:principal");
  cliente.iniciar(mensaje);
  cliente.detener();

  // Caso 6: Existencia - con bodega indicada, producto existe
  mensaje.comando = Comando::Existencia;
  strcpy(mensaje.contenido, "garbanzos:principal");
  cliente.iniciar(mensaje);
  cliente.detener();

  // Caso 7: Existencia - sin bodega indicada, producto existe
  mensaje.comando = Comando::Existencia;
  strcpy(mensaje.contenido, "sal");
  cliente.iniciar(mensaje);
  cliente.detener();

  // Caso 8: Existencia - producto no existe en ninguna bodega
  mensaje.comando = Comando::Existencia;
  strcpy(mensaje.contenido, "leche");
  cliente.iniciar(mensaje);
  cliente.detener();

  // Caso 9: Filtrar - hay coincidencia
  mensaje.comando = Comando::Filtrar;
  strcpy(mensaje.contenido, "condimentos:acei:principal");
  cliente.iniciar(mensaje);
  cliente.detener();

  // Caso 10: Filtrar - sin coincidencia
  mensaje.comando = Comando::Filtrar;
  strcpy(mensaje.contenido, "condimentos:xyz:principal");
  cliente.iniciar(mensaje);
  cliente.detener();

  // Caso 11: CrearProducto - bodega existente
  mensaje.comando = Comando::CrearProducto;
  strcpy(mensaje.contenido, "frijoles:granos:principal:40:700");
  cliente.iniciar(mensaje);
  cliente.detener();

  // Caso 12: CrearProducto - bodega inexistente
  mensaje.comando = Comando::CrearProducto;
  strcpy(mensaje.contenido, "leche:lacteos:secundaria:10:100");
  cliente.iniciar(mensaje);
  cliente.detener();

  // Caso 13: ActualizarProducto - producto existente
  mensaje.comando = Comando::ActualizarProducto;
  strcpy(mensaje.contenido, "garbanzos:principal:60:650");
  cliente.iniciar(mensaje);
  cliente.detener();

  // Caso 14: ActualizarProducto - producto no existe en esa bodega
  mensaje.comando = Comando::ActualizarProducto;
  strcpy(mensaje.contenido, "inexistente:principal:10:10");
  cliente.iniciar(mensaje);
  cliente.detener();

  // Caso 15: EliminarProducto - producto existente
  mensaje.comando = Comando::EliminarProducto;
  strcpy(mensaje.contenido, "aceite_vegetal:principal");
  cliente.iniciar(mensaje);
  cliente.detener();

  // Caso 16: EliminarProducto - producto no existe
  mensaje.comando = Comando::EliminarProducto;
  strcpy(mensaje.contenido, "noexiste:principal");
  cliente.iniciar(mensaje);
  cliente.detener();

  // Caso 17: Comando no manejado por el Servidor (cae en mensajeInvalido)
  mensaje.comando = Comando::Conectar;
  strcpy(mensaje.contenido, "");
  cliente.iniciar(mensaje);
  cliente.detener();
}

// Pide un dato al usuario por consola
static std::string pedir(const std::string &etiqueta) {
  std::cout << etiqueta << ": " << std::flush;
  std::string linea;
  std::getline(std::cin, linea);
  return linea;
}

// Une campos con ':' descartando los vacíos del final (campos opcionales)
static std::string unir(std::vector<std::string> campos) {
  while (!campos.empty() && campos.back().empty()) campos.pop_back();
  std::string resultado;
  for (size_t i = 0; i < campos.size(); i++) {
    if (i > 0) resultado += ":";
    resultado += campos[i];
  }
  return resultado;
}

// Envía una solicitud por medio del Cliente y espera su respuesta impresa
static void enviar(Cliente &cliente, Comando comando, const std::string &contenido) {
  Mensaje mensaje;
  mensaje.idCliente = 1;
  mensaje.comando = comando;
  copiarContenido(mensaje.contenido, TAMANIO_MAXIMO, contenido);
  cliente.iniciar(mensaje);
  cliente.detener();
}

// Entrega un mensaje 60/61 al registro (simula que llegó por broadcast)
static void difundir(RegistroIslas &registro, const std::string &mensaje) {
  std::cout << "[broadcast] " << mensaje << std::endl;
  std::string resultado = registro.procesarAnuncio(mensaje);
  std::cout << (resultado.empty() ? "Mensaje ignorado (inválido, duplicado o eco)" : resultado) << std::endl;
}

static void mostrarMenu() {
  std::cout << "\n===== TicAmazon - " << ID_ISLA << " =====\n"
            << "-- Inventario local (protocolo interno) --\n"
            << " 1. Buscar producto\n"
            << " 2. Listar categorías\n"
            << " 3. Listar productos de una categoría\n"
            << " 4. Consultar existencia\n"
            << " 5. Filtrar productos\n"
            << " 6. Crear producto\n"
            << " 7. Actualizar producto\n"
            << " 8. Eliminar producto\n"
            << "-- Otras islas (protocolo mancomunado) --\n"
            << " 10. Ver islas conocidas\n"
            << " 11. Una isla emite ANNOUNCE (60)\n"
            << " 12. Una isla emite ANNOUNCE_BYE (61)\n"
            << " 13. Consultar categorías de una isla (10)\n"
            << " 14. Consultar productos de una isla (20)\n"
            << " 15. Reservar producto en una isla (50)\n"
            << " 16. Comprar en una isla (40)\n"
            << " 0. Salir\n";
}

// Menú interactivo
void menuInteractivo(Cliente &cliente, RegistroIslas &registro, ListaIslas &islas) {
  while (std::cin.good()) {
    mostrarMenu();
    std::string opcion = pedir("Opción");
    if (!std::cin.good() || opcion == "0") break;

    if (opcion == "1") {
      enviar(cliente, Comando::BuscarProducto, pedir("Producto"));
    } else if (opcion == "2") {
      enviar(cliente, Comando::Categorias, "");
    } else if (opcion == "3") {
      enviar(cliente, Comando::Productos, unir({pedir("Categoría"), pedir("Bodega (opcional)")}));
    } else if (opcion == "4") {
      enviar(cliente, Comando::Existencia, unir({pedir("Producto"), pedir("Bodega (opcional)")}));
    } else if (opcion == "5") {
      enviar(cliente, Comando::Filtrar, unir({pedir("Categoría"), pedir("Criterio"), pedir("Bodega (opcional)")}));
    } else if (opcion == "6") {
      enviar(cliente, Comando::CrearProducto,
             unir({pedir("Producto"), pedir("Categoría"), pedir("Bodega"), pedir("Cantidad"), pedir("Precio")}));
    } else if (opcion == "7") {
      enviar(cliente, Comando::ActualizarProducto,
             unir({pedir("Producto"), pedir("Bodega"), pedir("Cantidad"), pedir("Precio")}));
    } else if (opcion == "8") {
      enviar(cliente, Comando::EliminarProducto, unir({pedir("Producto"), pedir("Bodega")}));
    } else if (opcion == "10") {
      std::vector<InfoIsla> conocidas = registro.listar();
      if (conocidas.empty()) std::cout << "No hay islas registradas todavía\n";
      for (const InfoIsla &i : conocidas) std::cout << i.id << "  " << i.ip << ":" << i.puerto << "\n";
    } else if (opcion == "11" || opcion == "12") {
      std::string id = pedir("Isla que emite (INT_01, INT_04)");
      IslaSimulada *isla = buscarIsla(islas, id);
      if (!isla) { std::cout << "Esa isla no existe en la simulación\n"; continue; }
      difundir(registro, opcion == "11" ? isla->crearAnuncio() : isla->crearDespedida());
    } else if (opcion == "13") {
      enviar(cliente, Comando::CategoriasIsla, pedir("Id de isla"));
    } else if (opcion == "14") {
      enviar(cliente, Comando::ProductosIsla, unir({pedir("Id de isla"), pedir("Categoría")}));
    } else if (opcion == "15") {
      enviar(cliente, Comando::ReservarIsla, unir({pedir("Id de isla"), pedir("Producto"), pedir("Cantidad")}));
    } else if (opcion == "16") {
      std::string id = pedir("Id de isla (recuerde reservar antes)");
      std::string items;
      while (true) {
        std::string producto = pedir("Producto (vacío para terminar)");
        if (producto.empty() || !std::cin.good()) break;
        std::string cantidad = pedir("Cantidad");
        if (!items.empty()) items += ";";
        items += producto + "," + cantidad;
      }
      enviar(cliente, Comando::ComprarIsla, unir({id, items}));
    } else {
      std::cout << "Opción inválida\n";
    }
  }
}

void test(bool interactivo) {
  // Crea el buzón compartido de solicitudes de cualquier cliente hacia el intermediario
  Buzon buzonHaciaIntermediario;
  // Crea los buzones fijos entre el intermediario y el servidor
  Buzon buzonHaciaServidor;
  Buzon buzonDesdeServidor;
  // Crea la central que direcciona las respuestas al buzón de cada cliente
  CentralBuzones centralBuzones;
  // Registro de islas descubiertas y red simulada entre intermediarios
  RegistroIslas registroIslas;
  RedSimulada red;

  // Inicializa el sistema de archivos, partiendo siempre limpio para pruebas
  SistemaArchivos sistemaArchivos;
  sistemaArchivos.crear_archivo(RUTA_ALMACENAMIENTO);
  // Prepara las bodegas y productos de prueba
  inicializarBodegas(sistemaArchivos);

  // Inicializa el mensaje a procesar
  Mensaje mensaje;
  mensaje.idCliente = 1;

  // Inicializa la bitácora y las entidades
  Bitacora bitacora(RUTA_BITACORA);
  Cliente cliente(buzonHaciaIntermediario, centralBuzones, bitacora);
  Intermediario intermediario(buzonHaciaIntermediario, buzonHaciaServidor, buzonDesdeServidor, centralBuzones, registroIslas, red, bitacora);
  Servidor servidor(buzonDesdeServidor, buzonHaciaServidor, sistemaArchivos, bitacora);

  // Islas remotas simuladas (cada una con su propio hilo e inventario)
  ListaIslas islas;
  crearIslasSimuladas(islas, red, bitacora);

  // Comienza la ejecución
  intermediario.iniciar();
  servidor.iniciar();

  if (interactivo) {
    // Al arrancar, las islas anuncian su presencia
    for (auto &isla : islas) difundir(registroIslas, isla->crearAnuncio());
    menuInteractivo(cliente, registroIslas, islas);
  } else {
    casosAutomaticos(cliente, mensaje);
  }

  // Se detienen las entidades
  servidor.detener();
  intermediario.detener();
  for (auto &isla : islas) isla->detener();
  // Cierra el sistema de archivos
  sistemaArchivos.cerrar_archivo();
}

int main(int argc, char *argv[]) {
  bool interactivo = !(argc > 1 && std::string(argv[1]) == "--auto");
  test(interactivo);
  return 0;
}