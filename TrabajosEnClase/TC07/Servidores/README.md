# Servidores TCP - Compilar y Ejecutar

## 1. Compilar

```bash
make clean
make ForkMirrorServer.out
make MirrorClient.out
make ThreadMirrorServer.out
```

## 2. Ejecutar

Antes de ejecutar, asegurar de que `MirrorClient.cc` tenga la IP correcta:

```cpp
s->Connect( "TU_IP_AQUI", PORT );
```
- Misma máquina: usar `"127.0.0.1"`
- Máquinas distintas: usar la IP real de la máquina servidor (`ip addr` o `hostname -I`)

### Prueba con servidor Fork

**Terminal 1:**
```bash
./ForkMirrorServer.out
```
(no imprime nada, queda esperando conexiones — esto es normal)

**Terminal 2:**
```bash
./MirrorClient.out "mensaje de prueba"
```
Debe imprimir el mismo mensaje de vuelta: `mensaje de prueba`

Para detener el servidor: `Ctrl+C` en Terminal 1.

### Prueba con servidor de Hilos (Threads)

**Terminal 1:**
```bash
./ThreadMirrorServer.out
```
(tampoco imprime nada hasta que llega una conexión)

**Terminal 2:**
```bash
./MirrorClient.out "mensaje de prueba"
```
Debe imprimir el mismo mensaje de vuelta, y en Terminal 1 debería aparecer:
```
Server received: mensaje de prueba
```

Para detener el servidor: `Ctrl+C` en Terminal 1.

## 3. Limpiar

```bash
make clean
```
