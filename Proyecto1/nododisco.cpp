#include "nododisco.h"

NodoDisco::NodoDisco(char* path, char letra)
{
    strcpy(this->path,path);
    this->letra = letra;
    this->cuenta=1;
    this->siguiente=nullptr;
    this->particiones = new ListaParticion();
}

NodoDisco::NodoDisco(){
    this->particiones= new ListaParticion();
    this->cuenta= -1;
    this->siguiente=nullptr;
}

