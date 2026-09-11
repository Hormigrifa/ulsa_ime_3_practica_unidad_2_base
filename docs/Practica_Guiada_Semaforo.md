# Práctica guiada: Simulador de Semáforo y Conductores
## Programación Orientada a Objetos — IME III


## Antes de empezar

Esta práctica **no es un proyecto de evaluación** — es un espacio para que construyas, paso a paso y con código en la mano, el modelo mental de qué significa "pensar en objetos". El lenguaje C++ y su sintaxis son solo el vehículo; lo que de verdad nos importa hoy es que salgas de esta sesión pudiendo responder, sin dudar: *¿quién es responsable de qué, y cómo se hablan entre sí los objetos de mi programa?*

Vamos a construir un simulador que corre indefinidamente (`while(true)`) y que modela un cruce vial:

- Un **Semáforo** que cambia de color solo: Verde (15 s) → Amarillo (5 s) → Rojo (25 s) → Verde...
- Dos **Conductores**, cada uno con su propio **Automóvil**.
- Cuando el semáforo cambia de color, **avisa** a los conductores registrados. Cada conductor decide, y le ordena a su propio auto que arranque o se detenga.

Esta práctica usa un dominio distinto (tránsito vehicular) al de tu Parcial 1 (competencia de robótica) **a propósito** — la idea es que practiques el razonamiento de diseño en un contexto nuevo, no que copies estructura literal.

**A lo largo del documento vas a encontrar dos tipos de nota:**

> **Nota conceptual:** te explica el *por qué* desde la Orientación a Objetos — qué principio estás poniendo en práctica y por qué importa para tu forma de pensar como diseñador de software.

> **Nota técnica:** te explica el *por qué* desde C++ — por qué la sintaxis se escribe así y no de otra forma, y qué problema evita.

Sigue los pasos en orden. Cada clase que construyas hoy es pequeña a propósito — la meta no es la cantidad de código, sino que entiendas cada línea que escribes.

---

## Paso 0 — Estructura del proyecto

Vamos a separar los archivos de declaración (`.h`) de los de implementación (`.cpp`) en dos carpetas distintas — `include/` y `src/` — en vez de tenerlos todos sueltos en la raíz:

```
practica-semaforo/
├── include/
│   ├── EstadoSemaforo.h
│   ├── Automovil.h
│   ├── Semaforo.h
│   └── Conductor.h
├── src/
│   ├── Automovil.cpp
│   ├── Semaforo.cpp
│   ├── Conductor.cpp
│   └── main.cpp
└── Makefile
```

> **Nota conceptual:** fíjate que ya, antes de escribir una sola línea de lógica, el proyecto está organizado por *responsabilidades*, no por "todo junto en un archivo". Esto es una decisión de diseño, no un detalle administrativo: cada archivo va a representar una entidad con un rol claro dentro del sistema. Separar `include/` de `src/` es la misma idea aplicada un nivel más arriba: no solo cada *clase* tiene su lugar, sino que cada *tipo de archivo* (el contrato público vs. la implementación privada) también lo tiene.

> **Nota técnica — rutas de `#include` y la bandera `-I`:** si `Automovil.cpp` vive en `src/` y `Automovil.h` vive en `include/`, técnicamente tendrías que escribir `#include "../include/Automovil.h"` para que el compilador lo encuentre. **No vamos a hacer eso.** En vez de "ensuciar" el código con rutas relativas, le decimos al compilador dónde buscar los headers usando la bandera `-Iinclude` (más abajo verás esto en el `Makefile`). Con esa bandera, cada `.cpp` sigue escribiendo simplemente `#include "Automovil.h"`, sin importar en qué carpeta esté guardado — el compilador ya sabe que debe revisar `include/` además de la carpeta actual. Esto es exactamente lo mismo que hace un IDE como VSCode cuando configuras las "include paths" del proyecto.

> **Nota técnica — dónde va `main.cpp`:** lo colocamos en `src/` junto con las demás implementaciones, porque conceptualmente `main` también es "código de implementación", no una interfaz pública que otros archivos necesiten incluir. Si prefieres dejarlo en la raíz del proyecto en vez de en `src/`, también es válido — solo asegúrate de ajustar la ruta correspondiente en el `Makefile` (lo señalamos en el Paso 8, más abajo).

---

## Paso 1 — Namespaces: separando los dominios del problema

Antes de escribir cualquier clase, vamos a decidir en qué "vecindario" vive cada una. Este simulador tiene dos dominios claramente distintos:

