#ifndef LISTAPARTICION_H
#define LISTAPARTICION_H

#include "nodoparticion.h"
class ListaParticion
{
public:
    NodoParticion * cabeza;
    ListaParticion();
    void insertar(char* nombre, int byteInicio,int tamano,char*nombrePart,char tipo,char* path);
};

#endif // LISTAPARTICION_H
