#ifndef ISLASIMULADA_H
#define ISLASIMULADA_H

#include "Bitacora.h"
#include "Buzon.h"
#include "SistemaArchivos.h"
#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class IslaSimulada {
  public:
    IslaSimulada(const std::string &id, const std::string &ip, int puerto, const std::string &ruta, Bitacora &);
    ~IslaSimulada();
    void agregarProducto(const std::string &categoria, const std::string &producto, int cantidad, const std::string &precio);
    void iniciar();
    void detener();
    std::string crearAnuncio();
    std::string crearDespedida() const;
    std::string atender(const std::string &solicitud);

    const std::string &id() const { return idIsla; }
    const std::string &ip() const { return ipIsla; }
    int puerto() const { return puertoIsla; }

  private:
    // Reserva temporal de stock
    struct Reserva {
      std::string producto;
      int cantidad;
      std::chrono::steady_clock::time_point expira;
    };

    std::string idIsla;
    std::string ipIsla;
    int puertoIsla;
    Bitacora &bitacora;
    SistemaArchivos sistemaArchivos;
    Buzon buzonEntrada;
    Buzon buzonSalida;
    // Evita que dos conexiones mezclen sus respuestas en el buzón de salida
    std::mutex candadoConexion;
    // Solo lo toca el hilo de la isla
    std::vector<Reserva> reservas;
    int contadorAnuncios;
    std::thread hilo;

    void ejecutar();
    std::string procesar(const std::string &solicitud);
    std::string atenderCategorias();
    std::string atenderProductos(const std::string &categoria);
    std::string atenderReserva(const std::string &producto, const std::string &cantidad);
    std::string atenderCompra(const std::string &n, const std::string &items);
    bool buscarProducto(const std::string &nombre, ProductoTexto &encontrado);
    void purgarReservas();
    int cantidadReservada(const std::string &producto);
};

#endif