- **`ControlVial`**: todo lo relacionado con la infraestructura que regula el tránsito (hoy: el semáforo).
- **`Vehiculos`**: todo lo relacionado con los vehículos que circulan (hoy: `Automovil`; en unidades futuras, cuando veamos herencia, aquí vivirán también `Moto`, `Pickup`, etc.).

> **Nota conceptual:** un namespace no es una clase ni agrega comportamiento — es una forma de decirle a cualquiera que lea tu código "estas cosas pertenecen conceptualmente juntas". Es la primera herramienta que ves en el curso para organizar *arquitectura*, no solo sintaxis. Cuando tu proyecto crezca (como el Parcial 2 con jerarquías de clases), vas a agradecer haber separado los dominios desde el principio.

> **Nota técnica:** en C++, un namespace se declara así:
> ```cpp
> namespace NombreDelNamespace {
>     // clases, funciones, etc.
> }
> ```
> Para usar algo que está dentro de un namespace desde afuera, escribes `NombreDelNamespace::LoQueNecesitas` (operador de resolución de ámbito `::`, el mismo que ya usaste para implementar métodos en `Robot.cpp`). También puedes escribir `using namespace NombreDelNamespace;` para evitar escribir el prefijo repetidamente — lo vas a usar en `main.cpp`, pero **evita hacerlo dentro de un archivo `.h`**, porque forzaría ese "acceso directo" a todo el que incluya tu header, incluso si no lo quiere.

No necesitas escribir código todavía — solo ten en mente que `Semaforo` va a vivir dentro de `ControlVial`, y `Automovil` dentro de `Vehiculos`.

---

## Paso 2 — `EstadoSemaforo`: modelando un conjunto cerrado de estados

El semáforo solo puede estar en tres estados posibles. Nunca en un cuarto. Nunca en dos a la vez.

Crea **EstadoSemaforo.h**:

```cpp
#pragma once

namespace ControlVial {

    enum class EstadoSemaforo {
        Verde,
        Amarillo,
        Rojo
    };

}
```

> **Nota conceptual:** esto es abstracción en su forma más pura — estás modelando *solo* lo que el problema necesita (tres colores posibles) y nada más. No hay un "por qué" físico aquí (no estás modelando el foco ni el circuito eléctrico del semáforo), porque eso no le importa a tu sistema. Esto es exactamente la misma pregunta que te hicimos en Unidad I sobre el Robot: ¿qué información es indispensable, y qué sería "de más"?

> **Nota técnica:** usamos `enum class` (enumeración con ámbito) en vez de un `enum` clásico por dos razones prácticas: (1) un `enum` clásico se puede mezclar con `int` sin que el compilador se queje, lo cual permite errores tontos como sumarle 1 a un color; `enum class` no lo permite, así que si te equivocas el compilador te avisa. (2) Un `enum class` obliga a escribir `EstadoSemaforo::Verde` en vez de solo `Verde`, evitando que ese nombre choque con otros `Verde` que puedas tener en otro lugar del programa. Esta es la misma filosofía de "proteger de errores por diseño" que ya viste con `private` y `const`.

---

## Paso 3 — `Automovil`: el objeto que recibe órdenes

`Automovil` va a vivir dentro de `namespace Vehiculos`. Es un objeto simple: sabe su nombre, su velocidad, y sabe reaccionar a dos formas de pedirle que arranque.

**Automovil.h**:

```cpp
#pragma once
#include <string>

namespace Vehiculos {

    class Automovil {
    private:
        std::string nombre;
        int velocidad;

    public:
        explicit Automovil(const std::string& nombre);

        void arrancar();
        void arrancar(int potencia);
        void detener();

        std::string getNombre() const;
        int getVelocidad() const;
    };

}
```

**Automovil.cpp**:

```cpp
#include "Automovil.h"
#include <iostream>

namespace Vehiculos {

    Automovil::Automovil(const std::string& nombre)
        : nombre(nombre), velocidad(0) {}

    void Automovil::arrancar() {
        velocidad = 20;
        std::cout << "  [Auto] " << nombre
                  << " arranca a velocidad por defecto (" << velocidad << " km/h)\n";
    }

    void Automovil::arrancar(int potencia) {
        velocidad = potencia;
        std::cout << "  [Auto] " << nombre
                  << " arranca con potencia asignada (" << velocidad << " km/h)\n";
    }

    void Automovil::detener() {
        velocidad = 0;
        std::cout << "  [Auto] " << nombre << " se detiene\n";
    }

    std::string Automovil::getNombre() const { return nombre; }
    int Automovil::getVelocidad() const { return velocidad; }

}
```

