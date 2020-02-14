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
    if(funcion->opciones[0]==1&&funcion->opciones[3]==1){
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
        cout<<"Error al crear disco, no están todos los campos necesarios"<<endl;
    }
}

void Administrador::crearParticion(Funcion *funcion){
    if(funcion->opciones[3]==1&&funcion->opciones[6]==1){

        string raidName = funcion->getRaidName();
        char comando[100];
        char comando2[100];
        char fit = 'w';
        char unidad = 'k';
        char tipo = 'p';
        if(funcion->opciones[2]==1)
            unidad = funcion->unit;
        if(funcion->opciones[4]==1)
            tipo = funcion->type;
        if(funcion->opciones[1]==1)
            fit = funcion->fit;
        strcpy(comando,funcion->getAbsPath().data());
        strcpy(comando2,funcion->getAbsPath().data());
        strcat(comando,raidName.data());
        strcat(comando2,funcion->fileName.data());
        FILE *archivo = fopen(comando,"rb+");
        FILE *fRaid = fopen(comando2,"rb+");
        int cantPrimarias=0;
        int posicion=-1;
        bool hayExtendida=false;
        int tempPrimarias=0;
        bool error=false;
        if(archivo!=nullptr&&fRaid!=nullptr){
            /*****************************CREAR PARTICION*************************************/
            if(funcion->opciones[0]==1&&funcion->opciones[5]==0&&funcion->opciones[7]==0){
                MBR mbr,mbRaid;
                mbr = leerMBR(archivo);
                mbRaid = leerMBR(fRaid);
                int numDisco;
                char nombreFunc[16];
                strcpy(nombreFunc,funcion->nombre.data());
                cout<<"****DATOS DEL DISCO****"<<endl;
                cout<<"Id: "<<mbr.mbr_disk_signature<<endl;
                cout<<"Tamaño: "<<mbr.mbr_tamano<<endl;
                for (numDisco=3;numDisco>=0;numDisco--) {
                    if(mbr.particiones[numDisco].part_status=='d'){
                        posicion=numDisco;
                        cout<<"Partición en posición: "<<posicion<<" vacía"<<endl;
                    }else{
                        int k=0;
                        int l=0;
                        if(funcion->nombre.size()<16)
                            nombreFunc[funcion->nombre.size()]='\0';
                        else
                            nombreFunc[15]='\0';
                        /*Comparamos el nombre*/
                        while(nombreFunc[k]!=NULL){
                            if(mbr.particiones[numDisco].part_name[k]==nombreFunc[k]){
                                l++;
                            }
                            k++;
                        }
                        if(l==k&&mbr.particiones[numDisco].part_status=='e'){
                            error=true;
                            cout<<"Error, ya existe una partición con ese nombre"<<endl;
                        }
                        if(mbr.particiones[numDisco].part_type=='p')
                            cantPrimarias++;
                        else if(mbr.particiones[numDisco].part_type=='e'&&hayExtendida==false)
                            hayExtendida=true;
                        else{
                            cout<<"Hay 2 extendidas :o"<<endl;
                        }
                    }
                }//Fin recorrido particiones
                cout<<"Particiones primarias: "<<cantPrimarias<<endl;
                cout<<"Particiones extendidas: "<<hayExtendida<<endl;
                tempPrimarias=cantPrimarias;
                /************INICIA LA CREACION DE LA PARTICION***************/
                int tamanoPart;
                if(unidad=='b')
                    tamanoPart=funcion->size;
                else if(unidad=='k')
                    tamanoPart=funcion->size*TAMANOBYTE;
                else if(unidad=='m')
                    tamanoPart=funcion->size*TAMANOBYTE*TAMANOBYTE;
                if(tipo=='p')
                    cantPrimarias++;
                else if(tipo=='e'&&!hayExtendida)
                    hayExtendida=true;
                else if(tipo=='l'){

                }
                else{
                    cout<<"Errror, ya existe una extendida"<<endl;
                    error=true;
                }
                EBR ebr;

                if(cantPrimarias>3&&funcion->type=='p'){
                    error = true;
                    cout<<"Hay más de 3 primarias";
                }
                if(error==false){
                    /*CREAR LA PRIMARIA*/
                    if(tipo=='p'){
                        int byteInicio;
                        if(mbr.disk_fit=='f')
                            byteInicio = getFFByte(archivo,tamanoPart);
                        else if(mbr.disk_fit=='b'){
                            byteInicio = getFFByte(archivo,tamanoPart);/*getBFByte CHECK*/
                        }else{
                            byteInicio = getFFByte(archivo,tamanoPart);/*getWFByte CHECK*/
                        }
                        if(posicion!=-1){
                            if(byteInicio>-1){
                                mbr.particiones[posicion].part_status='e';
                                mbr.particiones[posicion].part_start=byteInicio;
                                mbr.particiones[posicion].part_fit=fit;
                                mbr.particiones[posicion].part_size=tamanoPart;
                                mbr.particiones[posicion].part_type=tipo;
                                strcpy(mbr.particiones[posicion].part_name,nombreFunc);

                                escribirMBR(archivo,mbr);
                                escribirMBR(fRaid,mbr);
                                int tam = tamanoPart;
                                fseek(archivo,byteInicio,SEEK_SET);
                                fseek(fRaid,byteInicio,SEEK_SET);
                                char c= 'x';
                                while(tam>0){
                                    fwrite(&c,1,1,archivo);
                                    fwrite(&c,1,1,fRaid);
                                    tam--;
                                }
                                cout<<"Partición primaria creada satisfactoriamente"<<endl;
                            }else{
                                cout<<"Error, no hay espacio libre suficiente"<<endl;
                            }
                        }else{
                            cout<<"Error al crear partición, ya existen todas las posibles particiones"<<endl;
                        }

                    }
                    /*CREAR EXTENDIDA*/
                    else if(tipo=='e'){
                        int byteInicio;
                        if(mbr.disk_fit=='f')
                            byteInicio = getFFByte(archivo,tamanoPart);
                        else if(mbr.disk_fit=='b'){
                            byteInicio = getFFByte(archivo,tamanoPart);/*CHECK getBFByte*/
                        }else{
                            byteInicio = getFFByte(archivo,tamanoPart);/*CHECK getWFByte*/
                        }
                        if(posicion!=-1){
                            if(byteInicio!=-1&&hayExtendida){

                                mbr.particiones[posicion].part_status='e';
                                mbr.particiones[posicion].part_start=byteInicio;
                                mbr.particiones[posicion].part_fit=fit;
                                mbr.particiones[posicion].part_size=tamanoPart;
                                mbr.particiones[posicion].part_type=tipo;
                                strcpy(mbr.particiones[posicion].part_name,nombreFunc);
                                escribirMBR(archivo,mbr);
                                escribirMBR(fRaid,mbr);
                                int tam = tamanoPart;
                                fseek(archivo,byteInicio,SEEK_SET);
                                fseek(fRaid,byteInicio,SEEK_SET);
                                char c= 'y';
                                while(tam>0){
                                    fwrite(&c,1,1,archivo);
                                    fwrite(&c,1,1,fRaid);
                                    tam--;
                                }
                                ebr.part_status = 'd';
                                ebr.part_fit = fit;
                                strcpy("extendida",nombreFunc);
                                ebr.part_size = 0;
                                ebr.part_start = byteInicio;
                                ebr.part_next = -1;
                                fseek(archivo,byteInicio,SEEK_SET);
                                fseek(fRaid,byteInicio,SEEK_SET);
                                fwrite(&ebr,sizeof (EBR),1,archivo);
                                fwrite(&ebr,sizeof (EBR),1,fRaid);
                                cout<<"Partición extendida creada satisfactoriamente"<<endl;
                            }else{
                                cout<<"Error, no hay espacio libre suficiente"<<endl;
                            }
                        }else{
                            cout<<"Error al crear partición, no hay espacio"<<endl;
                        }

                    }
                    /*LOGICA*/
                    else if(funcion->type=='l'){
                        if(hayExtendida){
                            for (int i=0;i<4;i++) {
                                if(mbr.particiones[i].part_type=='e')
                                    posicion=i;

                            }
                            if(mbr.particiones[posicion].part_size>tamanoPart){
                                int byteInicio = mbr.particiones[posicion].part_inicia;
                                fseek(archivo,byteInicio,SEEK_SET);
                                fread(&ebr,sizeof (EBR),1,archivo);
                                if(ebr.part_size>0){
                                    /*Ya existe una lógica*/
                                    while(ebr.part_next!=-1){
                                        fseek(archivo,ebr.part_next,SEEK_SET);
                                        fread(&ebr,sizeof (EBR),1,archivo);
                                    }
                                    int inicio = ebr.part_start+ebr.part_size;
                                    //CHECK AGREGAR FF BF Y WF A LOGICA
                                    EBR ebr2;
                                    int byteFinal = inicio+tamanoPart-1;
                                    if(byteFinal<=mbr.particiones[posicion].part_start+mbr.particiones[posicion].part_size-1){
                                        strcpy(ebr2.part_name,nombreFunc);
                                        ebr2.part_fit=fit;
                                        ebr2.part_next = -1;
                                        ebr2.part_size = tamanoPart;
                                        ebr2.part_start = inicio;
                                        ebr2.part_status='e';
                                        fseek(archivo,inicio,SEEK_SET);
                                        fseek(fRaid,inicio,SEEK_SET);
                                        fwrite(&ebr2,sizeof (EBR),1,archivo);
                                        fwrite(&ebr2,sizeof (EBR),1,fRaid);
                                        fseek(archivo,inicio+sizeof (EBR),SEEK_SET);
                                        fseek(fRaid,inicio+sizeof (EBR),SEEK_SET);
                                        char c ='z';
                                        int tam = tamanoPart-sizeof (EBR);
                                        while(tam>0){
                                            fwrite(&c,1,1,archivo);
                                            fwrite(&c,1,1,fRaid);
                                            tam--;
                                        }
                                        ebr.part_next = ebr2.part_start;
                                        fseek(archivo,ebr.part_start,SEEK_SET);
                                        fseek(fRaid,ebr.part_start,SEEK_SET);
                                        fwrite(&ebr,sizeof (EBR),1,archivo);
                                        fwrite(&ebr,sizeof (EBR),1,fRaid);
                                        cout<<"Partición lógica creada"<<endl;
                                    }else{
                                        cout<<"Error, la lógica supera el espacio restante de la extendida"<<endl;
                                    }
                                }else{
                                    /*Es la primera lógica*/
                                    int inicio = byteInicio+ebr.part_size;
                                    ebr.part_fit = fit;
                                    ebr.part_size = tamanoPart;
                                    ebr.part_status = 'e';
                                    strcpy(ebr.part_name,nombreFunc);
                                    fseek(archivo,byteInicio,SEEK_SET);
                                    fseek(fRaid,byteInicio,SEEK_SET);
                                    fwrite(&ebr,sizeof (EBR),1,archivo);
                                    fwrite(&ebr,sizeof (EBR),1,fRaid);
                                    int tam = tamanoPart-sizeof (EBR);
                                    fseek(archivo,inicio+sizeof (EBR),SEEK_SET);
                                    fseek(fRaid,inicio+sizeof (EBR),SEEK_SET);
                                    char c = 'z';
                                    while(tam>0){
                                        fwrite(&c,1,1,archivo);
                                        fwrite(&c,1,1,fRaid);
                                        tam--;
                                    }
                                    cout<<"Particion lógica creada con éxito";
                                }
                            }else{
                                cout<<"Error, el tamano de la logica es mayor a la extendida"<<endl;
                            }
                        }else{
                            cout<<"Error, no hay particion extendida para la creación lógica"<<endl;
                        }
                    }else{
                        cout<<"Esa opción no existe"<<endl;
                    }
                }else{
                    cout<<"NO se pudo crear la partición, error, hubo un error"<<endl;
                }
            }
            /*******************************ELIMINAR PARTICION********************************/
            else if(funcion->opciones[0]==0&&funcion->opciones[5]==1&&funcion->opciones[7]==0){
                eliminarParticion(funcion);
            }
            /*****************************MODIFICAR TAMANO PARTICION*************************/
            else if(funcion->opciones[0]==0&&funcion->opciones[5]==0&&funcion->opciones[7]==1){

            }else{
                cout<<"Error fdisk, size, delete y add son excluyentes"<<endl;
            }

            fclose(archivo);
            fclose(fRaid);
        }else{
            cout<<"No se encontró uno de los discos(original o raid) en: "<<funcion->path<<endl;
        }
    }else {
        cout<<"Error al crear partición, no están los campos mínimos necesarios"<<endl;
    }
}
void Administrador::eliminarParticion(Funcion *funcion){
    string raidName = funcion->getRaidName();
    char comando[100];
    char comando2[100];
    strcpy(comando,funcion->getAbsPath().data());
    strcpy(comando2,funcion->getAbsPath().data());
    strcat(comando,raidName.data());
    strcat(comando2,funcion->fileName.data());
    FILE *archivo = fopen(comando,"rb+");
    FILE *fRaid = fopen(comando2,"rb+");

    int j = -1;
    if(funcion->eliminar.compare("full")==0){
        MBR mbr = leerMBR(archivo);
        int i;
        for (i = 0;i<4;i++) {

            if(funcion->nombre.compare((mbr.particiones[i].part_name))==0){
                mbr.particiones->part_status='d';
                j=i;
                break;
            }
        }
        if(j!=-1){
            char c = '0';
            fseek(archivo,mbr.particiones[j].part_start,SEEK_SET);
            fseek(fRaid,mbr.particiones[j].part_start,SEEK_SET);
            for (i=0;i<mbr.particiones[j].part_size;i++) {
                fwrite(&c,1,1,archivo);
                fwrite(&c,1,1,fRaid);
            }
            escribirMBR(archivo,mbr);
            escribirMBR(fRaid,mbr);
            cout<<"Particion eliminada por completo"<<endl;
        }else{
            //Buscar en las lógicas CHECK
            cout<<"No hay partición con ese nombre"<<endl;
        }
    }else{
        MBR mbr=leerMBR(archivo);
        bool logica=false;
        for (int i =0;i<4;i++) {
            if(funcion->nombre.compare((mbr.particiones[i].part_name))==0){
                mbr.particiones->part_status='d';
                logica = true;
                break;
            }
        }
        if(logica){
            //CHECK buscamos para deshabilitar la lógica
        }
        escribirMBR(archivo,mbr);
        escribirMBR(fRaid,mbr);

    }
    fclose(archivo);
    fclose(fRaid);
}
void Administrador::addParticion(Funcion *funcion){

}

