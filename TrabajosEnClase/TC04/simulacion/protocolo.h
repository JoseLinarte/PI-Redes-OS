#ifndef PROTOCOLO_H
#define PROTOCOLO_H

#include <sys/types.h>

const key_t CLAVE_COLA_SOLICITUDES = 0x5000;
const key_t CLAVE_COLA_RESPUESTAS  = 0x5001;

const int TAM_NOMBRE_PRODUCTO = 50;

// Tipos de mensaje (mtype) usados en cada cola
const long TIPO_SOLICITUD = 1;
const long TIPO_RESPUESTA = 2;

// idSolicitud reservado para indicarle al servidor de productos ue debe finalizar y liberar los recursos de IPC.
const int SENAL_FIN = -1;

struct MensajeSolicitud {
    long mtype;                         // TIPO_SOLICITUD
    int  idSolicitud;                   // identificador de la solicitud
    char producto[TAM_NOMBRE_PRODUCTO]; // nombre del producto pedido
    int  cantidadSolicitada;            // cantidad pedida
    int  pidIntermediario;              
};

struct MensajeRespuesta {
    long mtype;                         // TIPO_RESPUESTA
    int  idSolicitud;                   // debe calzar con la solicitud original
    char producto[TAM_NOMBRE_PRODUCTO]; // nombre del producto
    int  cantidadDisponible;            // cantidad realmente entregadadisponible
    int  disponible;                    
};

#endif // PROTOCOLO_H