> **Nota conceptual:** fíjate en algo importante — `Automovil` **no sabe que existe un semáforo**. No tiene ningún método que diga "reaccionar al color". Solo sabe hacer lo que un auto sabe hacer: arrancar y detenerse. Esto es responsabilidad única llevada a la práctica: `Automovil` es dueño únicamente de su propio comportamiento físico, nunca de la lógica de "cuándo" debe actuar. Esa decisión le pertenece a otro objeto, como vas a ver en el Paso 5.

> **Nota técnica — sobrecarga de funciones:** `arrancar()` y `arrancar(int potencia)` son dos versiones del **mismo mensaje**, distinguidas por su firma (la lista y tipo de parámetros). El compilador decide cuál usar según cómo lo llames — esto se llama *resolución de sobrecarga* y ocurre en tiempo de compilación, no en ejecución. No estás escribiendo dos comportamientos independientes con nombres distintos (como `arrancarSinPotencia()` y `arrancarConPotencia()`); estás ofreciendo dos formas de pedir la *misma* acción con distinta información disponible. Guarda este concepto: en Parcial 3 vas a ver sobrecarga de **operadores**, que es la misma idea aplicada a símbolos como `+` o `==`.

---

## Paso 4 — `Semaforo`: el objeto que cambia de estado y avisa

Aquí es donde entra el puntero que recorre el arreglo de estados, y donde el semáforo aprende a "avisar" a quien esté escuchando — sin saber qué hacen con el aviso.

**Semaforo.h**:

```cpp
#pragma once
#include <vector>
#include "EstadoSemaforo.h"

class Conductor; // declaración adelantada — ver nota técnica

namespace ControlVial {

    class Semaforo {
    private:
        EstadoSemaforo estados[3];
        EstadoSemaforo* actual;
        std::vector<Conductor*> conductores;

        void notificarConductores();

    public:
        Semaforo();

        void agregarConductor(Conductor* c);
        void cambiarEstado();
        EstadoSemaforo getColorActual() const;
    };

}
```

**Semaforo.cpp**:

```cpp
#include "Semaforo.h"
#include "Conductor.h"
#include <iostream>

namespace ControlVial {

    Semaforo::Semaforo() {
        estados[0] = EstadoSemaforo::Verde;
        estados[1] = EstadoSemaforo::Amarillo;
        estados[2] = EstadoSemaforo::Rojo;
        actual = &estados[0]; // arranca en Verde
    }

    void Semaforo::agregarConductor(Conductor* c) {
        conductores.push_back(c);
    }

    void Semaforo::cambiarEstado() {
        if (actual == &estados[2]) {
            actual = &estados[0]; // de Rojo regresa a Verde
        } else {
            ++actual; // avanza al siguiente estado del arreglo
        }

        std::cout << "\n>>> El semaforo cambio de estado <<<\n";
        notificarConductores();
    }

    void Semaforo::notificarConductores() {
        for (Conductor* c : conductores) {
            c->reaccionar(*actual);
        }
    }

    EstadoSemaforo Semaforo::getColorActual() const {
        return *actual;
    }

}
```

> **Nota conceptual — el paso más importante de hoy:** observa que `Semaforo` conoce a `Conductor` (lo necesita para poder llamarle `reaccionar()`), pero **`Automovil` nunca aparece aquí**. `Semaforo` jamás le habla directamente a un auto. Esto es acoplamiento controlado: el semáforo solo se compromete con lo mínimo necesario para cumplir su responsabilidad (avisar), y delega todo lo demás. También date cuenta de la secuencia de mensajes: `main` le dice a `Semaforo` que cambie → `Semaforo` le avisa a cada `Conductor` → cada `Conductor` le ordena a su `Automovil`. Ningún objeto "brinca" directamente al final de la cadena. Esta forma de organizar el aviso — un objeto que mantiene una lista de "suscriptores" a quienes notifica cuando cambia su estado — es una versión simplificada de un patrón de diseño muy usado llamado **Observer**, que vas a formalizar más adelante (Unidad V y VI) cuando tengas funciones virtuales e interfaces. Hoy lo estás viviendo en su forma más concreta, sin nombrarlo todavía como "patrón".