int Administrador::valExtension(Funcion*funcion){
    int cont=0;
    while(cont<funcion->path.size()){
        if(funcion->path[cont]=='.'){
            if(funcion->path[cont+1]=='d'&&funcion->path[cont+2]=='i'&&funcion->path[cont+3]=='s'&&funcion->path[cont+4]=='k'){
                return 1;
            }
        }
        cont++;
    }
    return 0;
}

void Administrador::eliminarDisco(Funcion *funcion){
    if(funcion->opciones[3]==1){
        /*Eliminar el disco*/
        const char* dosPuntosUve=funcion->path.data();
        if(remove(dosPuntosUve)==0){
            cout<<"Se eliminó "<<funcion->fileName<<" con exito"<<endl;
        }else{
            cout<<"Error al eliminar disco en: "<<funcion->path<<endl;
        }
    }else {
        cout<<"Error, no ingresó el parametro path"<<endl;
    }
}

void Administrador::montarDisco(Funcion *funcion){
    if(funcion->opciones[3]==1&&funcion->opciones[6]==1){
        FILE* file;
        char nombreFunc[16];
        char path[100];
        strcpy(path,funcion->path.data());
        strcpy(nombreFunc,funcion->nombre.data());
        if(funcion->nombre.size()>15){
            nombreFunc[15]='\0';
        }else{
            nombreFunc[funcion->nombre.size()]='\0';
        }
        file = fopen(path,"rb+");
        if(file!=nullptr){
            Particion p = getParticion(path,nombreFunc);
            if(p.part_status!='d'){
                listaDisco->insertar(path,p.part_start,p.part_size,p.part_name,p.part_type);
            }
            fclose(file);
        }else{
            cout<<"Error, no se pudo abrir el archivo"<<endl;
        }
    }else if (funcion->opciones[0]!=1&&funcion->opciones[1]!=1&&funcion->opciones[2]!=1&&funcion->opciones[3]!=1&&funcion->opciones[4]!=1&&funcion->opciones[5]!=1&&funcion->opciones[6]!=1&&funcion->opciones[7]!=1&&funcion->opciones[8]!=1&&funcion->opciones[9]!=1) {
        listaDisco->mostrarLista();
    }else {
        cout<<"Error al montar partición, no están todos los campos necesarios"<<endl;
    }

}

