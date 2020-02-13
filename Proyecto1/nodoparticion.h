#ifndef NODOPARTICION_H
#define NODOPARTICION_H

#include "stdio.h"
#include "string.h"

class NodoParticion
{
public:
    char nombrePart[16];
    int byteInicio;
    int tamano;
    char nombre[6];
    char tipo;
    char path[200];
    NodoParticion *siguiente;
    NodoParticion(char* nombre, int byteInicio,int tamano,char*nombrePart,char tipo,char* path);
    NodoParticion();
};

#endif // NODOPARTICION_H