> **Nota técnica — el puntero `actual`:** en vez de guardar el color en una variable simple y reasignarla, `actual` es un **puntero** que apunta a una posición dentro del arreglo `estados[3]`. Cambiar de estado no significa "calcular un nuevo color", significa *mover el puntero* a la siguiente casilla del arreglo (`++actual`), y si ya está en la última, regresarlo a la primera (`&estados[0]`). Esto es exactamente lo que vimos en teoría esta semana: un puntero guarda una dirección de memoria, no un valor por sí mismo, y `++actual` avanza esa dirección al siguiente elemento del arreglo (aritmética de punteros).

> **Nota técnica — declaración adelantada (forward declaration):** en el header `Semaforo.h` escribimos `class Conductor;` sin incluir `"Conductor.h"`. ¿Por qué? Porque en el header **solo necesitamos saber que el tipo `Conductor` existe** (para poder declarar `std::vector<Conductor*>` y el parámetro de `agregarConductor`), no necesitamos ver su implementación completa todavía. La implementación completa (donde sí llamamos a `c->reaccionar(...)`) solo hace falta en el `.cpp`, y ahí sí incluimos `"Conductor.h"`. Esto reduce el acoplamiento entre archivos: si `Conductor.h` cambia, no todos los archivos que incluyen `Semaforo.h` tienen que recompilarse innecesariamente.

---

## Paso 5 — `Conductor`: el mensajero entre Semaforo y Automovil

`Conductor` vive fuera de ambos namespaces, a propósito: su trabajo es cruzar la frontera entre el mundo del tránsito y el mundo de los vehículos.

**Conductor.h**:

```cpp
#pragma once
#include <string>
#include "Automovil.h"
#include "EstadoSemaforo.h"

class Conductor {
private:
    std::string nombre;
    Vehiculos::Automovil* miAuto;

public:
    Conductor(const std::string& nombre, Vehiculos::Automovil* auto_);

    void reaccionar(ControlVial::EstadoSemaforo estado);
};
```

**Conductor.cpp**:

```cpp
#include "Conductor.h"
#include <iostream>

Conductor::Conductor(const std::string& nombre, Vehiculos::Automovil* auto_)
    : nombre(nombre), miAuto(auto_) {}

void Conductor::reaccionar(ControlVial::EstadoSemaforo estado) {
    using ControlVial::EstadoSemaforo;

    std::cout << "[Conductor] " << nombre << " ve el cambio del semaforo...\n";

    if (estado == EstadoSemaforo::Verde) {
        miAuto->arrancar(60);
    } else {
        miAuto->detener();
    }
}
```

> **Nota conceptual:** `Conductor` es la pieza que le da sentido a todo el diseño de hoy — es el objeto que **traduce** un aviso ("el semáforo cambió") en una decisión ("entonces mi auto debe arrancar o detenerse"). Ningún otro objeto del sistema podría tomar esa decisión sin volverse responsable de cosas que no le corresponden. Esta es la respuesta a la pregunta que dejamos abierta la sesión pasada: la inteligencia de "qué hacer" no vive en `Semaforo` (que solo cambia de color) ni en `Automovil` (que solo ejecuta la orden que recibe) — vive en `Conductor`, que es exactamente donde le corresponde vivir en el mundo real.

> **Nota técnica — HAS-A con puntero, uno a uno:** `Conductor` tiene un `Automovil*` (no un `vector`), porque la relación aquí es uno a uno: cada conductor maneja *su propio* auto, no una colección de autos. Compara esto con `Semaforo`, que sí necesita `vector<Conductor*>` porque puede tener varios conductores suscritos. La estructura de datos que eliges debe reflejar la cardinalidad real de la relación que estás modelando — no uses `vector` "por si acaso" si la relación es siempre uno a uno.

> **Nota técnica — parámetro por valor:** `reaccionar(EstadoSemaforo estado)` recibe el enum **por valor**, no por referencia. Esto es intencional: `EstadoSemaforo` es un tipo tan pequeño (internamente es casi un `int`) que copiarlo no cuesta nada — pasar por referencia aquí no ganaría eficiencia y solo agregaría una indirección innecesaria. Compáralo con `std::string`, donde sí usamos `const std::string&` en los constructores, porque copiar una cadena de texto sí puede ser costoso. La regla práctica: tipos pequeños y simples (`int`, `bool`, `enum class`) se pasan por valor; tipos que pueden ser grandes (`std::string`, objetos completos, contenedores) se pasan por referencia constante.

---

