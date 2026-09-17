#pragma once

#include <vector>
#include "EstadoSemaforo.h"

class Conductor;

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
