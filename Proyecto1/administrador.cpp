#include "administrador.h"
#define TAMANOBYTE 1024
Administrador::Administrador()
{
    noDiscos = 0;
    this->listaDisco = new ListaDisco();
//    this->sesion = new Sesion();

}

void Administrador::crearDisco(Funcion *funcion){
    funcion->getName();
    string absPath;
    string raidName;
    absPath = funcion->path;
    if(funcion->opciones[0]==1&&funcion->opciones[3]==1&&funcion->opciones[6]==1){
        if (funcion->size>0) {
            int i =0;
            raidName = funcion->getRaidName();
            char fit = 'f';
            if(funcion->opciones[1]==1){
                fit=funcion->fit;
            }
            /*creamos el dijkito*/
            cout<<"Inicia creación de disco"<<endl;
            MBR mbr;
            /*fecha y hora*/
            mbr.mbr_fecha_creacion = time(0);
            /*Identificador de disco*/
            mbr.mbr_disk_signature=noDiscos++;
            /*Tamano en bytes*/
            if(funcion->opciones[2]==1){
                if(funcion->unit=='k')
                {
                    mbr.mbr_tamano=funcion->size*TAMANOBYTE;
                }
                else
                {
                    mbr.mbr_tamano=funcion->size*(TAMANOBYTE*TAMANOBYTE);
                }
                /*FIXED UNIT*/

            }else{
                mbr.mbr_tamano=funcion->size*TAMANOBYTE*TAMANOBYTE;
            }

            /*Verificamos que el filename sea extensión disk*/
            if(this->valExtension(funcion)==1){
                /*Damos formato a las particiones*/
                for (i=0;i<4;i++) {
                    mbr.particiones[i].part_fit='f';
                    strcpy(mbr.particiones[i].part_name,"");
                    mbr.particiones[i].part_size=0;
                    mbr.particiones[i].part_start=0;
                    /*d significa disabled*/
                    mbr.particiones[i].part_status='d';
                    mbr.particiones[i].part_type=0;
                }

                i=0;
                absPath=funcion->getAbsPath();
                /*se crea el directorio*/
                char comando1[200];
                string tokenActual;
                strcpy(comando1,"mkdir '");
                //absPath=this->getAbsPath(funcion);
                strcat(comando1,absPath.data());
                strcat(comando1,"' -p");
                cout<<"Comando a ejecutar: "<<comando1<<endl;
                system(comando1);
                char comando2[100];
                char comando3[103];
                strcpy(comando2,absPath.data());
                strcat(comando2,funcion->fileName.data());
                strcpy(comando3,absPath.data());
                strcat(comando3,raidName.data());

                cout<<"Abriendo archivo.. "<<comando2<<endl;
                /***********Creación del disco como tal*************/
                FILE *archivo=fopen(comando2,"wb");
                FILE *archivoRaid = fopen(comando3,"wb");
                if(archivo!=nullptr){
                    int fin = (mbr.mbr_tamano/TAMANOBYTE);
                    char buffer[TAMANOBYTE];
                    for (i=0;i<TAMANOBYTE;i++) {
                        buffer[i]='0';
                    }
                    i=0;
                    while(i!=fin){
                        fwrite(&buffer,TAMANOBYTE,1,archivo);
                        fwrite(&buffer,TAMANOBYTE,1,archivoRaid);
                        i++;
                    }
                    fclose(archivo);
                    fclose(archivoRaid);
                }else {
                    cout<<"Value of errno: "<<errno<<endl;
                    cout<<"Error al abrir archivo:"<<comando2<<endl;
                }
                archivo = fopen(comando2,"rb+");
                archivoRaid = fopen(comando3,"rb+");
                if(archivo!=nullptr){
                    fseek(archivo,0,SEEK_SET);
                    fseek(archivoRaid,0,SEEK_SET);
                    fwrite(&mbr,sizeof (MBR),1,archivoRaid);
                    fwrite(&mbr,sizeof (MBR),1,archivo);
                    fclose(archivo);
                    fclose(archivoRaid);
                    cout<<"Disco creado con exito"<<endl;
                }else{
                    cout<<"Error al escribir archivo"<<comando2<<endl;
                }
            }else {
                cout<<"Error al crear disco, no tiene terminación .disk"<<endl;
            }
        }else{
            cout<<"Error, el tamaño del disco no puede ser menor a 1"<<endl;
        }
    }else {
        cout<<"Error al crear partición, no están todos los campos necesarios"<<endl;
    }
}

void Administrador::crearParticion(Funcion *funcion){

}

int Administrador::valExtension(Funcion*funcion){

}

void Administrador::eliminarDisco(Funcion *funcion){

}

void Administrador::montarDisco(Funcion *funcion){

}

void Administrador::desmontarDisco(Funcion *funcion){

}

void Administrador::reportes(Funcion *funcion){

}
