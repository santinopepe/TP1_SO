# TP1_SO - Chomp Champs

Este proyecto implementa un juego llamado **Chomp Champs**, donde múltiples jugadores compiten en un tablero compartido. El juego utiliza memoria compartida y semáforos para la sincronización entre procesos.

## Estructura del Proyecto

El proyecto está compuesto por los siguientes archivos principales:

- **`Master.c`**: Controla el flujo principal del juego, inicializa los jugadores, la vista y gestiona la lógica del juego.
- **`Vista.c`**: Representa la vista del juego, mostrando el estado del tablero y los jugadores.
- **`PlayerInteligente.c`**: Implementa un jugador con lógica inteligente para decidir sus movimientos.
- **`Utilis.h`**: Contiene las definiciones de estructuras y funciones compartidas entre los diferentes módulos.
- **`Makefile`**: Facilita la compilación del proyecto.

## Compilación

Para compilar el proyecto, utiliza el archivo `Makefile` incluido. Ejecuta el siguiente comando en la raíz del proyecto:
```bash
make all 
```

##  Descripción del juego

Cada jugador es un proceso independiente que interactúa con el máster del juego a través de memoria compartida. El máster coordina la ejecución, administra el tablero y sincroniza los turnos mediante semáforos. El objetivo de cada jugador es realizar movimientos estratégicos para dominar el tablero y vencer a los oponentes.

##  Parámetros del máster

El máster acepta los siguientes parámetros:

| Parámetro        | Descripción                                                                 | Valor por defecto     |
|------------------|-----------------------------------------------------------------------------|------------------------|
| `-w width`       | Ancho del tablero                                                           | 10                     |
| `-h height`      | Alto del tablero                                                            | 10                     |
| `-d delay`       | Milisegundos de espera entre impresiones del estado                         | 200                    |
| `-t timeout`     | Tiempo máximo (en segundos) para recibir un movimiento válido de un jugador | 10                     |
| `-s seed`        | Semilla para la generación aleatoria del tablero                            | `time(NULL)`           |
| `-v view`        | Ruta al binario de la vista (opcional)                                      | Sin vista              |
| `-p player1 ...` | Rutas a los binarios de los jugadores (mínimo 1, máximo 9)                  | **Obligatorio**        |

###  Ejemplo de ejecución

```bash
./master -w 15 -h 12 -d 300 -t 8 -s 1234 -v ./view.out -p ./playerI.out ./playerI.out
```



##  Integrantes

- **Manuel Araujo** - Legajo: 64090 
- **Santino Pepe** - Legajo: 64147
- **Santiago Nogueira** - Legajo: 64113
