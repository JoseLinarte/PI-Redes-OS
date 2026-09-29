## Requisitos

- Linux (probado en Fedora/Ubuntu)
- g++ con soporte para C++17
- make


## Compilar

```bash
make
```

Genera el binario `servidor`.

## Limpiar

```bash
make clean
```

## Ejecutar

```bash
./servidor 8080 bodegas.data ci0123.pem ci0123.pem
```

## Probar

Con `curl` (desde otra terminal, mientras el servidor sigue corriendo):

```bash
curl -k -v https://localhost:8080/TicAmazon/list.php
     
```

## Otros comandos del Makefile

```bash
make objetos   # compila sin enlazar, solo para revisar errores rapido
make clean     # borra los binarios y objetos compilados
```