## Paso 6 — `main.cpp`: el reloj, y nada más

Ahora armamos todo. Presta atención a qué tan poco hace `main` — eso no es un accidente.

```cpp
#include <iostream>
#include <thread>
#include <chrono>
#include "Automovil.h"
#include "Semaforo.h"
#include "Conductor.h"

int main() {
    using namespace Vehiculos;
    using namespace ControlVial;

    Automovil auto1("Auto de Ana");
    Automovil auto2("Auto de Beto");

    Conductor conductorAna("Ana", &auto1);
    Conductor conductorBeto("Beto", &auto2);

    Semaforo semaforo;
    semaforo.agregarConductor(&conductorAna);
    semaforo.agregarConductor(&conductorBeto);

    int duraciones[3] = {15, 5, 25}; // Verde, Amarillo, Rojo, en segundos
    int indiceDuracion = 0;
    int segundosTranscurridos = 0;

    std::cout << "Simulador de semaforo iniciado (Ctrl+C para detener)\n";

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ++segundosTranscurridos;

        if (segundosTranscurridos >= duraciones[indiceDuracion]) {
            semaforo.cambiarEstado();
            indiceDuracion = (indiceDuracion + 1) % 3;
            segundosTranscurridos = 0;
        }
    }

    return 0;
}
```

> **Nota conceptual:** léelo de nuevo — `main` no tiene ni un solo `if` que pregunte "¿el color es verde?". No sabe nada sobre arrancar ni detener autos. Su única responsabilidad es llevar la cuenta del tiempo y decirle al semáforo "ya pasó tu tiempo, cambia". Cuando el punto de entrada de tu programa es corto y "tonto", casi siempre es señal de que repartiste bien las responsabilidades entre tus objetos. Si en tu Parcial 1 tu `main` termina teniendo decenas de líneas de lógica de negocio, es una señal de que esa lógica debería vivir en una clase, no en `main`.

> **Nota técnica — por qué `<thread>` y `<chrono>`, y no `sleep()`:** la función clásica `sleep()` viene de librerías distintas en Windows (`Sleep()`, en milisegundos, de `<windows.h>`) y en Linux/macOS (`sleep()`, en segundos, de `<unistd.h>`). Si usas cualquiera de las dos directamente, tu código deja de ser portable — y recuerda que tu README debe garantizar compatibilidad Windows/macOS. `std::this_thread::sleep_for(std::chrono::seconds(1))` es parte del estándar de C++ (C++11 en adelante) y funciona igual sin importar el sistema operativo. Es la misma filosofía que ya aplicaste al preferir `<random>` sobre `rand()`: usar la librería estándar moderna en vez de una alternativa dependiente de la plataforma.

---

## Paso 7 — El `Makefile`: le decimos al compilador dónde buscar

Con `include/` y `src/` separados, necesitamos ajustar el `Makefile` para que sepa dónde está cada cosa:

```makefile
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude

SRC = src/main.cpp src/Automovil.cpp src/Semaforo.cpp src/Conductor.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = simulador

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
```

> **Nota técnica:** la parte que hace posible todo lo del Paso 0 es `-Iinclude` dentro de `CXXFLAGS` — esa bandera se aplica a *cada* archivo que se compila (tanto para generar los `.o` como para el enlazado final), así que cualquier `#include "Automovil.h"` en cualquier `.cpp` de `src/` va a encontrar el header sin necesidad de rutas relativas. Si decidiste dejar `main.cpp` en la raíz del proyecto en vez de `src/`, solo cambia esa línea de `SRC` por `main.cpp src/Automovil.cpp ...` — el resto del `Makefile` no necesita ningún otro ajuste.

---

## Paso 8 — Mini-reto opcional (si te sobra tiempo)

Si tu equipo termina antes de que se acabe la sesión, intenta uno de estos (no son obligatorios):

1. Agrega un tercer conductor con su propio auto y regístralo en el semáforo — no deberías tener que modificar `Semaforo` ni `Automovil` para lograrlo. Si tuviste que tocar esas clases, algo en el diseño no quedó del todo desacoplado.
2. Haz que `Automovil::arrancar(int potencia)` valide que `potencia` esté en un rango razonable (por ejemplo, entre 5 y 120) antes de asignarla.
3. Agrega un contador **estático** en `Automovil` (`static int autosCreados`) que se incremente en cada constructor, y un método estático `getTotalAutos()`. Esto te da un adelanto de qué significa que un dato "le pertenezca a la clase" y no a cada objeto individual.

