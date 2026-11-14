# TicAmazon

Cada componente tiene su carpeta y se compila y ejecuta por separado.

## 1. Servidor de productos

### Compilar con make

```bash
cd ServidorProductos
make
```

Genera el ejecutable `servidor`. Para borrar lo compilado: `make clean`.


### Ejecutar con make

```bash
make run
```

Usa estas variables, todas opcionales:

| Variable | Valor por defecto | Significado |
|---|---|---|
| PUERTO_PROD | 9090 | Puerto TCP del servidor |
| DATOS | bodegas.data | Archivo de datos |
| NOMBRE | PC1 | Nombre con el que se registra |

Si corre en la misma PC que el intermediario, usa otro puerto, porque el TCP 9090 lo usa el intermediario:

```bash
make run PUERTO_PROD=9091 DATOS=bodegas2.data NOMBRE=PC2
```

### Ejecutar sin make

```bash
./servidor [puerto] [archivo] [nombre] [broadcast] [ipIntermediario] [puertoIntermediario]
```

Todos los parámetros son opcionales y posicionales: para dar uno hay que dar los anteriores.

| Parámetro | Por defecto | Significado |
|---|---|---|
| puerto | 8080 | Puerto TCP (TLS) del servidor. Con el valor por defecto choca con el intermediario, así que conviene pasarlo siempre |
| archivo | bodegas.data | Archivo de almacenamiento. Si no existe, se crea con datos de ejemplo |
| nombre | productos-PUERTO | Nombre con el que se registra. No puede contener el carácter \| |
| broadcast | automático | Direcciones de broadcast separadas por coma, o `auto`. Automático usa las interfaces 172.16.123.x y 172.16.123.63 |
| ipIntermediario | ninguna | IP fija del intermediario, además del descubrimiento por broadcast |
| puertoIntermediario | 8080 | Puerto HTTP del intermediario, solo si se dio ipIntermediario |

Ejemplos:

```bash
./servidor 9091 bodegas.data PC1
./servidor 9091 bodegas.data PC1 127.255.255.255
./servidor 9091 bodegas.data PC1 auto 192.168.1.5 8080
```

Para detenerlo: Ctrl+C. Dos servidores en la misma PC necesitan puerto y archivo distintos.

## 2. Servidor intermediario

### Compilar con make

```bash
cd Intermediario
make
```

Genera el ejecutable `intermediario`. Para borrar lo compilado: `make clean`.


### Ejecutar con make

```bash
make run
make run PUERTO=8081
```

La única variable es `PUERTO` (por defecto 8080). Los demás parámetros usan sus valores por defecto.

### Ejecutar sin make

```bash
./intermediario [puerto] [broadcast] [host:puerto ...] [isla=N] [ip=A.B.C.D] [destinos=ip,ip] [publica=A.B.C.D]
```

Todos los parámetros son opcionales. El puerto va primero y el resto puede ir en cualquier orden.

| Parámetro | Por defecto | Significado |
|---|---|---|
| puerto | 8080 | Puerto HTTP para clientes |
| broadcast | automático | Broadcast para descubrir servidores de productos. Un argumento sin `:` y sin prefijo se toma como broadcast. Automático usa las interfaces 172.16.123.x y 172.16.123.63 |
| host:puerto | ninguno | Servidor de productos conocido de antemano. Se pueden dar varios |
| isla=N | 3 | Número de isla (1 a 6). Su identificador es INT_0N |
| ip=A.B.C.D | primera IP 172.16.123.x de la máquina | IP que se anuncia a las otras islas. Si no hay ninguna, no anuncia y solo escucha |
| destinos=ip,ip | broadcasts de las otras islas | Direcciones a las que anuncia. Sirve para pruebas |
| publica=A.B.C.D | IP 10.1.35.x detectada | IP donde atiende a los clientes. Si no hay, o con `publica=todas`, atiende en todas las interfaces |

Puertos fijos: TCP 8080 (clientes, configurable), TCP y UDP 9090 (otras islas) y UDP 7071 (servidores de productos). Si la PC tiene una IP 10.1.35.x, el HTTP deja de atender en 127.0.0.1; para probar local usa `publica=todas`.

Ejemplos:

```bash
./intermediario
./intermediario 8080 127.255.255.255
./intermediario 8080 192.168.1.10:9091
./intermediario 8080 isla=3 ip=172.16.123.50 publica=todas
```

