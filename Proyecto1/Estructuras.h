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


typedef struct{
    int s_filesystem_type;
    unsigned int s_inodes_count;
    unsigned int s_blocks_count;
    unsigned int s_free_inodes_count;
    unsigned int s_free_blocks_count;
    int s_mtime;
    int s_umtime;
    int s_mnt_count;
    const int s_magic = 0xef53;
    unsigned int s_inode_size;
    unsigned int s_block_size;
    unsigned int s_free_inode_count;
    unsigned int s_free_block_count;
    unsigned int s_first_ino;
    unsigned int s_first_blo;
    unsigned int s_bm_inode_start;
    unsigned int s_bm_block_start;
    unsigned int s_inode_start;
    unsigned int s_block_start;
} SuperBloque;

typedef struct{
    int i_uid;  //UID USUARIO
    int i_gid;  //GID GRUPO
    int i_size;
    time_t i_atime; //Lectura
    time_t i_ctime; //Creacion
    time_t i_mtime; //Modificacion
    int i_block[15];
    char i_type;     // 1 archivo 0 carpeta
    char i_perm_escritura;
    char i_perm_lectura;
    char i_perm_ejecucion;//Permisos 664
    char i_extra;

} iNodo;

typedef struct{
    char log_tipo;
    char log_tipo_operacion;
    time_t log_fecha;
    char log_nombre[12];
    char log_path[200];
    char contenidousr[100];
    char contenido;
    int log_propietario;
    int log_grupo;
    int ugo;
} Journal;

typedef struct{
    char status;
} Bitmap;

typedef struct{
    char b_name[12];
    int b_inodo;/*apuntador a un inodo asociado al archivo o carpeta*/
} Content;
typedef struct{
    Content b_content[4];
} BloqueCarpeta;

typedef struct{
    char b_content[64];
} BloqueArchivo;

typedef struct{
    int b_pointers[16];
} BloqueApuntador;

#endif // ESTRUCTURAS_H
