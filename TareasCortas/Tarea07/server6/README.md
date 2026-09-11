# Servidores Mirror (Fork / Hilos) — IPv4 + IPv6

## Compilar

```bash
make clean
make ForkMirrorServer.out
make ThreadMirrorServer.out
make MirrorClient.out
```


## Ejecutar

Se necesitan **dos terminales**.

### Prueba con IPv4 (por defecto)

**Terminal 1 (servidor fork):**
```bash
./ForkMirrorServer.out
```

**Terminal 2 (cliente):**
```bash
./MirrorClient.out "mensaje de prueba"
```
Debe imprimir de vuelta `mensaje de prueba`.

Para detener el servidor: `Ctrl+C` en Terminal 1.

### Prueba con IPv6

**Terminal 1:**
```bash
./ForkMirrorServer.out -6
```

**Terminal 2:**
```bash
./MirrorClient.out "mensaje de prueba" -6
```

Lo mismo aplica con el servidor de hilos:

```bash
./ThreadMirrorServer.out -6
./MirrorClient.out "mensaje de prueba" -6
```


```bash
make clean
```