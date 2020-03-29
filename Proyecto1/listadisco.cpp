#include "listadisco.h"

ListaDisco::ListaDisco()
{
    this->letra='a';
    this->cabeza=nullptr;

}

void ListaDisco::insertar(char *path,int byteInicio,int tamano,char* nombrePart,char tipo){
    if(this->cabeza!=nullptr){
        bool sonIguales=false;
        NodoDisco* dm = this->cabeza;
        NodoDisco* anterior = new NodoDisco();
        while(dm){
            if(strcmp(dm->path,path)==0){
                sonIguales=true;
                break;
            }
            anterior = dm;
            dm = dm->siguiente;
        }
        if(!sonIguales){
            /*Se crea un nuevo disco con otra letra yes*/
            NodoDisco* nuevo = new NodoDisco(path,this->letra);
            char nombre[6];
            std::string nameNombre = "vd";
            nameNombre+=nuevo->letra;
            nameNombre+=std::to_string(nuevo->cuenta);
            strcpy(nombre,nameNombre.c_str());
            nuevo->particiones->insertar(nombre,byteInicio,tamano,nombrePart,tipo,path);
            nuevo->cuenta++;
            anterior->siguiente=nuevo;
            std::cout<<"\nPartición montada con id: "<<nombre<<std::endl<<std::endl;
            this->letra++;
        }else{
            std::string nameNombre="vd";
            nameNombre+=dm->letra;
            nameNombre+=std::to_string(dm->cuenta);
            char nombre[6];
            strcpy(nombre,nameNombre.c_str());
            dm->particiones->insertar(nombre,byteInicio,tamano,nombrePart,tipo,path);
            dm->cuenta++;
            std::cout<<"\nPartición montada con id: "<<nombre<<std::endl<<std::endl;
        }

    }else{
        std::string nameNombre;
        this->cabeza=new NodoDisco(path,this->letra);
        nameNombre="vd";
        nameNombre+=cabeza->letra;
        nameNombre+=std::to_string(cabeza->cuenta);
        char nombre[6];
        strcpy(nombre,nameNombre.c_str());
        cabeza->particiones->insertar(nombre,byteInicio,tamano,nombrePart,tipo,path);
        std::cout<<"\nPartición montada con id: "<<nombre<<std::endl<<std::endl;
        this->letra++;
        cabeza->cuenta++;
    }

}

void ListaDisco::mostrarLista(){
    NodoDisco * auxDisco = this->cabeza;
    int i = 0;
    while(auxDisco){
        NodoParticion* auxParticion=auxDisco->particiones->cabeza;
        while (auxParticion) {
            std::cout<<"Part: "<< auxParticion->nombrePart<<" Id"<<i++<<": "<<auxParticion->nombre<<" Path: "<<auxParticion->path<<std::endl;
            auxParticion = auxParticion->siguiente;
        }
        auxDisco = auxDisco->siguiente;
    }
}

void ListaDisco::eliminar(std::string nombre){
    NodoDisco* aux = this->cabeza;
    while (aux) {
        NodoParticion* nodito = aux->particiones->cabeza;
        NodoParticion* anterior = nullptr;
        while (nodito) {
            if(strcmp(nodito->nombrePart,nombre.c_str())==0){
                anterior->siguiente = nodito->siguiente;
                break;
            }
            anterior=nodito;
            nodito = nodito->siguiente;
        }
        aux = aux->siguiente;
    }
}

NodoParticion* ListaDisco::existeId(std::string id){
    NodoDisco * actual = this->cabeza;
    while(actual){
        NodoParticion* partActual = actual->particiones->cabeza;
        while(partActual){
            if(strcmp(partActual->nombre,id.c_str())==0){
                return partActual;
            }
            if(partActual==actual->particiones->cabeza){
                actual->particiones->cabeza = partActual->siguiente;
            }
            partActual = partActual->siguiente;
        }

        actual = actual->siguiente;
    }
    return nullptr;
}
