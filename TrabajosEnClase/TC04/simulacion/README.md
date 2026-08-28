# Trabajo en clase #04 — Simulación del protocolo servidor de productos / intermediario

## Cómo compilar
```bash
make
```

## Cómo ejecutar (dos terminales)
Terminal 1:
```bash
./servidor_productos
```

Terminal 2:
```bash
./servidor_intermediario
```

El servidor de productos debe iniciarse primero o quedará esperando; si el intermediario arranca primero simplemente creará las colas y esperará mensajes de respuesta que el productor irá contestando cuando inicie.

## Limpieza manual (por si un proceso termina abruptamente)
```bash
ipcs -q            # lista las colas de mensajes activas
ipcrm -q <id>       # elimina una cola específica
```

