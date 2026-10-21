# Avance de negociaciones del protocolo inter-grupal

**Grupos:** Grupo 5 y Grupo 2 (Protocolo mancomunado)
**Medio:** chat de Telegram
**Estado:** en discusión, nada cerrado todavía

## Qué hemos hablado

Estuvimos comparando los dos protocolos por Telegram. La idea es no escoger uno completo, sino juntar lo mejor de cada uno. Hasta ahora las ideas son estas.

## Lo que se podría unir

- **Descubrimiento de islas:** usar el `ANNOUNCE` por UDP del Grupo 2. Es más simple que configurar las IP a mano y que el `NOTIFY_CHANGE` / `SYNC` del Grupo 5, porque cada isla se anuncia sola.
- **Puerto entre islas:** usar el 9090 (TCP para datos y UDP para anuncios), que ya está definido en el protocolo del Grupo 2. El 2026 del Grupo 5 se queda solo para intermediario-bodega dentro de la isla.
- **Formato de mensajes:** quedarnos con el formato del Grupo 2 (`TIPO/campo1/campo2`, con código de 2 dígitos al inicio). Se puede agregar el `\n` al final de cada mensaje, como en el Grupo 5, para que el `getline()` funcione bien.
- **Identificadores:** usar `INT_XX` para las islas (como el Grupo 2) para que todos usemos el mismo formato.
- **Consultas:** el Grupo 2 solo busca por categoría, mientras que el Grupo 5 tiene filtros de precio mínimo, precio máximo, cantidad y características. Se podrían agregar esos filtros como opcionales al `GET_PRODUCTS`.
- **Compra:** usar el flujo del Grupo 2 (`RESERVE` y luego `BUY`), que evita vender algo sin stock, y agregarle la factura proforma del Grupo 5 (IVA 13 % y registro de 256 bytes en disco).
- **Errores:** usar los códigos 100 a 104 del Grupo 2, que son más claros que el `STATUS` del Grupo 5.
- **Bitácora:** usar la del Grupo 5 (`TIMESTAMP | NODO | EVENTO | RESULTADO | DETALLE`), porque el Grupo 2 solo dice que el log es local y no define formato.

## Lo que falta decidir

- Si se acepta el formato con `/` o hay que defender el de `CLAVE=VALOR`.
- Cómo se maneja la moneda y los decimales de los precios, porque los dos protocolos los escriben distinto.
- Si el `RESERVE` se mantiene en los 60 s del Grupo 2.
- Quién escribe la versión final del documento unificado.
