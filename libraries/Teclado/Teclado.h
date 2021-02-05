
#ifndef TECLADO
#define TECLADO

class Teclado {
    public:
        static const int BOTONES_MAXIMOS = 4;

        Teclado(const int pines[BOTONES_MAXIMOS], int cantidad, unsigned int duracionRebote = 20);

        boolean haPresionado(int boton) { return botonPresionado == boton; }
        boolean alPresionar(int boton) { return botonAlPresionar == boton; }
        boolean alLiberar(int boton) { return botonAlLiberar == boton; }
        
        void actualizar();

    private:
        int pines[Teclado::BOTONES_MAXIMOS];
        int total;
        unsigned int duracionRebote;

        int botonPresionado;
        int botonAlPresionar;
        int botonAlLiberar;
        int estadoBoton;

        unsigned long tiempoActualizacion = 0;
        unsigned long ultimoTiempoActualizacion = 0;
        unsigned long botonTiempoPresionado = 0;
        unsigned long botonLiberadoTiempoPresionado = 0;
        int ultimoBotonRebote = -1;
        unsigned long ultimoTiempoBotonRebote = 0;

        int obtenerBotonPresionado();
        boolean botonRebote(int pin);
};

#endif