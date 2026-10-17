#include "Intermediario.h"
#include "ProtocoloGrupal.h"
#include <iostream>

Intermediario::Intermediario(Buzon &buzonDesdeCliente, Buzon &buzonHaciaServidor, Buzon &buzonDesdeServidor, CentralBuzones &centralBuzones, RegistroIslas &registroIslas, RedSimulada &red, Bitacora &bitacora) :
    buzonDesdeCliente(buzonDesdeCliente),
    buzonHaciaServidor(buzonHaciaServidor),
    buzonDesdeServidor(buzonDesdeServidor),
    centralBuzones(centralBuzones),
    registroIslas(registroIslas),
    red(red),
    bitacora(bitacora),
    estado(Estado::Esperando) {}

Intermediario::~Intermediario() {}

void Intermediario::iniciar() {
  // Registro de bitácora: inicialización
  this->bitacora.registrarAccion("Intermediario", "inicialización");
  // Inicializa y ejecuta el hilo del intermediario
  this->hilo = std::thread(&Intermediario::ejecutar, this);
}

void Intermediario::detener() {
  // Se crea un mensaje para finalizar el programa
  Mensaje fin;
  // Se declara la operación "FIN"
  fin.idCliente = FIN;
  // Se indica el comando
  fin.comando = Comando::Fin;
  // Se copia el contenido indicando el final
  strcpy(fin.contenido, "TERMINAR");
  // Se ingresa el mensaje en el buzón
  this->buzonDesdeCliente.encolar(fin);
  // Registro de bitácora: encolamiento de operación final
  this->bitacora.registrarAccion("Intermediario", "encolamiento de operación final");
  // Se cambia el estado del intermediario
  this->estado = Estado::Detenido;
  // Se finaliza el hilo de ejecución
  if (this->hilo.joinable()) {
    this->hilo.join();
  }
}

Mensaje Intermediario::procesarSolicitud(const Mensaje &mensaje) {
  // Copia base de la respuesta
  Mensaje respuesta = mensaje;
  // Variable local donde se arma el texto final que verá el cliente
  std::string textoFinal;
  // Se traduce el comando de resultado del Servidor al formato textual del protocolo
  if (mensaje.comando == Comando::Ok)
    textoFinal = "OK " + reemplazarSeparador(mensaje.contenido, ':');
  else if (mensaje.comando == Comando::OkCatalogo)
    textoFinal = "OK_CATALOGO";
  else if (mensaje.comando == Comando::Disponible)
    textoFinal = "DISPONIBLE " + reemplazarSeparador(mensaje.contenido, ':');
  else if (mensaje.comando == Comando::Si)
    textoFinal = "SI " + std::string(mensaje.contenido);
  else if (mensaje.comando == Comando::No)
    textoFinal = "NO";
  else if (mensaje.comando == Comando::Coincidencia)
    textoFinal = "COINCIDENCIA " + reemplazarSeparador(mensaje.contenido, ':');
  else if (mensaje.comando == Comando::SinCoincidencia)
    textoFinal = "SIN_COINCIDENCIA";
  else if (mensaje.comando == Comando::Bloqueado)
    textoFinal = "BLOQUEADO " + std::string(mensaje.contenido);
  else if (mensaje.comando == Comando::ErrorFormatoInvalido)
    textoFinal = "ERROR " + std::string(mensaje.contenido) + " FORMATO_INVALIDO";
  else if (mensaje.comando == Comando::ErrorSesionInvalida)
    textoFinal = "ERROR SESION_INVALIDA";
  else if (mensaje.comando == Comando::ErrorCategoriaInexistente)
    textoFinal = "ERROR " + std::string(mensaje.contenido) + " CATEGORIA_INEXISTENTE";
  else if (mensaje.comando == Comando::ErrorCategoriaExistente)
    textoFinal = "ERROR " + std::string(mensaje.contenido) + " CATEGORIA_EXISTENTE";
  else if (mensaje.comando == Comando::ErrorCategoriaNoVacia)
    textoFinal = "ERROR " + std::string(mensaje.contenido) + " CATEGORIA_NO_VACIA";
  else if (mensaje.comando == Comando::ErrorBodegaInexistente)
    textoFinal = "ERROR " + std::string(mensaje.contenido) + " BODEGA_INEXISTENTE";
  else if (mensaje.comando == Comando::ErrorProductoExistente)
    textoFinal = "ERROR " + std::string(mensaje.contenido) + " PRODUCTO_EXISTENTE";
  else if (mensaje.comando == Comando::ErrorNoDisponible)
    textoFinal = "ERROR " + std::string(mensaje.contenido) + " NO_DISPONIBLE";
  else if (mensaje.comando == Comando::ErrorPermisoDenegado)
    textoFinal = "ERROR PERMISO_DENEGADO";
  else if (mensaje.comando == Comando::ErrorRecursoBloqueado)
    textoFinal = "ERROR RECURSO_BLOQUEADO";
  else
    textoFinal = "ERROR FORMATO_INVALIDO";
  // Se copia el texto final al contenido del mensaje para el cliente
  strcpy(respuesta.contenido, textoFinal.c_str());
  return respuesta;
}

