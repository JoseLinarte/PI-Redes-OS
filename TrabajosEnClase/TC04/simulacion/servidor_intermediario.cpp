

#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "protocolo.h"

using namespace std;

int main() {
    cout << "[INTERMEDIARIO] Iniciando servidor intermediario (pid=" << getpid() << ")\n";
    int colaSolicitudes = msgget(CLAVE_COLA_SOLICITUDES, IPC_CREAT | 0666);
    int colaRespuestas  = msgget(CLAVE_COLA_RESPUESTAS,  IPC_CREAT | 0666);

    if (colaSolicitudes == -1 || colaRespuestas == -1) {
        perror("[INTERMEDIARIO] Error accediendo a las colas de mensajes");
        return 1;
    }
    vector<pair<string, int>> pedidos = {
        {"teclado", 3},
        {"mouse", 5},
        {"monitor", 10},  
        {"laptop", 2},
        {"cable_red", 20},
        {"producto_inexistente", 1}
    };

    int idActual = 1;

    for (const auto& pedido : pedidos) {
        MensajeSolicitud solicitud{};
        solicitud.mtype = TIPO_SOLICITUD;
        solicitud.idSolicitud = idActual;
        strncpy(solicitud.producto, pedido.first.c_str(), TAM_NOMBRE_PRODUCTO - 1);
        solicitud.cantidadSolicitada = pedido.second;
        solicitud.pidIntermediario = getpid();

        cout << "[INTERMEDIARIO] -> Enviando solicitud #" << idActual
             << ": producto=\"" << pedido.first
             << "\" cantidad=" << pedido.second << "\n";

        if (msgsnd(colaSolicitudes, &solicitud, sizeof(solicitud) - sizeof(long), 0) == -1) {
            perror("[INTERMEDIARIO] Error enviando solicitud");
            break;
        }
        MensajeRespuesta respuesta;
        if (msgrcv(colaRespuestas, &respuesta, sizeof(respuesta) - sizeof(long),
                   TIPO_RESPUESTA, 0) == -1) {
            perror("[INTERMEDIARIO] Error recibiendo respuesta");
            break;
        }

        cout << "[INTERMEDIARIO] <- Respuesta #" << respuesta.idSolicitud
             << " recibida: disponible=" << (respuesta.disponible ? "SI" : "NO")
             << " cantidad=" << respuesta.cantidadDisponible << "\n\n";

        idActual++;
        sleep(1); 
    }
    MensajeSolicitud fin{};
    fin.mtype = TIPO_SOLICITUD;
    fin.idSolicitud = SENAL_FIN;
    fin.cantidadSolicitada = 0;
    fin.pidIntermediario = getpid();
    strncpy(fin.producto, "FIN", TAM_NOMBRE_PRODUCTO - 1);

    cout << "[INTERMEDIARIO] -> Enviando señal de fin al servidor de productos\n";
    msgsnd(colaSolicitudes, &fin, sizeof(fin) - sizeof(long), 0);

    cout << "[INTERMEDIARIO] Fin del programa.\n";
    return 0;
}
