# Práctica Unidad II — Simulador de Semáforo y Conductores

Programación Orientada a Objetos — Mecatrónica 3er semestre — La Salle Saltillo

## Descripción

Simulador que corre en un `while(true)`: un `Semaforo` cambia de color (Verde 15s → Amarillo 5s → Rojo 25s) y avisa a los `Conductor` registrados, quienes deciden si su propio `Automovil` arranca o se detiene.

Ver el documento completo de la práctica guiada (con notas conceptuales y técnicas paso a paso) en `docs/Practica_Guiada_Semaforo.md`.

## Estructura del proyecto

```
practica_unidad_2/
├── include/     Headers (.h) — namespaces ControlVial y Vehiculos
├── src/         Implementaciones (.cpp) y main.cpp
├── docs/        Guía de la práctica
└── Makefile
```

## Compilar y ejecutar

```bash
make
./build/trafico
```

Detener con `Ctrl+C`.

### Windows sin `make`

Con MinGW-w64 instalado y `g++` disponible en el `PATH`, abre PowerShell en la carpeta del proyecto y ejecuta:

```powershell
New-Item -ItemType Directory -Force build
g++ -std=c++17 -Wall -Iinclude src\Automovil.cpp src\Conductor.cpp src\main.cpp src\Semaforo.cpp -o build\trafico.exe
.\build\trafico.exe
```

En el símbolo del sistema (`cmd`) puedes usar:

```bat
if not exist build mkdir build
g++ -std=c++17 -Wall -Iinclude src\Automovil.cpp src\Conductor.cpp src\main.cpp src\Semaforo.cpp -o build\trafico.exe
build\trafico.exe
```

Detener con `Ctrl+C`.

El proyecto es compatible con Windows (MinGW-w64/g++), macOS y Linux. Usa `std::this_thread::sleep_for` en vez de `sleep()`/`Sleep()` por portabilidad.

## Limpiar binarios

```bash
make clean
```