// Arma el mensaje del protocolo grupal a partir del contenido interno; retorna false si los datos no son válidos
static bool armarPeticion(Comando comando, const std::string &datos, std::string &peticion) {
  if (comando == Comando::CategoriasIsla) {
    peticion = "10";
    return true;
  }
  if (comando == Comando::ProductosIsla) {
    if (!Grupal::esCategoria(datos)) return false;
    peticion = "20/" + datos;
    return true;
  }
  if (comando == Comando::ReservarIsla) {
    // datos = producto:cantidad
    std::vector<std::string> c = separarCampos(datos, ':');
    if (c.size() != 2 || !Grupal::esProducto(c[0]) || !Grupal::esCantidad(c[1])) return false;
    peticion = "50/" + c[0] + "/" + c[1];
    return true;
  }
  if (comando == Comando::ComprarIsla) {
    // datos = producto,cantidad;producto,cantidad;...
    if (datos.empty()) return false;
    std::vector<std::string> items = Grupal::dividir(datos, ';');
    for (const std::string &item : items) {
      std::vector<std::string> c = Grupal::dividir(item, ',');
      if (c.size() != 2 || !Grupal::esProducto(c[0]) || !Grupal::esCantidad(c[1])) return false;
    }
    peticion = "40/" + std::to_string(items.size()) + "/" + datos;
    return true;
  }
  return false;
}

// Traduce la respuesta grupal (01/02/03) a texto legible para el cliente
static std::string interpretarRespuesta(Comando comando, const std::string &respuesta) {
  std::vector<std::string> campos = Grupal::dividir(respuesta, '/');
  if (campos[0] == "01") {
    if (comando == Comando::CategoriasIsla) {
      std::string lista = campos.size() > 1 ? campos[1] : "";
      return lista.empty() ? "CATEGORIAS NINGUNA" : "CATEGORIAS " + reemplazarSeparador(lista, ',');
    }
    if (comando == Comando::ProductosIsla && campos.size() >= 3) {
      std::string texto = "PRODUCTOS " + campos[1];
      for (const std::string &item : Grupal::dividir(campos[2], ';')) {
        std::vector<std::string> c = Grupal::dividir(item, ',');
        if (c.size() == 3) texto += "\n  " + c[0] + " precio=" + c[1] + " stock=" + c[2];
      }
      return texto;
    }
    if (comando == Comando::ReservarIsla) return "RESERVA_OK";
    if (comando == Comando::ComprarIsla && campos.size() >= 4) {
      std::string texto = "FACTURA total=" + campos[1] + " items=" + campos[2];
      for (const std::string &item : Grupal::dividir(campos[3], ';')) {
        std::vector<std::string> c = Grupal::dividir(item, ',');
        if (c.size() == 3) texto += "\n  " + c[0] + " x" + c[1] + " = " + c[2];
      }
      return texto;
    }
    return "ERROR RESPUESTA_INVALIDA";
  }
  if (campos[0] == "02" && campos.size() >= 2) {
    if (campos[1] == "100") return "ERROR MENSAJE_MAL_FORMADO";
    if (campos[1] == "101") return "ERROR TIPO_NO_SOPORTADO";
    if (campos[1] == "102") return "ERROR VALOR_INVALIDO";
    if (campos[1] == "103") return "ERROR STOCK_INSUFICIENTE";
    if (campos[1] == "104") return "ERROR SIN_RESERVA_VIGENTE";
    return "ERROR CODIGO_DESCONOCIDO " + campos[1];
  }
  if (campos[0] == "03") return "NO_ENCONTRADO " + (campos.size() > 1 ? campos[1] : std::string());
  return "ERROR RESPUESTA_INVALIDA";
}