void Administrador::desmontarDisco(Funcion *funcion){
    if(funcion->opciones[8]){
        listaDisco->eliminar(funcion->id[0]);
    }else {
        cout<<"Error al montar partición, no están todos los campos necesarios"<<endl;
    }
}

void Administrador::reporteMBR(Funcion *funcion, MBR mbr){
    FILE* archivo = fopen("/home/mia/Reportes/mbr.dot","w");
    fprintf(archivo,"Digraph g{ \n node[shape = record];\n");
    fprintf(archivo, "struct1[shape = record, label = \"{");
    fprintf(archivo,"{  MBR  |    }|");
    fprintf(archivo,"{mbr_tamaño | %d }|",mbr.mbr_tamano);
    fprintf(archivo,"{mbr_fecha_creacion | %s}|",mbr.mbr_fecha_creacion);
    fprintf(archivo,"{mbr_disk_signature | %d}|",mbr.mbr_disk_signature);
    int i = 0;
    for(i= 0;i<4;i++){
        if(mbr.particiones[i].part_status=='e'){
            fprintf(archivo,"{part_status_%d | %c }|", i+1, mbr.particiones[i].part_status);
            fprintf(archivo,"{part_type_%d | %c }|", i+1, mbr.particiones[i].part_type);
            fprintf(archivo,"{part_fit_%d | %c }|", i+1, mbr.particiones[i].part_fit);
            fprintf(archivo,"{part_start_%d | %d}|", i+1, mbr.particiones[i].part_start);
            fprintf(archivo,"{part_size_%d | %d }|", i+1, mbr.particiones[i].part_size);
            fprintf(archivo,"{part_name_%d | %s }|", i+1, mbr.particiones[i].part_name);
        }
    }
    if(existeEBR(mbr)!=-1){
        int puntero = mbr.particiones[existeEBR(mbr)].part_start;
        EBR ebr;
        int n_struct=2;
        NodoParticion* part = listaDisco->existeId(funcion->id[0]);
        FILE* file = fopen(part->path,"rb+");
        fseek(file,puntero,SEEK_SET);
        fread(&ebr,sizeof(EBR),1,file);
        fprintf(archivo, "{  EBR  |  %d }|",n_struct-1);
        fprintf(archivo,"{  Nombre  |  Valor  }|");
        fprintf(archivo,"{ebr_size_%d | %d }|",n_struct-1,ebr.part_size);
        fprintf(archivo,"{ebr_name_%d | %s }|",n_struct-1,ebr.part_name);
        fprintf(archivo,"{ebr_start_%d | %d }|",n_struct-1,ebr.part_start);
        fprintf(archivo,"{ebr_status_%d | %c }|",n_struct-1,ebr.part_status);
        fprintf(archivo,"{ebr_next_%d | %d }|",n_struct-1,ebr.part_next);
        fprintf(archivo,"{ebr_fit_%d | %c }|",n_struct-1,ebr.part_fit);
        fprintf(archivo, "}\"];\n");
        puntero = ebr.part_next;
        n_struct++;
        while(puntero>0){
            fseek(file,puntero,SEEK_SET);
            fread(&ebr,sizeof(EBR),1,file);
            fprintf(archivo, "\nstruct%d[shape = record, label = \"{",n_struct-1);
            fprintf(archivo,"{  EBR  |  %d  }|",n_struct-1);
            fprintf(archivo,"{ebr_size_%d | %d }|",n_struct-1,ebr.part_size);
            fprintf(archivo,"{ebr_name_%d | %s }|",n_struct-1,ebr.part_name);
            fprintf(archivo,"{ebr_start_%d | %d }|",n_struct-1,ebr.part_start);
            fprintf(archivo,"{ebr_status_%d | %c }|",n_struct-1,ebr.part_status);
            fprintf(archivo,"{ebr_next_%d | %d }|",n_struct-1,ebr.part_next);
            fprintf(archivo,"{ebr_fit_%d | %c }|",n_struct-1,ebr.part_fit);
            fprintf(archivo, "}\"];\n");
            puntero = ebr.part_next;
            n_struct++;
        }
        fclose(file);
    }
    fprintf(archivo, "}\"];\n");
    fprintf(archivo, "}");
    fclose(archivo);
    generarReporte(funcion,"mbr");
}

