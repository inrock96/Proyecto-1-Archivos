#include "interprete.h"
#include <iostream>
using namespace std;
bool menuPrincipal();
void limpiarVar(char *var, int n);
Interprete *interprete;
int main(int argc, char *argv[])
{
    interprete = new Interprete();
    while (!menuPrincipal()){

    }
    return 0;
}

bool menuPrincipal(){
    char comando[250];
    cout<<"Universidad de San Carlos de Guatemala"<<endl;
    cout<<"Facultad de Ingeniería"<<endl;
    cout<<"Escuela de Ciencias y Sistemas"<<endl;
    cout<<"Manejo e Implementación de Archivos"<<endl;
    cout<<"Proyecto 1"<<endl;
    cout<<"Inti Andrés Samayoa Ortíz"<<endl;
    cout<<"201504002"<<endl<<endl;
    cout<<"Sistema manejador de Archivos"<<endl;
    cout<<endl<<"Ingrese un comando:>>"<<endl;
    scanf(" %[^\n]",comando);
    if(strcmp(comando,"exit")!=0){
        interprete->ejecutar(comando);
        return false;
    }
    return true;

}
void limpiarVar(char *var,int n){
    for (int i = 0;i<n;i++) {
        var[i]='\0';
    }
}
