#include "listaparticion.h"

ListaParticion::ListaParticion()
{
    cabeza=nullptr;
}

void ListaParticion::insertar(char *nombre, int byteInicio, int tamano, char *nombrePart, char tipo,char* path){
    NodoParticion* nuevo = new NodoParticion(nombre,byteInicio,tamano,nombrePart,tipo,path);
    if(cabeza!=nullptr){
        NodoParticion* aux = this->cabeza;
        while(aux->siguiente){
            aux = aux->siguiente;
        }
        aux->siguiente=nuevo;
    }else{
        cabeza=nuevo;
    }
}
