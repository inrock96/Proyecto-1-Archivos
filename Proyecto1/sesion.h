#ifndef SESION_H
#define SESION_H

#include "string"
#include "string.h"

class Sesion
{
public:
    std::string usuario;
    std::string contrasena;
    char idPart[5];
    int groupid;
    int usrid;
    int tipo;
    Sesion();
    void iniciarSesion(std::string usuario,std::string contrasena,std::string idPart);
    void cerrarSesion();
};
#endif // SESION_H
