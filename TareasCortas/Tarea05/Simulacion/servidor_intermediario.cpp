
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "protocolo.h"

using namespace std;

using ListaPedidos = vector<pair<string, int>>;

void ejecutarHijo(int numeroSolicitante, const ListaPedidos& pedidos) {
    pid_t miPid = getpid();

    int colaSolicitudes = msgget(CLAVE_COLA_SOLICITUDES, IPC_CREAT | 0666);
    int colaRespuestas  = msgget(CLAVE_COLA_RESPUESTAS,  IPC_CREAT | 0666);
    if (colaSolicitudes == -1 || colaRespuestas == -1) {
        perror("[HIJO] Error accediendo a las colas de mensajes");
        exit(1); 
    }

    cout << "[INTERMEDIARIO][hijo " << numeroSolicitante << ", pid=" << miPid<< "] iniciando con " << pedidos.size() << " pedido(s)\n";
    int idLocal = 1;
    for (const auto& pedido : pedidos) {
        MensajeSolicitud solicitud{};
        solicitud.mtype = TIPO_SOLICITUD;
        solicitud.idSolicitud = idLocal;
        strncpy(solicitud.producto, pedido.first.c_str(), TAM_NOMBRE_PRODUCTO - 1);
        solicitud.cantidadSolicitada = pedido.second;
        solicitud.pidSolicitante = miPid;

        cout << "[INTERMEDIARIO][hijo " << numeroSolicitante << "] -> solicitud #"<< idLocal << ": producto=\"" << pedido.first<< "\" cantidad=" << pedido.second << "\n";

        if (msgsnd(colaSolicitudes, &solicitud, sizeof(solicitud) - sizeof(long), 0) == -1) {
            perror("[HIJO] Error enviando solicitud");
            break;
        }

        MensajeRespuesta respuesta;
        if (msgrcv(colaRespuestas, &respuesta, sizeof(respuesta) - sizeof(long),
                   miPid, 0) == -1) {
            perror("[HIJO] Error recibiendo respuesta");
            break;
        }
        cout << "[INTERMEDIARIO][hijo " << numeroSolicitante << "] <- respuesta #"<< respuesta.idSolicitud << ": disponible="<< (respuesta.disponible ? "SI" : "NO")<< " cantidad=" << respuesta.cantidadDisponible << "\n";

        idLocal++;
        usleep(300000); // 0.3s, simula tiempo de procesamiento del cliente
    }

    cout << "[INTERMEDIARIO][hijo " << numeroSolicitante << ", pid=" << miPid
         << "] terminó todos sus pedidos.\n";
    exit(0); // exit() (no _exit()) para que se vacíe el buffer de cout
}

int main() {
    pid_t pidPadre = getpid();
    cout << "[INTERMEDIARIO][padre, pid=" << pidPadre << "] iniciando\n";

    // Asegura que las colas existan antes de que los hijos las usen
    int colaSolicitudes = msgget(CLAVE_COLA_SOLICITUDES, IPC_CREAT | 0666);
    int colaRespuestas  = msgget(CLAVE_COLA_RESPUESTAS,  IPC_CREAT | 0666);
    if (colaSolicitudes == -1 || colaRespuestas == -1) {
        perror("[INTERMEDIARIO] Error accediendo a las colas de mensajes");
        return 1;
    }

    vector<ListaPedidos> solicitantes = {
        { {"teclado", 3}, {"mouse", 5} },
        { {"monitor", 10}, {"laptop", 2} },          
        { {"cable_red", 20}, {"producto_inexistente", 1} }
    };

    vector<pid_t> hijos;

    for (size_t i = 0; i < solicitantes.size(); i++) {
        cout.flush();
        pid_t pid = fork();
        if (pid < 0) {
            perror("[INTERMEDIARIO] Error en fork()");
            continue;
        }
        if (pid == 0) {
            ejecutarHijo(static_cast<int>(i + 1), solicitantes[i]);
        }
        hijos.push_back(pid);
    }
    for (pid_t pidHijo : hijos) {
        int estado;
        waitpid(pidHijo, &estado, 0);
        cout << "[INTERMEDIARIO][padre] hijo pid=" << pidHijo << " finalizó\n";
    }
    MensajeSolicitud fin{};
    fin.mtype = TIPO_SOLICITUD;
    fin.idSolicitud = SENAL_FIN;
    fin.cantidadSolicitada = 0;
    fin.pidSolicitante = pidPadre;
    strncpy(fin.producto, "FIN", TAM_NOMBRE_PRODUCTO - 1);
    cout << "[INTERMEDIARIO][padre] todos los hijos terminaron, enviando señal de fin\n";
    msgsnd(colaSolicitudes, &fin, sizeof(fin) - sizeof(long), 0);
    cout << "[INTERMEDIARIO][padre] fin del programa.\n";
    return 0;
}