---

## Checklist antes de cerrar la práctica

- [ ] Cada clase compila con `include guards` (`#pragma once`)
- [ ] `Semaforo` y `Automovil` están dentro de sus namespaces correspondientes
- [ ] `Conductor` nunca incluye a `Semaforo`, ni `Semaforo` conoce a `Automovil` directamente
- [ ] El `while(true)` en `main` no contiene lógica de negocio, solo control de tiempo
- [ ] Puedes explicar, en una frase, la responsabilidad de cada una de las 3 clases

---

## Quiz de cierre

**1.** ¿Cuál es el propósito principal de usar `namespace ControlVial` y `namespace Vehiculos` en este proyecto?
* A) Hacer que el programa compile más rápido
* B) Organizar el código en dominios conceptuales relacionados y evitar colisión de nombres
* C) Es un requisito obligatorio para usar clases en C++
* D) Permite que las clases hereden entre sí automáticamente

**2.** ¿Por qué se usa `enum class` en vez de un `enum` tradicional para `EstadoSemaforo`?
* A) `enum class` es más rápido en tiempo de ejecución
* B) `enum` tradicional no existe en C++ moderno
* C) `enum class` evita conversiones implícitas a `int` y obliga a calificar el valor con el nombre del tipo
* D) No hay ninguna diferencia real entre ambos

**3.** En la clase `Automovil`, `arrancar()` y `arrancar(int potencia)` son un ejemplo de:
* A) Herencia
* B) Sobrecarga de funciones (mismo nombre, distinta firma)
* C) Polimorfismo con funciones virtuales
* D) Sobrecarga de operadores

**4.** ¿Por qué `Semaforo.h` declara `class Conductor;` en vez de incluir `"Conductor.h"` directamente?
* A) Porque Conductor no es una clase válida en C++
* B) Es un error de sintaxis que no debería estar ahí
* C) Es una declaración adelantada: el header solo necesita saber que el tipo existe, no su implementación completa
* D) Porque Conductor y Semaforo no pueden estar en el mismo programa

**5.** ¿Qué representa físicamente el puntero `actual` dentro de la clase `Semaforo`?
* A) Una copia del color actual
* B) Una dirección de memoria que apunta a una posición dentro del arreglo `estados[3]`
* C) Un índice numérico independiente del arreglo
* D) Una referencia al conductor más cercano

**6.** ¿Por qué `Semaforo` mantiene un `vector<Conductor*>` pero `Conductor` solo tiene un `Automovil*` (sin vector)?
* A) Es una elección arbitraria sin relación con el diseño
* B) Porque los vectores no pueden almacenar punteros a Automovil
* C) Porque la relación Semaforo-Conductor es uno a muchos, mientras que Conductor-Automovil es uno a uno
* D) Porque Conductor no necesita relacionarse con ningún Automovil

**7.** ¿Cuál de las siguientes afirmaciones describe mejor la responsabilidad de `Conductor` en este diseño?
* A) Cambiar el color del semáforo
* B) Almacenar el estado interno del semáforo
* C) Traducir el aviso del semáforo en una decisión sobre qué ordenarle a su propio auto
* D) Ejecutar directamente el arranque o detenimiento sin pasar por Automovil

**8.** ¿Por qué `reaccionar(EstadoSemaforo estado)` recibe el parámetro por valor y no por referencia?
* A) Porque en C++ los enums nunca pueden pasarse por referencia
* B) Porque `EstadoSemaforo` es un tipo pequeño y copiarlo no tiene costo significativo
* C) Porque pasar por valor siempre es más rápido que por referencia, sin excepción
* D) Porque así lo exige la sintaxis de `enum class`

**9.** ¿Por qué se usa `std::this_thread::sleep_for` con `<chrono>` en vez de `sleep()`?
* A) `sleep()` ya no existe en C++
* B) `std::this_thread::sleep_for` es portable entre sistemas operativos, a diferencia de `sleep()`/`Sleep()`
* C) `sleep()` solo funciona con números negativos
* D) No hay ninguna diferencia práctica entre ambas opciones

**10.** ¿Qué principio de diseño se refleja en que `main()` no contenga ningún `if` sobre el color del semáforo?
* A) Herencia múltiple
* B) Sobrecarga de operadores
* C) Responsabilidad única: la lógica de decisión vive en los objetos, no en el punto de entrada del programa
* D) Encapsulamiento de atributos privados

---

*Fin de la práctica guiada.*
