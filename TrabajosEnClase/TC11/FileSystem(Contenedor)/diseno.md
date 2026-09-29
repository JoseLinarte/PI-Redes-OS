# MODELO DE ALMACENAMIENTO - VERSION FINAL(espero)

## 1. Descripcion general

- Todo el almacenamiento es **un unico archivo**, dividido en bloques de tamaño fijo de 256 bytes, numerados 0, 1, 2, ..., n-1.
- El bloque 0 es un **bloque de control** con informacion global del sistema.
- Existe una zona de **bloques de directorio**, encadenados, que listan las bodegas existentes y donde empieza cada una.
- Cada bodega vive en una **cadena de bloques de datos** dentro de ese mismo archivo, donde se guardan sus productos como texto separado por comas.
- El encadenado de bloques (tanto del directorio como de cada bodega) es **enlazado**: cada bloque tiene un campo `bloque_siguiente` que apunta al proximo bloque de la cadena, o -1 si es el ultimo.
- Como un bloque es de tamaño fijo pero el directorio o una bodega pueden crecer mas alla de un bloque, ambas zonas son **auto-extensibles**: cuando un bloque se llena, se engancha un bloque nuevo a la cadena. No hay un limite fijo de bodegas ni de productos por bodega.
- Los bloques que no estan en uso forman una **lista de bloques libres**, para reciclarlos antes de hacer crecer el archivo.

## 2. Tipos de bloque

| Valor | Nombre | Descripcion |
|---|---|---|
| 0 | bloque libre | no esta en uso, disponible para asignar |
| 1 | bloque de directorio | contiene entradas (nombre de bodega + puntero a sus datos) |
| 2 | bloque de datos | contiene el texto de una bodega (sus productos) |

El bloque 0 del archivo es distinto a todos estos: es el bloque de control (ver seccion 3), y no representa ninguno de estos tipos.

## 3. Bloque de control (bloque 0) - 256 bytes

| Campo | Tamaño (bytes) | Descripcion |
|---|---|---|
| num_bloques_totales | 4 | cantidad total de bloques en el archivo |
| primer_bloque_directorio | 4 | indice del primer bloque de directorio |
| primer_bloque_libre | 4 | indice del primer bloque libre de la lista de libres, -1 si no hay ninguno |
| cantidad_bloques_libres | 4 | cuantos bloques libres hay actualmente |
| cantidad_bodegas | 4 | cuantas bodegas existen actualmente |
| relleno | 236 | bytes sin usar (reservado) |

Suma total: 4+4+4+4+4+236 = 256 bytes

## 4. Encabezado comun a todos los demas bloques (12 bytes)

Todo bloque que no sea el bloque de control empieza con estos 3 campos:

| Campo | Tamaño (bytes) | Descripcion |
|---|---|---|
| tipo_bloque | 4 | 0 = libre, 1 = directorio, 2 = datos |
| bloque_siguiente | 4 | indice del siguiente bloque encadenado, -1 si no hay mas |
| cantidad_usada | 4 | segun el tipo: cantidad de entradas de directorio validas, o bytes de texto usados en el bloque |

## 5. Bloque tipo 1: directorio - 256 bytes

Encabezado comun (12 bytes) + una lista de entradas de directorio. Cada entrada:

| Campo | Tamaño (bytes) | Descripcion |
|---|---|---|
| nombre_bodega | 20 | nombre de la bodega |
| primer_bloque_datos | 4 | indice del primer bloque de datos de esa bodega |

Cada entrada ocupa 24 bytes. En el espacio disponible (256 - 12 = 244 bytes) caben 10 entradas (240 bytes), quedando 4 bytes de relleno.

Suma: 12 (comun) + 10×24 + 4 (relleno) = 256 bytes

Si se necesita una entrada de directorio numero 11 (o mas), el campo `bloque_siguiente` de este bloque apunta a otro bloque tipo 1, formando una cadena tan larga como se necesite.

## 6. Bloque tipo 2: datos de una bodega - 256 bytes

Encabezado comun (12 bytes) + un area de texto de 244 bytes, donde se van empacando registros de productos, uno detras de otro, separados por comas y terminados en punto:
nombre_bodega,categoria,producto,cantidad,precio.

| Campo | Tamaño (bytes) | Descripcion |
|---|---|---|
| texto | 244 | registros de productos en texto, empacados uno tras otro mientras quepan completos |

Suma: 12 (comun) + 244 = 256 bytes

`cantidad_usada` (del encabezado comun) indica cuantos bytes del area de texto estan ocupados. Un registro nunca se parte a la mitad entre dos bloques: si el proximo registro no cabe completo en el espacio restante del bloque actual, se engancha un bloque de datos nuevo a la cadena de esa bodega (mismo mecanismo de auto-extension).

