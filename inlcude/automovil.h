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
