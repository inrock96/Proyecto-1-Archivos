#ifndef INTERPRETE_H
#define INTERPRETE_H

#include "Estructuras.h"
#include "administrador.h"
#include "funcion.h"
#include "stdio.h"
#include "string.h"
#include <iostream>
#include <fstream>
#include <algorithm>
class Interprete
{
public:
    /* Atributos*/
    Funcion *funcion;
    Administrador *admin;
    /* Métodos */
    Interprete();
    void ejecutar(string entrada);
    bool esEspacio(char caracter);
    bool esId(char caracter);
    bool esNumero(char caracter);
    bool esLetra(char caracter);
    int getParam(string token);
    int getOpcion(string token);
    void ejecutarComando(Funcion *funcion);
};

#endif // INTERPRETE_H
