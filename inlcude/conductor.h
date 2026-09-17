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
