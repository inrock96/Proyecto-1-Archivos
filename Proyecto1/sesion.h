#ifndef SESION_H
#define SESION_H

#include "string"

class Sesion
{
public:
    std::string usuario;
    std::string contrasena;
    std::string idPart;
    int lectura;
    int escritura;
    int ejecucion;
    int groupid;
    int usrid;
    int tipo;
    Sesion();
    void iniciarSesion(std::string usuario,std::string contrasena,std::string idPart);

};
#endif // SESION_H