void Administrador::reportes(Funcion *funcion){
    if(funcion->opciones[6]==1&&funcion->opciones[3]==1&&funcion->opciones[8]==1){
        if(funcion->nombre.compare("mbr")==0){
            MBR mbr;
            mbr = getListaMBR2(funcion);
            if(mbr.mbr_disk_signature!=-1){
                reporteMBR(funcion,mbr);
            }else{
                cout<<"No se encontró partición montada con ese id"<<endl;
            }
        }else if(funcion->nombre.compare("disk")==0){
            MBR mbr;
            mbr = getListaMBR2(funcion);
            if(mbr.mbr_disk_signature!=-1){
              //  reporteDisco(funcion,mbr);
            }
            else
                cout<<"No se encontró partición montada con ese id"<<endl;
        }else if(funcion->nombre.compare("bitacora")==0){
            //repJournaling(funcion);
        }else if(funcion->nombre.compare("bm_arbdir")==0){
            //repBMArbVirtual(funcion);
        }else if(funcion->nombre.compare("bm_detdir")==0){
            //repBMDetalle(funcion);
        }else if(funcion->nombre.compare("bm_inode")==0){
            //repBMInode(funcion);
        }else if(funcion->nombre.compare("bm_block")==0){
            //repBMBlock(funcion);
        }else if(funcion->nombre.compare("directorio")==0){
            //repDirectorio(funcion);
        }else if(funcion->nombre.compare("tree_file")==0){
            //repTree_file(funcion);
        }else if(funcion->nombre.compare("tree_directorio")==0){
            //repTree_Directorio(funcion);
        }else if(funcion->nombre.compare("tree_complete")==0){      //Primero
            //repTreeComplete(funcion);
        }else if(funcion->nombre.compare("sb")==0){        //fresh
            //repSuperBloque(funcion);
        }else if(funcion->nombre.compare("ls")==0){        //Hard
            //repLS(funcion);
        }
        else{
            cout<<"Error, ese reporte no existe papá"<<endl;
        }
    }else {
        cout<<"Error al crear reporte, no están todos los campos necesarios"<<endl;
    }
}

