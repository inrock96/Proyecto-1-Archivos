#ifndef USUARIO_H
#define USUARIO_H
#include "string"

class Usuario
{
public:
    std::string nombre;
    std::string contrasena;
    int grupo;
    int usuario;
    Usuario();
};

#endif // USUARIO_H
