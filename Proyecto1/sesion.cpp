#include "sesion.h"

Sesion::Sesion()
{
    this->usrid = -1;
    this->idPart = "";
    this->groupid = -1;
    this->tipo = -1;
    this->usuario="";
    this->contrasena="";
}

void Sesion::iniciarSesion(std::string usuario, std::string contrasena, std::string idPart){
    this->usuario = usuario;
    this->contrasena = contrasena;
    this->idPart = idPart;
}

void Sesion::cerrarSesion(){
    this->usrid = -1;
    this->idPart = -1;
    this->groupid = -1;
    this->tipo = -1;
    this->usuario="";
    this->contrasena="";
}
