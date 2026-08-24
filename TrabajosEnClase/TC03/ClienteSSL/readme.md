# CI0123 - Proyecto integrador de redes y sistemas operativos
Universidad de Costa Rica - ECCI

## Descripción
Cliente TCP/SSL sobre IPv4 usando las clases `VSocket`, `Socket` y `SSLSocket`.
`SSLSocket` hereda de `VSocket` y agrega soporte para conexiones cifradas
usando OpenSSL (`SSL_CTX`, `SSL_connect`, `SSL_read`, `SSL_write`), mientras
que `Socket` maneja conexiones TCP normales.

## Requisitos
- Linux (probado en Ubuntu)
- g++
- Librerías de desarrollo de OpenSSL

```bash
sudo apt update
sudo apt install libssl-dev build-essential
```

## Compilar

```bash
make clean
make
```

Esto genera el ejecutable `ipv4-ssl-cli.out`.

## Ejecutar

El cliente soporta dos modos según se le pase un argumento o no:

**Conexión normal (sin SSL), por el puerto 80:**

```bash
./ipv4-ssl-cli.out
```

**Conexión segura SSL, por el puerto 443:**

```bash
./ipv4-ssl-cli.out 1
```

(el valor del argumento no importa, solo su presencia activa la rama SSL
en el `main`).

 **Nota:** el servidor de destino es un host de la UCR
 (`163.178.104.187`). Si se corre fuera de la red de la universidad
 puede ser necesario conectarse por VPN institucional para que la
 conexión no dé timeout.

## Salida esperada

```
Bytes read <N>
HTTP/1.1 200 OK
...
```
