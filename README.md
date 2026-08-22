# CI0123 - Proyecto integrador de redes y sistemas operativos
Universidad de Costa Rica - ECCI

## Descripción del repositorio

Este repositorio contiene las tareas y trabajos en clase del curso
CI0123, organizados en carpetas independientes. Cada carpeta tiene su
propio `README.md` con instrucciones específicas de compilación y
ejecución para esa entrega.

## Estructura

```
.
├── README.md             
├── .gitignore
├── Tareas/
│   ├── Tarea01/
│   │   └── README.md
│   ├── Tarea02/
│   │   └── README.md
│   └── .../
└── TrabajosClase/
    ├── TClase#01/
    │   └── README.md
    ├── TClase#02/
    │   └── README.md
    └── .../
```

Cada carpeta de trabajo suele incluir:
- Código fuente (`.cc`, `.h`)
- `Makefile` propio
- `README.md` con instrucciones de esa entrega en particular

## Cómo usar este repositorio

1. Entrar a la carpeta correspondiente según el tipo de entrega
   (`Tareas/` o `TrabajosClase/`) y luego a la carpeta específica:
   ```bash
   cd Tareas/TareaN
   # o bien
   cd TrabajosClase/TClase#0N
   ```
2. Seguir las instrucciones del `README.md` dentro de esa carpeta.

## Notas generales

- Cada entrega es independiente: compilar una no afecta a las demás.
- Los binarios y archivos objeto (`.o`, `.out`) no se versionan (ver
  `.gitignore`); hay que compilar localmente con `make` en cada carpeta.
