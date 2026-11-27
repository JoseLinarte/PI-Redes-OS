#include <cctype>
#include <cstdio>
#include <sstream>
#include <iomanip>
#include "PaginasHtml.hpp"

std::string PaginasHtml::Escapar( const std::string & texto ) {
   std::string r;
   for ( char c : texto ) {
      switch ( c ) {
         case '&':  r += "&amp;";  break;
         case '<':  r += "&lt;";   break;
         case '>':  r += "&gt;";   break;
         case '"':  r += "&quot;"; break;
         default:   r += c;
      }
   }
   return r;
}

std::string PaginasHtml::CodificarUrl( const std::string & texto ) {
   std::ostringstream out;
   for ( unsigned char c : texto ) {
      if ( isalnum( c ) || c == '-' || c == '_' || c == '.' || c == '~' ) {
         out << c;
      } else {
         out << '%' << std::uppercase << std::hex << std::setw( 2 ) << std::setfill( '0' ) << (int)c;
      }
   }
   return out.str();
}

// con punto decimal porque el cliente lo lee con std::stod
std::string PaginasHtml::Decimal( double valor ) {
   char buf[64];
   snprintf( buf, sizeof( buf ), "%.2f", valor );
   return buf;
}

static const char * ESTILO = R"CSS(<style><!--
html{background:#e3ecf0}
body{font-family:Segoe UI,Arial,sans-serif;color:#13262f;max-width:980px;margin:0 auto;padding:0 24px 70px;min-height:100vh;position:relative;background-color:#f3f7f9;background-image:linear-gradient(rgba(20,90,110,.08) 1px,transparent 1px),linear-gradient(90deg,rgba(20,90,110,.08) 1px,transparent 1px);background-size:26px 26px;border-left:6px solid #0f3b4c;border-right:6px solid #0f3b4c}
nav{display:flex;flex-wrap:wrap;gap:8px;justify-content:flex-end;padding:14px 0 12px}
nav a{text-decoration:none;color:#0f3b4c;font-weight:600;font-size:.88em;padding:7px 15px;border-radius:20px;border:2px solid #b9d2db;background:#fff;transition:all .15s}
nav a::before{content:attr(data-icono) '  ' attr(data-t)}
nav a:hover{background:#0f3b4c;color:#fff;border-color:#0f3b4c}
h1{background:linear-gradient(135deg,#0b2d3a,#145a6e 70%,#1d7590);color:#fff;margin:0 0 22px;padding:18px 22px;border-radius:10px;font-size:1.5em;border-bottom:5px solid #e0a526;box-shadow:0 3px 8px rgba(0,0,0,.25)}
h1::before{content:'\1F6F0  '}
h1::after{content:'\25CF  EN LINEA';float:right;font-size:.55em;letter-spacing:2px;margin-top:9px;color:#ffd166;animation:latido 1.6s infinite}
@keyframes latido{0%,100%{opacity:1}50%{opacity:.3}}
table{width:100%;border-collapse:collapse;background:#fff;border-radius:10px;overflow:hidden;box-shadow:0 2px 6px rgba(0,0,0,.18);border-top:4px solid #e0a526;margin-bottom:22px}
th{background:#145a6e;color:#fff;text-align:left;padding:10px 12px;font-size:.82em;letter-spacing:1px;text-transform:uppercase}
td{padding:9px 12px;border-bottom:1px solid #dbe7ec}
tr:nth-child(even) td{background:#f1f7f9}
tr:hover td{background:#fff3d1}
td.num{text-align:right;font-variant-numeric:tabular-nums}
td.org{font-weight:700;color:#145a6e}
td.org::before{content:'\1F4CD ';font-size:.9em}
td.cat{text-transform:capitalize}
td.on{color:#1b7a43;font-weight:700;letter-spacing:1px}
td.on::before{content:'\25CF  ';animation:latido 1.6s infinite}
td.off{color:#a12a2a;font-weight:700;letter-spacing:1px}
td.off::before{content:'\25CB  '}
td.dir{font-family:Consolas,monospace;color:#0f3b4c}
span.chip{display:inline-block;background:#e4f1f5;color:#0f3b4c;border:1px solid #b9d2db;border-radius:14px;padding:3px 11px;margin:2px 4px 2px 0;font-size:.85em;font-weight:600}
span.sep{display:none}
ul{list-style:none;padding:0;display:grid;grid-template-columns:repeat(auto-fill,minmax(190px,1fr));gap:14px}
li a{display:block;position:relative;background:#fff;padding:22px 14px 18px;border-radius:12px;text-align:center;text-decoration:none;color:#0f3b4c;font-weight:600;border:2px solid #b9d2db;border-top:6px solid #e0a526;box-shadow:0 2px 5px rgba(0,0,0,.12);transition:all .15s;text-transform:capitalize}
li a::before{content:'\1F4E6';display:block;font-size:1.8em;margin-bottom:6px}
li a[href*='aliment' i]::before{content:'\1F35E'}
li a[href*='bebida' i]::before{content:'\1F964'}
li a[href*='salud' i]::before{content:'\1F48A'}
li a[href*='electr' i]::before,li a[href*='tecno' i]::before{content:'\1F4F1'}
li a[href*='hogar' i]::before{content:'\1F3E0'}
li a[href*='ropa' i]::before{content:'\1F455'}
li a[href*='deport' i]::before{content:'\26BD'}
li a[href*='jugue' i]::before{content:'\1F9F8'}
li a[href*='libro' i]::before{content:'\1F4DA'}
li a[href*='herram' i]::before{content:'\1F527'}
li a[href*='mascota' i]::before{content:'\1F43E'}
li a::after{content:'\2192';position:absolute;right:14px;bottom:10px;opacity:0;transition:opacity .15s}
li a:hover{background:#0f3b4c;color:#fff;border-color:#0b2d3a;transform:translateY(-3px)}
li a:hover::after{opacity:1}
ul.errores{display:block}
ul.errores li{background:#fff;border-left:5px solid #a12a2a;padding:9px 14px;border-radius:6px;margin-bottom:8px}
a.volver{display:inline-block;margin-bottom:14px;color:#145a6e;font-weight:700;text-decoration:none;border-bottom:2px solid #e0a526}
a.volver:hover{color:#0b2d3a}
p{font-size:1.05em;background:#fff;border-left:5px solid #e0a526;padding:10px 14px;border-radius:6px;box-shadow:0 1px 3px rgba(0,0,0,.1)}
p.total{font-size:1.3em;text-align:right;border-left:0;border-right:6px solid #e0a526;color:#0b2d3a}
p a,td a{color:#145a6e;font-weight:600}
div.resumen{display:grid;grid-template-columns:repeat(auto-fit,minmax(260px,1fr));gap:18px}
table.eventos td{background:#0e2630;color:#d4e4ea;font-family:Consolas,monospace;font-size:.85em;border-bottom:1px solid #1c3c48}
table.eventos tr:nth-child(even) td{background:#122e3a}
table.eventos tr:hover td{background:#1b4252}
td.tipo{font-weight:700}
table.eventos td.t-ERROR{color:#ff7b72}
table.eventos td.t-INACTIVO{color:#f0a35e}
table.eventos td.t-ACTIVO,table.eventos td.t-REGISTRO{color:#7ee2a8}
table.eventos td.t-CONSULTA{color:#79c0ff}
table.eventos td.t-PROFORMA{color:#ffd166}
table.eventos td.t-INFO{color:#c9b8ff}
body::after{content:'TicAmazon  \2022  Centro de distribucion  \2022  Intermediario';position:absolute;left:0;right:0;bottom:0;height:44px;line-height:44px;text-align:center;background:#0b2d3a;color:#bcd7e0;font-size:.85em;letter-spacing:2px;border-top:4px solid #e0a526}
--></style>
)CSS";

static std::string Lista( const std::vector<std::string> & elementos ) {
   std::string r;
   for ( size_t i = 0; i < elementos.size(); i++ ) {
      if ( i > 0 ) r += "<span class=\"sep\">, </span>";
      r += "<span class=\"chip\">" + PaginasHtml::Escapar( elementos[i] ) + "</span>";
   }
   return r;
}

std::string PaginasHtml::Inicio( const std::string & titulo ) {
   std::string nav = "<nav><a href=\"/TicAmazon/list.php\" data-icono=\"&#x1F5C2;\" data-t=\"Categorias\"></a><a href=\"/TicAmazon/servidores\" data-icono=\"&#x1F5A5;\" data-t=\"Servidores\"></a><a href=\"/TicAmazon/rutas\" data-icono=\"&#x1F5FA;\" data-t=\"Rutas\"></a><a href=\"/TicAmazon/bitacora\" data-icono=\"&#x1F4DC;\" data-t=\"Bitacora\"></a></nav>\n";
   return std::string( "<!DOCTYPE html>\n<HTML>\n<HEAD><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"><TITLE>" ) + Escapar( titulo ) + "</TITLE>\n" + ESTILO + "</HEAD>\n<BODY>\n" + nav + "<H1>" + Escapar( titulo ) + "</H1>\n";
}

std::string PaginasHtml::Fin() {
   return "</BODY>\n</HTML>\n";
}

std::string PaginasHtml::PaginaCategorias( const std::vector<std::string> & categorias ) {
   std::string html = Inicio( "TicAmazon - Categorias" );
   html += "<UL>\n";
   for ( const std::string & c : categorias ) {
      html += "<LI><A href=\"/TicAmazon/list.php?category=" + CodificarUrl( c ) + "\">" + Escapar( c ) + "</A></LI>\n";
   }
   html += "</UL>\n";
   return html + Fin();
}

std::string PaginasHtml::PaginaProductos( const std::string & categoria, const std::vector<Producto> & productos ) {
   std::string html = Inicio( "TicAmazon - " + categoria );
   html += "<a class=\"volver\" href=\"/TicAmazon/list.php\">&larr; Volver a categorias</a>\n";
   html += "<TABLE>\n";
   html += "<TR><TH>Intermediario</TH><TH>Bodega</TH><TH>Categoria</TH><TH>Descripcion</TH><TH>Cantidad</TH><TH>Precio</TH></TR>\n";
   for ( const Producto & p : productos ) {
      html += "<TR><TD class=\"org\"></TD>";
      html += "<TD>" + Escapar( p.Bodega() ) + "</TD>";
      html += "<TD class=\"cat\">" + Escapar( p.Categoria() ) + "</TD>";
      html += "<TD>" + Escapar( p.Descripcion() ) + "</TD>";
      html += "<TD class=\"num\">" + std::to_string( p.Cantidad() ) + "</TD>";
      html += "<TD class=\"num\">" + Decimal( p.Precio() ) + "</TD></TR>\n";
   }
   html += "</TABLE>\n";
   return html + Fin();
}

std::string PaginasHtml::PaginaProforma( const Proforma & pf ) {
   std::string html = Inicio( "TicAmazon - Factura proforma" );
   if ( !pf.EsValida() ) {
      html += "<P>No se pudo generar la proforma:</P>\n<UL class=\"errores\">\n";
      for ( const std::string & e : pf.Errores() ) {
         html += "<LI>" + Escapar( e ) + "</LI>\n";
      }
      html += "</UL>\n";
      return html + Fin();
   }
   html += "<TABLE>\n";
   html += "<TR><TH>Bodega</TH><TH>Categoria</TH><TH>Descripcion</TH><TH>Cantidad</TH><TH>Precio</TH><TH>Subtotal</TH></TR>\n";
   for ( const LineaProforma & l : pf.Lineas() ) {
      html += "<TR><TD>" + Escapar( l.bodega ) + "</TD>";
      html += "<TD class=\"cat\">" + Escapar( l.categoria ) + "</TD>";
      html += "<TD>" + Escapar( l.descripcion ) + "</TD>";
      html += "<TD class=\"num\">" + std::to_string( l.cantidad ) + "</TD>";
      html += "<TD class=\"num\">" + Decimal( l.precioUnitario ) + "</TD>";
      html += "<TD class=\"num\">" + Decimal( l.Subtotal() ) + "</TD></TR>\n";
   }
   html += "</TABLE>\n<P class=\"total\"><B>TOTAL: " + Decimal( pf.Total() ) + "</B></P>\n";
   return html + Fin();
}

std::string PaginasHtml::PaginaError( int codigo, const std::string & detalle ) {
   return Inicio( "Error " + std::to_string( codigo ) ) + "<P>" + Escapar( detalle ) + "</P>\n" + Fin();
}

std::string PaginasHtml::PaginaMensaje( const std::string & titulo, const std::string & texto ) {
   return Inicio( titulo ) + "<P>" + Escapar( texto ) + "</P>\n" + Fin();
}

std::string PaginasHtml::PaginaServidores( const std::vector<InfoServidor> & servidores ) {
   std::string html = Inicio( "TicAmazon - Servidores registrados" );
   html += "<TABLE>\n";
   html += "<TR><TH>Nombre</TH><TH>Direccion</TH><TH>Estado</TH><TH>Categorias</TH></TR>\n";
   for ( const InfoServidor & s : servidores ) {
      html += "<TR><TD class=\"org\">" + Escapar( s.nombre ) + "</TD>";
      html += "<TD class=\"dir\">" + Escapar( s.host ) + ":" + std::to_string( s.puerto ) + "</TD>";
      html += std::string( s.activo ? "<TD class=\"on\">ACTIVO</TD>" : "<TD class=\"off\">INACTIVO</TD>" );
      html += "<TD>" + Lista( s.categorias ) + "</TD></TR>\n";
   }
   html += "</TABLE>\n";
   return html + Fin();
}

std::string PaginasHtml::PaginaRutas( const std::vector<std::pair<std::string, std::vector<std::string>>> & rutas ) {
   std::string html = Inicio( "TicAmazon - Tabla de rutas" );
   html += "<TABLE>\n<TR><TH>Categoria</TH><TH>Servidores</TH></TR>\n";
   for ( const auto & r : rutas ) {
      html += "<TR><TD class=\"cat\"><a href=\"/TicAmazon/list.php?category=" + CodificarUrl( r.first ) + "\">" + Escapar( r.first ) + "</a></TD><TD>" + Lista( r.second ) + "</TD></TR>\n";
   }
   html += "</TABLE>\n";
   return html + Fin();
}

std::string PaginasHtml::PaginaProductosDistribuidos( const std::string & categoria, const std::vector<FilaProducto> & filas ) {
   std::string html = Inicio( "TicAmazon - " + categoria );
   html += "<a class=\"volver\" href=\"/TicAmazon/list.php\">&larr; Volver a categorias</a>\n";
   html += "<TABLE>\n";
   html += "<TR><TH>Intermediario</TH><TH>Bodega</TH><TH>Categoria</TH><TH>Descripcion</TH><TH>Cantidad</TH><TH>Precio</TH></TR>\n";
   for ( const FilaProducto & f : filas ) {
      html += "<TR><TD class=\"org\">" + Escapar( f.intermediario ) + "</TD>";
      html += "<TD>" + Escapar( f.bodega ) + "</TD>";
      html += "<TD class=\"cat\">" + Escapar( f.categoria ) + "</TD>";
      html += "<TD>" + Escapar( f.descripcion ) + "</TD>";
      html += "<TD class=\"num\">" + Escapar( f.cantidad ) + "</TD>";
      html += "<TD class=\"num\">" + Escapar( f.precio ) + "</TD></TR>\n";
   }
   html += "</TABLE>\n";
   return html + Fin();
}

std::string PaginasHtml::PaginaBitacora( const std::vector<EventoBitacora> & eventos, const ResumenBitacora & resumen, const std::string & filtroTipo ) {
   std::string html = Inicio( "TicAmazon - Bitacora" );
   html.insert( html.find( "</HEAD>" ), "<meta http-equiv=\"refresh\" content=\"3\">" );
   html += "<P>Total de eventos registrados: <B>" + std::to_string( resumen.total ) + "</B>";
   if ( !filtroTipo.empty() ) {
      html += " | Filtro: <B>" + Escapar( filtroTipo ) + "</B>";
   }
   html += " | <a href=\"/TicAmazon/bitacora\">Ver todos</a></P>\n";
   html += "<div class=\"resumen\">\n<TABLE>\n<TR><TH>Tipo</TH><TH>Cantidad</TH></TR>\n";
   for ( const auto & par : resumen.porTipo ) {
      html += "<TR><TD class=\"tipo\"><a href=\"/TicAmazon/bitacora?tipo=" + CodificarUrl( par.first ) + "\">" + Escapar( par.first ) + "</a></TD><TD class=\"num\">" + std::to_string( par.second ) + "</TD></TR>\n";
   }
   html += "</TABLE>\n<TABLE>\n<TR><TH>Origen</TH><TH>Cantidad</TH></TR>\n";
   for ( const auto & par : resumen.porOrigen ) {
      html += "<TR><TD class=\"org\">" + Escapar( par.first ) + "</TD><TD class=\"num\">" + std::to_string( par.second ) + "</TD></TR>\n";
   }
   html += "</TABLE>\n</div>\n";
   html += "<TABLE class=\"eventos\">\n<TR><TH>#</TH><TH>Fecha</TH><TH>Tipo</TH><TH>Origen</TH><TH>Mensaje</TH></TR>\n";
   for ( const EventoBitacora & e : eventos ) {
      html += "<TR><TD>" + std::to_string( e.id ) + "</TD>";
      html += "<TD>" + Escapar( e.fecha ) + "</TD>";
      html += "<TD class=\"tipo t-" + Escapar( e.tipo ) + "\">" + Escapar( e.tipo ) + "</TD>";
      html += "<TD>" + Escapar( e.origen ) + "</TD>";
      html += "<TD>" + Escapar( e.mensaje ) + "</TD></TR>\n";
   }
   html += "</TABLE>\n";
   return html + Fin();
}