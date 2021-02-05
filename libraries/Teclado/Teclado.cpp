#include <Arduino.h>
#include "Teclado.h"

Teclado::Teclado(const int pines[], int cantidad, unsigned int duracionRebote)
{
    for(int x=0; x<cantidad; x++)
    {
        pinMode(pines[x], INPUT_PULLUP);
        this->pines[x] = pines[x];
    } 

    this->duracionRebote = duracionRebote;
    this->total = cantidad;
}

void Teclado::actualizar()
{
    botonAlPresionar = -1;
    botonAlLiberar = -1;
    ultimoTiempoActualizacion = tiempoActualizacion;
    tiempoActualizacion = millis();  

    int boton = obtenerBotonPresionado();

    if(botonRebote(boton) && boton != botonPresionado)
    {
        botonLiberadoTiempoPresionado = botonTiempoPresionado;

        if(boton != -1)
            botonTiempoPresionado = tiempoActualizacion;

        botonAlPresionar = boton;
        botonAlLiberar = botonPresionado;

        botonPresionado = boton;
    }
}

int Teclado::obtenerBotonPresionado()
{
    for(int x=0; x<total; x++)
    {
        estadoBoton = digitalRead(pines[x]);
        if (estadoBoton == LOW) {  
            return pines[x];
        }
    }
}

boolean Teclado::botonRebote(int boton)
{
    if(boton != ultimoBotonRebote)
        ultimoBotonRebote = tiempoActualizacion;

    ultimoBotonRebote = boton;
    return (tiempoActualizacion - ultimoBotonRebote > duracionRebote);
}
