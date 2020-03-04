#ifndef ADMINISTRADOR_H
#define ADMINISTRADOR_H

#include "funcion.h"
#include "listadisco.h"
#include "Estructuras.h"
#include "math.h"
#include "usuario.h"
class Administrador
{
public:
    int noDiscos;
    ListaDisco *listaDisco;
    char cont[1000000];
    char pathTrampa[200];
    //Sesion *sesion;
    Administrador();
    void crearDisco(Funcion *funcion);
    void eliminarDisco(Funcion *funcion);
    void crearParticion(Funcion *funcion);
    void eliminarParticion(Funcion*funcion);
    void addParticion(Funcion*funcion);
    void montarDisco(Funcion *funcion);
    void desmontarDisco(Funcion *funcion);
    void reportes(Funcion *funcion);
    void formatear(Funcion *funcion);
    void login(Funcion *funcion);
    void logout(Funcion *funcion);
    void crearGrupo(Funcion *funcion);
    void eliminarGrupo(Funcion *funcion);
    void crearUsr(Funcion *funcion);
    void eliminarUsr(Funcion *funcion);
    void chmod(Funcion *funcion);
    void crearArchivo(Funcion *funcion);
    void catArchivo(Funcion *funcion);
    void eliminarArchivo(Funcion *funcion);
    void editarArchivo(Funcion *funcion);
    void renombrarArchivo(Funcion *funcion);
    void crearDirectorio(Funcion *funcion);
    void copiarDir(Funcion *funcion);
    void moverDir(Funcion *funcion);
    void encontrarDir(Funcion *funcion);
    void cambiarDueno(Funcion *funcion);
    void cambiarGrp(Funcion *funcion);
    void pausar(Funcion *funcion);
    void recuperar(Funcion *funcion);
    void perdida(Funcion *funcion);
    void reporteMBR(Funcion* funcion,MBR mbr);
    void reporteDisco(Funcion* funcion,MBR mbr);
    void repJournaling(Funcion* funcion);
    void repBMArbVirtual(Funcion* funcion);
    void repBMDetalle(Funcion* funcion);
    void repBMBlock(Funcion* funcion);
    void repBMInode(Funcion* funcion);
    void repDirectorio(Funcion* funcion);
    void repTree_file(Funcion* funcion);
    void repTree_Directorio(Funcion* funcion);
    void repTreeComplete(Funcion* funcion);
    void repSuperBloque(Funcion* funcion);
    void repLS(Funcion* funcion);
private:
    int getEspacio(Particion particion,FILE* archivo,int add);
    int valExtension(Funcion*funcion);
    string getAbsPath(Funcion *funcion);
    int getBFByte(FILE* file, int tamano);
    int getFFByte(FILE* file, int tamano);
    int getWFByte(FILE* file, int tamano);
    MBR leerMBR(FILE* file);
    int getFFByteLogica(FILE* file, int tamano, int startExtendida, int finExtendida);
    int getWFByteLogica(FILE* file, int tamano, int startExtendida, int finExtendida);
    int getBFByteLogica(FILE* file, int tamano, int startExtendida, int finExtendida);
    void escribirMBR(FILE* file, MBR mbr);
    Particion getParticion(char* path,char*nombre);
    int existeEBR(MBR mbr);
    EBR getLogica(FILE* file,char* nombre);
    EBR getEBR(FILE* file);
    void aumentarMount(NodoParticion* particion);
    void aumentarUnMount(NodoParticion* particion);
    MBR getListaMBR2(Funcion *funcion);
    void crearDirectorioLinux(string path,Funcion*funcion);
    string getAbsPath(string path,Funcion*funcion);
    void generarReporte(Funcion *funcion,string nombreRep);
    int numeroEstructuras(int tamanoPart);
    SuperBloque crearSuperBloque(int bit_inicio,int tamano,NodoParticion *part);
    void escribirBitMap(int inicio,int n,FILE* archivo);
    int getNumeroBloques(int size);
    void escribirBloques(int noBloques, FILE* archivo,char* contenido, int tamano);
    int getUltimoNBloques(int cont);
    void escribirSuperBloque(char* path,SuperBloque super);
    SuperBloque getSuperBloque(char* ide);
    void horaAString(char* output,time_t t);
    Usuario *getUsuario(string usr, NodoParticion* part);
    void discoInterno(int bitActual, MBR mbr,FILE* archivo,NodoParticion* part);
    void discoEBR(int posicion, MBR mbr,FILE* archivo,NodoParticion* part,EBR ebr);
    bool hayLogicas(char* path);
    int numeroDirectorios(char* path);
    bool estaLoggeado(char* id);
//    void crearCarpeta(SuperBoot super,int posBitmap,string nombresDir[],int posActualCarpeta,int nEstructuras,int numeroCarpetas,NodoParticion* part);
//    void crearPadres(SuperBoot super, int posBtimap, string nombresDir[],int posActualCarpeta, int numeroCarpetas, NodoParticion* part,int nEstructuras);
//    void insertarCarpeta(SuperBoot super,Arbol_Virtual_Directorio nodo, int posBitmap, string nombresCarpetas[],int posActualCarpeta,char path[],int nEstructuras,int numeroCarpetas,NodoParticion* part);
    int getFirstFreeBit(int inicio,int nEstruct,FILE* archivo);
    int getPosJournal(int inicio, int nEstruct,FILE* archivo);
    string getFileName(char path[]);
//    void crearFile(SuperBoot super,int posBitmap,string nombresDir[],int posActualCarpeta,int nEstructuras,int numeroCarpetas,NodoParticion* part,char nombre[],char cont[],int size);
//    void insertarArchivo(SuperBoot super,Arbol_Virtual_Directorio nodo, int posBitmap, string nombresCarpetas[],int posActualCarpeta,char path[],int nEstructuras,int numeroCarpetas,NodoParticion* part,char nombre[],char cont[],int size);
    //void crearInodo(SuperBloque super,Arbol_Virtual_Directorio nodoAvd,Detalle_Directorio detalle,int nEstructuras,NodoParticion* part,int posBitmap,int size, char cont[],int apInodo,char nombre[]);
//    void insertarInodo(SuperBoot super,Arbol_Virtual_Directorio nodo, int posBitmap, string nombresCarpetas[],int posActualCarpeta,char path[],int nEstructuras,int numeroCarpetas,NodoParticion* part);
//    void crearBloque(SuperBoot super,int posBitmap,string nombresDir[],int posActualCarpeta,int nEstructuras,int numeroCarpetas,NodoParticion* part);
    void insertarBloque(SuperBloque super,iNodo nodo, int posBitmap, string nombresCarpetas[],int posActualCarpeta,char path[],int nEstructuras,int numeroCarpetas,NodoParticion* part);
    char * toArray(int numero);
//    void escribirJournal(Bitacora bitacora,char path[],SuperBoot super, int nEstructuras);
    void llenarContSize(int size,char cont[]);
    void escribirFileBitmap(FILE* archivo,FILE*archivoBM,int nEstructuras,int inicio);
//    void definirAVDDir(FILE* archivo, FILE* archivoAVD,int apActual,SuperBoot super);
//    void definirDDTree(FILE* archivo, FILE* archivoAVD,int apActual,SuperBoot super,char nombre[]);
//    void definirAVDTree(FILE* archivo, FILE* archivoAVD,int apActual,SuperBoot super);
//    void definirInodeTree(FILE* archivo, FILE* archivoAVD,int apActual,SuperBoot super);
//    void definirBlockTree(FILE* archivo, FILE* archivoAVD,int apActual,SuperBoot super);
//    void definirAVDTreeDir(FILE* archivo, FILE* archivoAVD,int apActual,SuperBoot super);
//    void definirDDDir(FILE* archivo, FILE* archivoAVD,int apActual,SuperBoot super,char nombre[]);
//    void definirInodeDir(FILE* archivo, FILE* archivoAVD,int apActual,SuperBoot super);
//    void definirBitacora(FILE* archivo, FILE* archivoAVD,int apActual,SuperBoot super);
    void limpiarVar(char *var,int n);
};

#endif // ADMINISTRADOR_H
