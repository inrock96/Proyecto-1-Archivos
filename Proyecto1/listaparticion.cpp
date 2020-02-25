#include "listaparticion.h"
#include "iostream"
ListaParticion::ListaParticion()
{
    cabeza=nullptr;
}

void ListaParticion::insertar(char *nombre, int byteInicio, int tamano, char *nombrePart, char tipo,char* path){
    NodoParticion* nuevo = new NodoParticion(nombre,byteInicio,tamano,nombrePart,tipo,path);
    bool igual=false;
    if(cabeza!=nullptr){
        NodoParticion* aux = this->cabeza;
        while(aux->siguiente){
            if(aux->nombrePart==nombrePart)
                igual = true;
            aux = aux->siguiente;
        }
        if(!igual)
            aux->siguiente=nuevo;
        else
            std::cout<<"Esa "<<std::endl;
    }else{
        cabeza=nuevo;
    }
}
