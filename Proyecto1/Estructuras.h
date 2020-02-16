#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H
#include <string>

using namespace std;

typedef struct{
    int part_start; //byte en el disco donde inicia la partición
    int part_size; //contiene el tamaño total de la partición
    char part_status;// indica si está activa
    char part_type; //si es primaria o extendida
    char part_fit; //Tipo de ajuste
    char part_name[16];//nombre
} Particion;

typedef struct{
    int part_start; //byte en el disco donde inicia la partición
    int part_size; //contiene el tamaño total de la partición
    int part_next; //si es primaria o extendida
    char part_status;// indica si está activa
    char part_fit; //Tipo de ajuste
    char part_name[16];//nombre
} EBR;

typedef struct{
    int mbr_tamano;
    time_t mbr_fecha_creacion;
    int mbr_disk_signature;
    char disk_fit;
    Particion particiones[4];
} MBR;

typedef struct NodoFit{
    int inicio;
    int size;
    NodoFit* siguiente;
} NodoFit;

#endif // ESTRUCTURAS_H
