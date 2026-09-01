# TicAmazon - Cliente

Cliente en C++ que consulta el servidor de bodega TicAmazon vía HTTP sobre
sockets propios (jerarquía `VSocket`/`Socket`), permite elegir productos por
categoría, agregarlos a un carrito, y genera una factura local.

## Compilar

```bash
make cliente
```

## Ejecutar

```bash
./cliente
```

## Uso

1. Ingresar el nombre exacto de una categoría (ej: `Alimentos y bebidas`).
2. Elegir el índice de un producto de la lista mostrada y la cantidad deseada.
3. Repetir para otras categorías, o finalizar.
4. Al finalizar, se muestra la factura con el detalle y el total.

## Requisitos

- Linux
- Compilador C++17 (`g++`).
- Acceso de red al servidor `os.ecci.ucr.ac.cr` 

## Limpiar binarios

```bash
make clean
```