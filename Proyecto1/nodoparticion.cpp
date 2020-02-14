#include "nodoparticion.h"

NodoParticion::NodoParticion(char* nombre, int byteInicio,int tamano,char*nombrePart,char tipo, char* path)
{
    strcpy(this->nombre,nombre);
    strcpy(this->nombrePart,nombrePart);
    strcpy(this->path,path);
    this->byteInicio=byteInicio;
    this->tamano = tamano;
    this->tipo = tipo;
    this->siguiente=nullptr;
}

NodoParticion::NodoParticion(){
    this->tamano=-1;
    this->siguiente=nullptr;
}

