

#include <iostream>
#include <map>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "protocolo.h"

using namespace std;

int main() {
    cout << "[PRODUCTOS] Iniciando servidor de productos (pid=" << getpid() << ")\n";
    map<string, int> inventario = {
        {"teclado",   15},
        {"mouse",     30},
        {"monitor",    8},
        {"laptop",     5},
        {"cable_red", 100}
    };

    cout << "[PRODUCTOS] Inventario inicial:\n";
    for (const auto& item : inventario) {
        cout << "    - " << item.first << ": " << item.second << " unidades\n";
    }
    int colaSolicitudes = msgget(CLAVE_COLA_SOLICITUDES, IPC_CREAT | 0666);
    int colaRespuestas  = msgget(CLAVE_COLA_RESPUESTAS,  IPC_CREAT | 0666);
    if (colaSolicitudes == -1 || colaRespuestas == -1) {
        perror("[PRODUCTOS] Error creando colas de mensajes");
        return 1;
    }
    cout << "[PRODUCTOS] Colas de mensajes listas. Esperando solicitudes...\n\n";

    while (true) {
        MensajeSolicitud solicitud;
        ssize_t leido = msgrcv(colaSolicitudes, &solicitud, sizeof(solicitud) - sizeof(long),TIPO_SOLICITUD, 0);
        if (leido == -1) {
            perror("[PRODUCTOS] Error recibiendo solicitud");
            break;
        }
        if (solicitud.idSolicitud == SENAL_FIN) {
            cout << "[PRODUCTOS] Señal de fin recibida. Cerrando servidor.\n";
            break;
        }
        cout << "[PRODUCTOS] <- Solicitud #" << solicitud.idSolicitud<< " recibida: producto=\"" << solicitud.producto<< "\" cantidad=" << solicitud.cantidadSolicitada<< " (de pid " << solicitud.pidIntermediario << ")\n";

        MensajeRespuesta respuesta{};
        respuesta.mtype = TIPO_RESPUESTA;
        respuesta.idSolicitud = solicitud.idSolicitud;
        strncpy(respuesta.producto, solicitud.producto, TAM_NOMBRE_PRODUCTO - 1);

        auto it = inventario.find(solicitud.producto);
        if (it != inventario.end() && it->second >= solicitud.cantidadSolicitada) {
            it->second -= solicitud.cantidadSolicitada;
            respuesta.disponible = 1;
            respuesta.cantidadDisponible = solicitud.cantidadSolicitada;
        } else {
            respuesta.disponible = 0;
            respuesta.cantidadDisponible = (it != inventario.end()) ? it->second : 0;
        }
        if (msgsnd(colaRespuestas, &respuesta, sizeof(respuesta) - sizeof(long), 0) == -1) {
            perror("[PRODUCTOS] Error enviando respuesta");
            break;
        }

        cout << "[PRODUCTOS] -> Respuesta #" << respuesta.idSolicitud<< " enviada: disponible=" << (respuesta.disponible ? "SI" : "NO")<< " cantidad=" << respuesta.cantidadDisponible << "\n\n";
    }
    msgctl(colaSolicitudes, IPC_RMID, nullptr);
    msgctl(colaRespuestas, IPC_RMID, nullptr);
    cout << "[PRODUCTOS] Colas de mensajes liberadas. Fin del programa.\n";

    return 0;
}