## 7. Bloque tipo 0: libre - 256 bytes

| Campo | Tamaño (bytes) | Descripcion |
|---|---|---|
| (encabezado comun) | 12 | tipo_bloque = 0, bloque_siguiente apunta al proximo bloque libre (o -1 si es el ultimo) |
| sin_usar | 244 | contenido sin definir |

La lista de bloques libres es una lista enlazada: el bloque de control apunta al primero (`primer_bloque_libre`) y sabe cuantos hay en total (`cantidad_bloques_libres`); cada bloque libre apunta al siguiente usando su propio `bloque_siguiente`.

## 8. Como se agrega una bodega nueva

1. Se recorre la cadena de bloques de directorio buscando uno con espacio (menos de 10 entradas). Si ninguno tiene espacio, se obtiene un bloque libre (o se agrega uno nuevo al final del archivo si no hay libres), se marca tipo 1, y se engancha al final de la cadena de directorio con `bloque_siguiente`.
2. Se obtiene un bloque para los datos de la bodega (reciclado de la lista de libres, o nuevo al final del archivo), se marca tipo 2, y se pone `bloque_siguiente = -1` y `cantidad_usada = 0` (todavia sin productos).
3. Se agrega una entrada nueva en el bloque de directorio con espacio: `nombre_bodega` y `primer_bloque_datos` apuntando al bloque creado en el paso 2.
4. Se suma 1 a `cantidad_bodegas` en el bloque de control, y se resta del contador de libres cada bloque que se haya tomado de la lista de libres.

## 9. Como se agrega un producto a una bodega

1. Se busca la entrada de la bodega recorriendo la cadena de bloques de directorio, y se obtiene `primer_bloque_datos`.
2. Se recorre la cadena de bloques de datos de esa bodega hasta llegar al ultimo (el que tiene `bloque_siguiente = -1`).
3. Se arma el texto del registro: `nombre_bodega,categoria,producto,cantidad,precio.`
4. Si el registro completo cabe en el espacio restante del ultimo bloque (256 - 12 - cantidad_usada), se agrega ahi y se actualiza `cantidad_usada`.
5. Si no cabe, se obtiene un bloque nuevo (reciclado o al final del archivo), se marca tipo 2, se engancha con `bloque_siguiente` al final de la cadena de esa bodega, se escribe el registro ahi, y se actualiza `cantidad_usada` de ese bloque nuevo.

## 10. Como se elimina una bodega

1. Se recorre la cadena de bloques de datos de la bodega (a partir de `primer_bloque_datos`), y cada bloque se marca tipo_bloque = 0 (libre) y se inserta al inicio de la lista de bloques libres, sumando 1 a `cantidad_bloques_libres` por cada uno.
2. Se quita la entrada correspondiente del bloque de directorio donde estaba (corriendo las demas entradas de ese bloque o marcando el espacio como vacio).
3. Se resta 1 a `cantidad_bodegas` en el bloque de control.
4. (Opcional) Si al quitar la entrada un bloque de directorio queda completamente vacio y no es el unico de la cadena, tambien se puede liberar siguiendo la misma logica del paso 1.

## 11. Ejemplo

**Bloque de control (bloque 0):**
- num_bloques_totales = 10
- primer_bloque_directorio = 1
- primer_bloque_libre = -1
- cantidad_bloques_libres = 0
- cantidad_bodegas = 2

**Bloque 1 (tipo 1, directorio):**
- tipo_bloque = 1, bloque_siguiente = -1, cantidad_usada = 2
- entrada 1: nombre_bodega = "Bodega-1", primer_bloque_datos = 2
- entrada 2: nombre_bodega = "Bodega-2", primer_bloque_datos = 4

**Bloque 2 (tipo 2, datos de Bodega-1):**
- tipo_bloque = 2, bloque_siguiente = 3, cantidad_usada = 244 (lleno)
- texto: `Bodega-1,Alimentos y bebidas,banano,100,10.Bodega-1,Alimentos y bebidas,manzana,10,300.Bodega-1,Alimentos y bebidas,naranja,40,50.Bodega-1,Salud y belleza,perfume,33,44....`

**Bloque 3 (tipo 2, datos de Bodega-1, continuacion):**
- tipo_bloque = 2, bloque_siguiente = -1, cantidad_usada = 60
- texto: `Bodega-1,Salud y belleza,shampoo,33,44.Bodega-1,Isla1,shampoo,22,11.`

**Bloque 4 (tipo 2, datos de Bodega-2):**
- tipo_bloque = 2, bloque_siguiente = -1, cantidad_usada = 0
- texto: (vacio, bodega recien creada)