# CI0123 - Proyecto integrador de redes y sistemas operativos
Universidad de Costa Rica - ECCI

## Descripción
Cliente y servidor UDP sobre IPv4 usando las clases `VSocket` y `Socket`.

## Requisitos
- Linux (probado en Ubuntu)
- g++
- IPv4 habilitado en loopback (`127.0.0.1`)

## Compilar

```bash
make clean
make ipv4-udp-server.out
make ipv4-udp-client.out
```

Esto genera `ipv4-udp-server.out` y `ipv4-udp-client.out`.

## Ejecutar de forma local (una sola PC)

El cliente ya está configurado para enviar a `127.0.0.1`, por lo que
servidor y cliente pueden correr en la misma máquina, en dos terminales
distintas.

**Terminal 1 - iniciar el servidor primero:**

```bash
./ipv4-udp-server.out
```

Queda esperando un mensaje.

**Terminal 2 - correr el cliente:**

```bash
./ipv4-udp-client.out
```

## Salida esperada

Terminal del servidor:
```
Server: message received: Hello from client 2026-ii
Server: Hello message sent.
```

Terminal del cliente:
```
Client: Hello message sent.
Client message received: Hello from CI0123 server 2026-ii
```
