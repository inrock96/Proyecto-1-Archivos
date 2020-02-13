#ifndef NODODISCO_H
#define NODODISCO_H


#include "listaparticion.h"
#include "nodoparticion.h"

class NodoDisco
{
public:
    int cuenta;
    char path[120];
    char letra;
    ListaParticion *particiones;
    NodoDisco* siguiente;
    NodoDisco(char* path, char letra);
    NodoDisco();
  };

#endif // NODODISCO_H
