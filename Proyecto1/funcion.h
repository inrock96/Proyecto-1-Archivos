#ifndef FUNCION_H
#define FUNCION_H


#include <iostream>
#include "string"
using namespace std;
class Funcion
{
public:
    int funcion;
    int opciones[20];
    int size;
    int ugo;
    int add;
    char fit;
    char unit;
    char type;
    bool r;
    bool p;
    bool rf;
    string path;
    string eliminar;
    string nombre;
    string id[6];
    string usr;
    string pwd;
    string grp;
    string cont;
    string file[6];
    string dest;
    string fileName;
    string ruta;
    Funcion();
    void mostrar();
    string getRaidName();
    void getName();
    string getAbsPath();
};

#endif // FUNCION_H
