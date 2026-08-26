# CI0123 - Proyecto integrador de redes y sistemas operativos
Universidad de Costa Rica - ECCI

## Descripción
Cliente TCP/SSL sobre IPV6 usando las clases `VSocket`, `Socket` y `SSLSocket`.
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
make tcp-cli.out
```

Esto genera el ejecutable `./tcp-cli.out 6 1 o ./tcp-cli.out 4 1`.