bool Intermediario::esComandoIsla(Comando comando) {
  return comando == Comando::CategoriasIsla || comando == Comando::ProductosIsla ||
         comando == Comando::ReservarIsla || comando == Comando::ComprarIsla;
}

Mensaje Intermediario::atenderIsla(const Mensaje &solicitud) {
  Mensaje respuesta = solicitud;
  respuesta.comando = Comando::Ok;
  // El contenido interno es "idIsla[:datos]"
  std::string contenido = solicitud.contenido;
  size_t pos = contenido.find(':');
  std::string idIsla = contenido.substr(0, pos);
  std::string datos = (pos == std::string::npos) ? "" : contenido.substr(pos + 1);

  std::string texto, peticion;
  InfoIsla isla;
  if (!this->registroIslas.obtener(idIsla, isla)) {
    texto = "ERROR " + idIsla + " ISLA_DESCONOCIDA";
  } else if (!armarPeticion(solicitud.comando, datos, peticion)) {
    texto = "ERROR FORMATO_INVALIDO";
  } else {
    std::string respuestaIsla;
    // Se muestra el intercambio del protocolo mancomunado para poder seguirlo en la simulación
    std::cout << "[" << ID_ISLA << " -> " << idIsla << "] " << peticion << std::endl;
    if (this->red.enviarSolicitud(isla.ip, isla.puerto, peticion, respuestaIsla)) {
      std::cout << "[" << idIsla << " -> " << ID_ISLA << "] " << respuestaIsla << std::endl;
      texto = interpretarRespuesta(solicitud.comando, respuestaIsla);
    } else {
      texto = "ERROR " + idIsla + " ISLA_NO_RESPONDE";
    }
  }
  copiarContenido(respuesta.contenido, TAMANIO_MAXIMO, texto);
  return respuesta;
}

void Intermediario::ejecutar() {
  // Variable local para guardar el mensaje a enviar
  Mensaje mensaje;
  // Se ejecuta mientras el estado del intermediario no sea detenido
  while (this->estado != Estado::Detenido) {
    // Se declara que el intermediario está esperando
    this->estado = Estado::Esperando;
    // Se recibe la solicitud de cualquier cliente
    Mensaje solicitud = this->buzonDesdeCliente.desencolar();
    // Registro de bitácora: desencolamiento de mensaje
    this->bitacora.registrarAccion("Intermediario", "desencolamiento de mensaje");
    // Se declara que el intermediario está procesando la solicitud del cliente
    this->estado = Estado::Procesando;
    // Si se indica la operación "FIN", finaliza la operación actual
    if (solicitud.idCliente == FIN) { break; }
    if (esComandoIsla(solicitud.comando)) {
      Mensaje respuestaIsla = atenderIsla(solicitud);
      this->bitacora.registrarAccion("Intermediario", "proceso de solicitud a otra isla");
      this->centralBuzones.encolar(respuestaIsla.idCliente, respuestaIsla);
      this->bitacora.registrarAccion("Intermediario", "encolamiento de mensaje");
      continue;
    }
    // Se encola el mensaje de solicitud para el servidor
    this->buzonHaciaServidor.encolar(solicitud);
    // Registro de bitácora: encolamiento de mensaje
    this->bitacora.registrarAccion("Intermediario", "encolamiento de mensaje");
    // Se declara que el intermediario está esperando
    this->estado = Estado::Esperando;
    // Recibe la respuesta desde el servidor
    Mensaje respuesta = this->buzonDesdeServidor.desencolar();
    // Registro de bitácora: desencolamiento de mensaje
    this->bitacora.registrarAccion("Intermediario", "desencolamiento de mensaje");
    // Se declara que el intermediario está procesando la solicitud del cliente
    this->estado = Estado::Procesando;
    // Se crea la respuesta para el cliente
    mensaje = procesarSolicitud(respuesta);
    // Registro de bitácora: proceso de solicitud
    this->bitacora.registrarAccion("Intermediario", "proceso de solicitud");
    // Se encola la respuesta en el buzón del cliente correspondiente
    this->centralBuzones.encolar(mensaje.idCliente, mensaje);
    // Registro de bitácora: encolamiento de mensaje
    this->bitacora.registrarAccion("Intermediario", "encolamiento de mensaje");
  }
}
