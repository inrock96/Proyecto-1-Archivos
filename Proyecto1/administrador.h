#ifndef ADMINISTRADOR_H
#define ADMINISTRADOR_H

#include "funcion.h"
#include <vector>
#include "listadisco.h"
#include "Estructuras.h"
#include "math.h"
#include "usuario.h"
#include "sesion.h"
#include "chrono"
#include "thread"
#include "fstream"
class Administrador
{
public:
    int noDiscos;
    ListaDisco *listaDisco;
    char cont[1000000];
    char pathTrampa[200];
    Sesion *sesion;
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
    void logout();
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
    void repBMBlock(Funcion* funcion);
    void repBMInode(Funcion* funcion);
    void repBlock(Funcion* funcion);
    void repInodo(Funcion* funcion);
    void repTree(Funcion* funcion);
    void repSuperBloque(Funcion* funcion);
    void repLS(Funcion* funcion);
    void repFile(Funcion* funcion);
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
    int numeroEstructuras(int tamanoPart, int tipo);
    SuperBloque crearSuperBloque(int bit_inicio,int tamano,NodoParticion *part,int tipo);
    void escribirBitMap(int inicio,int n,FILE* archivo);
    void escribirPosBitmap(int inicio, int posicion, char path[],char valor);
    int getNumeroBloques(int size);
    void escribirBloques(int noBloques, FILE* archivo,char* contenido, int tamano);
    int getUltimoNBloques(int cont);
    void escribirSuperBloque(char* path,SuperBloque sb, int inicio);
    SuperBloque getSuperBloque(char* ide);
    void horaAString(char* output,time_t t);
    Usuario *getUsuario(string usr, NodoParticion* part);
    void discoInterno(int bitActual, MBR mbr,FILE* archivo,NodoParticion* part);
    void discoEBR(int posicion, MBR mbr,FILE* archivo,NodoParticion* part,EBR ebr);
    bool hayLogicas(char* path);
    int numeroDirectorios(char* path);
    bool estaLoggeado(char* id);
    iNodo getInodo(char path[],int inicio, int pos);
    BloqueApuntador getApuntadorLibre(char path[],int inicio,int posActual,int tipoIndirecto);
    BloqueCarpeta getBloqueCarpeta(char path[],int inicio, int pos);
    BloqueArchivo getBloqueArchivo(char path[],int inicio, int pos);
    BloqueApuntador getBloqueApuntador(char path[],int inicio, int pos);
    int getBloqueCarpetaNombre(char path[],SuperBloque sb,iNodo inodo_actual,string nombre_directorio);
    int getBloqueCarpetaNombreIndirecto(char path[],SuperBloque sb,int bloqueApuntador,string nombre_directorio,int tipoIndirecto);

    void leerBMInodo(int inicio, int n,char* path,Bitmap *bmInodo);
    iNodo getInodoUsuarios(Sesion sesion);
    BloqueCarpeta carpetaInicial(int padre, int actual);
    void crearCarpeta(SuperBloque sb,int posBitmap,vector<string>array_directorios,int posActualCarpeta,int numero_directorios,NodoParticion* part,int nEstructuras);
    void crearPadres(SuperBloque sb, int posBtimap, vector<string> array_directorios,int posActualCarpeta, int numero_directorios, NodoParticion* part,int nEstructuras);
    void crearFile(SuperBloque sb, int posBitmap,string nombre_archivo, vector<string> array_directorios,int posActualCarpeta, int numero_directorios, NodoParticion* part, int nEstructuras,int tamano,string contenido);
    void insertarCarpeta(SuperBloque sb, int posBitmap, vector<string>array_directorios,int posActualCarpeta,int numero_directorios,NodoParticion* part,int nEstructuras, char path[],iNodo nodo);
    bool insertarCarpetaApuntador(SuperBloque sb, int posBitmap, vector<string>array_directorios,int posActualCarpeta,int numero_directorios,NodoParticion* part,int nEstructuras, char path[],int posApuntador,int tipoIndirecto);
    bool insertarFileApuntador(SuperBloque sb, int posBitmap, string nombre_archivo, NodoParticion *part, int nEstructuras, char *path, int posApuntador, int tipoIndirecto,int tamano, string contenido);
    void insertarFile(SuperBloque sb, int posBitmap, vector<string>array_directorios,string nombre_archivo,int posActualCarpeta,int numero_directorios,NodoParticion* part,int nEstructuras, char path[],iNodo nodo,int tamano,string contenido);
    void crearBloques(SuperBloque sb, int posBitmap, iNodo inodo_archivo,int nEstructuras,char path[],int tamano, string contenido);
    void insertarBloqueArchivo(SuperBloque sb, iNodo inodo_archivo,int nEstructuras,char path[],vector<BloqueArchivo> bloques,int bloque_actual);

