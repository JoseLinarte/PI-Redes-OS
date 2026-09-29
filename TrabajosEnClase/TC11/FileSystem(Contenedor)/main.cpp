// main.cpp
#include <fstream>
#include <iostream>
#include <string>
#include "almacenamiento.hpp"

// Lee una linea completa (permite espacios, ej. "Alimentos y bebidas").
// Devuelve false si se llego al fin de la entrada (Ctrl+D).
static bool leer_linea(const std::string& mensaje, std::string& destino) {
    std::cout << mensaje;
    return static_cast<bool>(std::getline(std::cin, destino));
}

// El separador de campos es ':' y el de registros '\n', asi que no pueden
// aparecer dentro de un dato o se corrompe el registro.
static bool campo_valido(const std::string& campo) {
    if (campo.empty()) {
        std::cout << "El campo no puede estar vacio.\n";
        return false;
    }
    if (campo.find_first_of(":\n") != std::string::npos) {
        std::cout << "El campo no puede contener ':'.\n";
        return false;
    }
    return true;
}

static bool archivo_existe(const std::string& ruta) {
    std::ifstream f(ruta);
    return f.good();
}

static void imprimir_bodega(Almacenamiento& almacen, const std::string& nombre) {
    std::cout << "Productos en " << nombre << ":\n";
    auto productos = almacen.listar_productos(nombre);
    if (productos.empty()) {
        std::cout << "  (sin productos o la bodega no existe)\n";
    }
    for (auto& p : productos)
        std::cout << "  " << p.producto << " (" << p.categoria << ") cant=" << p.cantidad
                  << " precio=" << p.precio << "\n";
    std::cout << "\n";
}

static void listar_bodegas(Almacenamiento& almacen) {
    auto bodegas = almacen.listar_bodegas();
    std::cout << "Bodegas en el archivo:\n";
    if (bodegas.empty()) std::cout << "  (ninguna)\n";
    for (auto& nombre : bodegas)
        std::cout << "  " << nombre << "\n";
    std::cout << "\n";
}

static void menu() {
    std::cout << "===== Almacenamiento de bodegas =====\n"
              << " 1. Crear bodega\n"
              << " 2. Insertar producto\n"
              << " 3. Extraer producto\n"
              << " 4. Listar bodegas\n"
              << " 5. Listar productos de una bodega\n"
              << " 6. Listar todo\n"
              << " 0. Salir\n";
}

int main(int argc, char* argv[]) {
    std::string ruta = (argc > 1) ? argv[1] : "bodegas.data";
    Almacenamiento almacen;

    // Abrir el archivo si existe, o crearlo si no (nunca se borra sin avisar)
    if (archivo_existe(ruta)) {
        if (!almacen.abrir_archivo(ruta)) {
            std::cerr << "No se pudo abrir " << ruta << "\n";
            return 1;
        }
        std::cout << "Archivo existente abierto: " << ruta << "\n\n";
    } else {
        if (!almacen.crear_archivo(ruta)) {
            std::cerr << "No se pudo crear " << ruta << "\n";
            return 1;
        }
        std::cout << "Archivo nuevo creado: " << ruta << "\n\n";
    }

    std::string opcion;
    while (true) {
        menu();
        if (!leer_linea("Opcion: ", opcion)) break;
        std::cout << "\n";

        if (opcion == "0") {
            break;

        } else if (opcion == "1") {
            std::string bodega;
            if (!leer_linea("Nombre de la bodega (max 19 caracteres): ", bodega)) break;
            if (!campo_valido(bodega)) continue;
            if (almacen.crear_bodega(bodega))
                std::cout << "Bodega '" << bodega << "' creada.\n\n";
            else
                std::cout << "No se pudo crear (ya existe o el nombre es muy largo).\n\n";

        } else if (opcion == "2") {
            std::string bodega, categoria, producto, cantidad, precio;
            if (!leer_linea("Bodega: ", bodega)) break;
            if (!leer_linea("Categoria: ", categoria)) break;
            if (!leer_linea("Producto: ", producto)) break;
            if (!leer_linea("Cantidad: ", cantidad)) break;
            if (!leer_linea("Precio: ", precio)) break;

            if (!campo_valido(bodega) || !campo_valido(categoria) || !campo_valido(producto) ||
                !campo_valido(cantidad) || !campo_valido(precio)) continue;

            try {
                std::stoi(cantidad);
                std::stod(precio);
            } catch (const std::exception&) {
                std::cout << "Cantidad debe ser entero y precio un numero.\n\n";
                continue;
            }

            if (almacen.insertar_producto(bodega, categoria, producto, cantidad, precio))
                std::cout << "Producto insertado.\n\n";
            else
                std::cout << "No se pudo insertar (la bodega no existe o el registro es muy largo).\n\n";

        } else if (opcion == "3") {
            std::string bodega, producto;
            if (!leer_linea("Bodega: ", bodega)) break;
            if (!leer_linea("Producto a extraer: ", producto)) break;
            if (almacen.extraer_producto(bodega, producto))
                std::cout << "Producto '" << producto << "' extraido de " << bodega << ".\n\n";
            else
                std::cout << "No se encontro ese producto en esa bodega.\n\n";

        } else if (opcion == "4") {
            listar_bodegas(almacen);

        } else if (opcion == "5") {
            std::string bodega;
            if (!leer_linea("Bodega: ", bodega)) break;
            imprimir_bodega(almacen, bodega);

        } else if (opcion == "6") {
            listar_bodegas(almacen);
            for (auto& nombre : almacen.listar_bodegas())
                imprimir_bodega(almacen, nombre);

        } else {
            std::cout << "Opcion invalida.\n\n";
        }
    }

    almacen.cerrar_archivo();
    std::cout << "Hasta luego.\n";
    return 0;
}