# Intermediario - TicAmazon

Servidor intermediario: recibe los requests HTTP del cliente y los reenvia
al servidor de productos propio, devolviendo la respuesta tal cual (incluyendo
el codigo de estado). Si el servidor de productos no esta disponible,
responde `502 Bad Gateway`.

```
cliente  -->  intermediario  -->  servidor de productos
```

## Requisitos

- Linux (probado en Fedora/Ubuntu)
- g++ con soporte para C++17
- make
- Un servidor de productos corriendo (ver su propio README)

## Compilar

```bash
make
```

Genera el binario `intermediario`.

## Limpiar

```bash
make clean
```

## Ejecutar

```bash
./intermediario [puerto] [host_productos] [puerto_productos]
```

- `puerto`: puerto en el que escucha el intermediario. Por defecto `8081`.
- `host_productos`: IP o nombre del servidor de productos. Por defecto `127.0.0.1`.
- `puerto_productos`: puerto del servidor de productos. Por defecto `8080`.

También se puede ejecutar directamente con:

```bash
make run
make run PUERTO=9000
make run HOST_PRODUCTOS=192.168.1.20 PUERTO_PRODUCTOS=8080
```

Para detenerlo: `Ctrl+C`.

### Orden de arranque

1. Servidor de productos:
```bash
   ./servidor 8080
```
2. Intermediario (apuntando al servidor de productos):
```bash
   ./intermediario 8081 127.0.0.1 8080
```
3. Cliente (apuntando al intermediario, **no** al servidor de productos):
```bash
   ./cliente 127.0.0.1 8081
```

### Direccion IP

Si el intermediario y el servidor de productos corren en computadoras distintas
de la misma red, hay que indicarle al intermediario la IP del servidor de
productos, y al cliente la IP del intermediario:

```bash
ip addr show
```

## Probar

Con `curl` (desde otra terminal, con el servidor de productos y el
intermediario corriendo). Las respuestas deben ser identicas a las que
devuelve el servidor de productos directamente:

```bash
# Listar categorias
curl "http://<IP-del-intermediario>:8081/TicAmazon/list.php"

# Listar productos de una categoria
curl "http://<IP-del-intermediario>:8081/TicAmazon/list.php?category=Alimentos%20y%20bebidas"

# Generar una proforma
curl -X POST "http://<IP-del-intermediario>:8081/TicAmazon/proforma.php" \
     -d "item0_categoria=Alimentos y bebidas&item0_producto=banano&item0_cantidad=3"
```

Para probar el caso de error, detener el servidor de productos y repetir
cualquiera de los comandos: el intermediario debe responder `502 Bad Gateway`.

```bash
curl -i "http://<IP-del-intermediario>:8081/TicAmazon/list.php"
```

## Otros comandos del Makefile

```bash
make objetos   # compila sin enlazar, solo para revisar errores rapido
make clean     # borra el binario y los objetos compilados
```