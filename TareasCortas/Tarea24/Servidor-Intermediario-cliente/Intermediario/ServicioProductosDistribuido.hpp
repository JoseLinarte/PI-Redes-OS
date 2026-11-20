#ifndef ServicioProductosDistribuido_hpp
#define ServicioProductosDistribuido_hpp

#include <string>
#include <vector>
#include "ServicioProductos.hpp"
#include "RegistroServidores.hpp"
#include "PaginasHtml.hpp"
#include "RegistroIslas.hpp"
#include <ctime>
#include <mutex>

struct ReservaIsla {
   std::string clave;
   std::string categoria;
   std::string descripcion;
   int cantidad = 0;
   std::time_t expira = 0;
};

class ServicioProductosDistribuido : public ServicioProductos {
   public:
      void AgregarSemilla( const std::string & host, int puerto );
      void IniciarMonitor( int segundos );     // hilo que sondea a todos cada N segundos
      void DescubrirServidor( const std::string & host, int puerto, const std::string & nombre );
      void IslaDescubierta( const std::string & id, const std::string & ip, int puerto );
      void IslaCaida( const std::string & id );
      std::string AtenderIsla( const std::string & mensaje ) const; 
      void IniciarHilosIslas( int miIsla, int segundos );  


      RespuestaServicio ListarCategorias() const override;
      RespuestaServicio ListarProductos( const std::string & categoria ) const override;
      RespuestaServicio GenerarProforma( const std::vector<SolicitudItem> & items ) const override;

      RespuestaServicio ListarServidores() const override;
      RespuestaServicio ListarRutas() const override;
      RespuestaServicio RegistrarServidor( const std::string & nombre, const std::string & host, int puerto,const std::vector<std::string> & categorias ) const override;
      RespuestaServicio ListarBitacora( const std::string & filtroTipo ) const override;
   private:
      void Sondear() const;
      bool SondearServidor( const InfoServidor & s ) const;
      bool ConsultarProductos( const InfoServidor & s, const std::string & categoria,std::vector<FilaProducto> & filas ) const;

      mutable RegistroServidores registro;     // tiene su propio mutex
      Proforma ComprarItems( const std::vector<SolicitudItem> & items ) const;
      bool BuscarProductoProtocolo( const std::string & producto, std::string & categoria, std::string & descripcion, int & stock ) const;
      std::string RespuestaCategorias() const;
      std::string RespuestaProductos( const std::string & categoria ) const;
      std::string RespuestaReserva( const std::string & producto, const std::string & cantidadTexto ) const;
      std::string RespuestaCompra( const std::vector<std::string> & p ) const;
      bool ConsultarIsla( const InfoIsla & isla, const std::string & mensaje, int intentos, std::string & respuesta ) const;
      bool ConsultarProductosIsla( const InfoIsla & isla, const std::string & categoria, std::vector<FilaProducto> & filas ) const;
      bool ExisteLocal( const SolicitudItem & item ) const;
      std::vector<InfoIsla> IslasDeCategoria( const std::string & categoria ) const;
      Proforma ComprarConIslas( const std::vector<SolicitudItem> & items ) const;
      mutable RegistroIslas islas;
      mutable std::mutex mtxReservas;
      mutable std::vector<ReservaIsla> reservas;
};

#endif