void Administrador::generarReporte(Funcion *funcion, string nombreRep){
    char cmd[200];
    //limpiarVar(cmd,200);
    int tamano=0;
    char path[50];
    strcpy(path,funcion->path.data());
    for (int i = 0;i<50;i++) {
        if(path[i]!='\0')
            tamano++;
        else{
            break;
        }
    }
    tamano--;
    crearDirectorioLinux(funcion->path,funcion);
    if(path[tamano]=='f'){
        sprintf(cmd,"dot -Tpdf  '/home/mia/Reportes/%s.dot' -o %s",nombreRep.data(),funcion->path.data());
        system(cmd);
    }else if(path[tamano-1]=='p'){
        sprintf(cmd,"dot -Tjpg  '/home/mia/Reportes/%s.dot' -o %s",nombreRep.data(),funcion->path.data());
        system(cmd);
    }else if(path[tamano-1]=='n'){
        sprintf(cmd,"dot -Tpng  '/home/mia/Reportes/%s.dot' -o %s",nombreRep.data(),funcion->path.data());
        system(cmd);
    }
    else{
        printf("formato no reconocido\n");
    }
}

int Administrador::getFFByte(FILE *file, int tamano){
    fseek(file,sizeof(MBR),SEEK_SET);
    int actual = ftell(file);
    int espacioLibre=0;
    char comp;
    while(!feof(file)&&espacioLibre<tamano){
        comp = fgetc(file);
        if(comp=='0')
            espacioLibre++;
        else{
            espacioLibre=0;
            actual = ftell(file);
        }
        if(espacioLibre>=tamano){
            return actual;
        }
    }
    return -1;
}