    int getFirstFreeBit(int inicio,int nEstruct,char path[]);
    int getPosJournal(int inicio, int nEstruct,FILE* archivo);
    string getFileName(char path[]);
    iNodo nuevoInodo(int tamano,char tipo);
    bool validarPermisoEscritura(iNodo inodo_actual);
    void escribirBloqueCarpeta(BloqueCarpeta carpeta,char*path,SuperBloque sb, int posicion);
    void escribirBloqueApuntador(BloqueApuntador bloque_apuntador,char*path,SuperBloque sb, int posicion);
    void escribirInodo(iNodo inodo,char* path, SuperBloque sb,int posicion);
//    void crearFile(SuperBoot super,int posBitmap,string nombresDir[],int posActualCarpeta,int nEstructuras,int numeroCarpetas,NodoParticion* part,char nombre[],char cont[],int size);
//    void insertarArchivo(SuperBoot super,Arbol_Virtual_Directorio nodo, int posBitmap, string nombresCarpetas[],int posActualCarpeta,char path[],int nEstructuras,int numeroCarpetas,NodoParticion* part,char nombre[],char cont[],int size);
    //void crearInodo(SuperBloque super,Arbol_Virtual_Directorio nodoAvd,Detalle_Directorio detalle,int nEstructuras,NodoParticion* part,int posBitmap,int size, char cont[],int apInodo,char nombre[]);
//    void insertarInodo(SuperBoot super,Arbol_Virtual_Directorio nodo, int posBitmap, string nombresCarpetas[],int posActualCarpeta,char path[],int nEstructuras,int numeroCarpetas,NodoParticion* part);
//    void crearBloque(SuperBoot super,int posBitmap,string nombresDir[],int posActualCarpeta,int nEstructuras,int numeroCarpetas,NodoParticion* part);
    void insertarBloque(SuperBloque super,iNodo nodo, int posBitmap, string nombresCarpetas[],int posActualCarpeta,char path[],int nEstructuras,int numeroCarpetas,NodoParticion* part);
    char * toArray(int numero);
    void crearBloqueApuntador(char path[], int apActual, SuperBloque sb, int tipoIndirecto, int tipoBloque,int nEstructuras);

    void escribirJournal(Journal journal,char path[],SuperBloque super, int nEstructuras, int inicio);
    void llenarContSize(int size,char cont[]);
    void escribirFileBitmap(FILE* archivo,FILE*archivoBM,int nEstructuras,int inicio);
    void definirInodoTree(FILE* archivo, char path[],int apActual,SuperBloque sb);
//    void definirDDTree(FILE* archivo, FILE* archivoAVD,int apActual,SuperBoot super,char nombre[]);
    void definirIndirectoTree(FILE* archivo, char path[],int apActual,SuperBloque sb,int tipoIndirecto,int tipoBloque);
    void definirCarpetaTree(FILE* archivo, char path[],int apActual,SuperBloque sb);
    void definirArchivoTree(FILE* archivo, char path[],int apActual,SuperBloque sb);
//    void definirAVDTreeDir(FILE* archivo, FILE* archivoAVD,int apActual,SuperBoot super);
//    void definirDDDir(FILE* archivo, FILE* archivoAVD,int apActual,SuperBoot super,char nombre[]);
//    void definirInodeDir(FILE* archivo, FILE* archivoAVD,int apActual,SuperBoot super);
//    void definirBitacora(FILE* archivo, FILE* archivoAVD,int apActual,SuperBoot super);
    void limpiarVar(char *var,int n);
};

#endif // ADMINISTRADOR_H