Páginas de consulta desde el navegador: `/TicAmazon/list.php`, `/TicAmazon/servidores`, `/TicAmazon/rutas` y `/TicAmazon/bitacora`.

Para detenerlo: Ctrl+C. Al cerrar avisa a las otras islas.

## 3. Cliente

### Compilar con make

```bash
cd Cliente
make
```

Genera el ejecutable `cliente`. Para borrar lo compilado: `make clean`.


### Ejecutar

```bash
./cliente [host] [puerto]
```

| Parámetro | Por defecto | Significado |
|---|---|---|
| host | os.ecci.ucr.ac.cr | IP o nombre del intermediario |
| puerto | http | Puerto HTTP del intermediario. Normalmente 8080 |

Primero va el host y después el puerto. Ejemplos:

```bash
./cliente 127.0.0.1 8080
./cliente 10.1.35.20 8080
```


## Rutas consultables desde el navegador

Reemplaza `IP_INTERMEDIARIO` y `IP_PRODUCTOS` por las IP reales (o `127.0.0.1` si todo corre en la misma PC) y usa el puerto con el que arrancaste cada programa.

### Servidor intermediario (HTTP)

Se abre con `http://`, por ejemplo con el puerto 8080.

| Ruta | Qué muestra |
|---|---|
| `http://IP_INTERMEDIARIO:8080/TicAmazon/list.php` | Categorías de todos los servidores de productos activos y de las otras islas |
| `http://IP_INTERMEDIARIO:8080/TicAmazon/list.php?category=Electronica` | Productos de una categoría, con el servidor de origen en la primera columna. Para categorías con espacios, `category=Alimentos%20y%20bebidas`. Las categorías de otras islas van en minúsculas y con `_`, por ejemplo `category=ropa` |
| `http://IP_INTERMEDIARIO:8080/TicAmazon/servidores` | Servidores de productos conocidos, con su dirección, estado (ACTIVO o INACTIVO) y categorías |
| `http://IP_INTERMEDIARIO:8080/TicAmazon/rutas` | Tabla de rutas: cada categoría con los servidores y las islas que la tienen |
| `http://IP_INTERMEDIARIO:8080/TicAmazon/bitacora` | Bitácora en tiempo real: total de eventos, conteo por tipo y por origen, y los últimos eventos. Se refresca sola cada 3 segundos |
| `http://IP_INTERMEDIARIO:8080/TicAmazon/bitacora?tipo=ERROR` | Bitácora filtrada por tipo. Tipos: `REGISTRO`, `ACTIVO`, `INACTIVO`, `CONSULTA`, `PROFORMA`, `ERROR`, `INFO` |

No hay ruta en `/`: abrir solo `http://IP_INTERMEDIARIO:8080/` da error 404.

### Servidor de productos (HTTPS)

Se abre con `https://` y el navegador avisa del certificado autofirmado: pulsa Avanzado y Continuar. Si se usa `http://` no responde.

| Ruta | Qué muestra |
|---|---|
| `https://IP_PRODUCTOS:9091/TicAmazon/list.php` | Categorías de ese servidor, como enlaces |
| `https://IP_PRODUCTOS:9091/TicAmazon/list.php?category=Electronica` | Productos de la categoría en ese servidor únicamente, con la columna Intermediario vacía |

Las páginas de este servidor tienen un botón rojo "Salir" abajo a la derecha, que apaga el servidor de productos (pide confirmación).

### Rutas que no se abren escribiendo la dirección

Estas rutas necesitan el método POST y no funcionan desde la barra del navegador (dan error 405):

| Ruta | Dónde | Uso |
|---|---|---|
| `/TicAmazon/proforma.php` | Intermediario y servidor de productos | Compra: el cliente envía los ítems y recibe la factura |
| `/TicAmazon/registrar` | Intermediario | Registro de un servidor de productos (lo hace el servidor solo) |
| `/TicAmazon/salir` | Servidor de productos | Apagado, desde el botón "Salir" |

Ejemplo de compra con curl:

```bash
curl -X POST http://IP_INTERMEDIARIO:8080/TicAmazon/proforma.php -d "item0_categoria=Electronica&item0_producto=NOMBRE&item0_cantidad=1"
```

Cualquier otra ruta responde error 404.