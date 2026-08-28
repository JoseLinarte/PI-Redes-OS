#ifndef PROTOCOLO_H
#define PROTOCOLO_H

#include <sys/types.h>

const key_t CLAVE_COLA_SOLICITUDES = 0x5000;
const key_t CLAVE_COLA_RESPUESTAS  = 0x5001;

const int TAM_NOMBRE_PRODUCTO = 50;

const long TIPO_SOLICITUD = 1;

const int SENAL_FIN = -1;


struct MensajeSolicitud {
    long mtype;                         
    int  idSolicitud;                   
    char producto[TAM_NOMBRE_PRODUCTO];
    int  cantidadSolicitada;            
    int  pidSolicitante;                
};


struct MensajeRespuesta {
    long mtype;                         
    int  idSolicitud;                   
    char producto[TAM_NOMBRE_PRODUCTO]; 
    int  cantidadDisponible;            
    int  disponible;                   
};

#endif // PROTOCOLO_H
