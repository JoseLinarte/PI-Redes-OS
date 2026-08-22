# CI0123 - Proyecto integrador de redes y sistemas operativos
Universidad de Costa Rica - ECCI

## Descripción
Cliente y servidor UDP sobre IPv6 usando las clases `VSocket` y `Socket`.

## Requisitos
- Linux (probado en Ubuntu)
- g++
- IPv6 habilitado en loopback (`::1`)

## Compilar

```bash
make clean
make ipv6-udp-server.out
make ipv6-udp-client.out
```

Esto genera `ipv6-udp-server.out` y `ipv6-udp-client.out`.

## Ejecutar de forma local (una sola PC)

El cliente ya está configurado para enviar a `::1`, por lo que
servidor y cliente pueden correr en la misma máquina, en dos terminales
distintas.

**Terminal 1 - iniciar el servidor primero:**

```bash
./ipv6-udp-server.out
```

Queda esperando un mensaje.

**Terminal 2 - correr el cliente:**

```bash
./ipv6-udp-client.out
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

## Notas
- Para probar contra otra máquina (no local), hay que cambiar la IP de
  destino en `ipv6-udp-client.cc` (línea con `inet_pton`) por la IP real
  del servidor, en vez de `::1`.
- Si la IP de destino es de tipo link-local (empieza con `fe80::`), se
  necesita además indicar la interfaz de red (scope id).
