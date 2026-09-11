# Bodegas — Servidor de productos + Cliente

## Compilar

```bash
make clean
make
```

Esto genera dos ejecutables: `ServidorProductos.out` y `Cliente.out`.

## Ejecutar

Se necesitan **dos terminales**.

**Terminal 1 — servidor:**
```bash
./ServidorProductos.out
```

**Terminal 2 — cliente:**
```bash
./Cliente.out
```

Menú del cliente:
```
1) Listar categorias
2) Ver productos de una categoria
3) Agregar producto al carrito
4) Pedir factura proforma
5) Salir
```

### Ejemplo de uso

1. `1` → ver categorías (Alimentos, Construccion, Vehiculos)
2. `2` → escribir `Alimentos` → ver los productos, con el `IdBodega` de cada uno
3. `3` → escribir el nombre del producto (por ejemplo `Refresco`), la cantidad, y el `IdBodega`
4. Repetir el paso 3 con más productos si querés
5. `4` → ver la factura proforma con el total

## Probar desde un navegador

```
http://127.0.0.1:8080/categorias
http://127.0.0.1:8080/productos/Alimentos
```

## Reiniciar la bodega desde cero

```bash
make clean
make
```