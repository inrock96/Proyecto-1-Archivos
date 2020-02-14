#ifndef LISTADISCO_H
#define LISTADISCO_H
#include "nododisco.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "iostream"
class ListaDisco
{
public:
    char letra;
    int cuenta;
    NodoDisco* cabeza;
    ListaParticion* lista;
    ListaDisco();
    void insertar(char *path,int byteInicio,int tamano,char* nombrePart,char tipo);
    void eliminar(std::string nombre);
    NodoParticion* existeId(std::string id);
    void mostrarLista();
};

#endif // LISTADISCO_H
