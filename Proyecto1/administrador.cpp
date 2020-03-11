#include "administrador.h"
#define TAMANOBYTE 1024
Administrador::Administrador()
{
    noDiscos = 0;
    this->listaDisco = new ListaDisco();
    this->sesion = new Sesion();

}

int Administrador::numeroEstructuras(int tamanoPart, int tipo){
    double n;
    if(tipo==3){
        n = (tamanoPart - sizeof (SuperBloque)) /
                (4.0 + 3.0 * 64.0 + sizeof (iNodo) + sizeof (Journal));


    }else if(tipo==2){
        n = (tamanoPart - sizeof (SuperBloque)) /
                (4.0 + 3.0 * 64.0 + sizeof (iNodo));
    }

    return floor(n);
}
int Administrador::getUltimoNBloques(int cont){
    int n = cont;
    int nbloques =0;
    if(n<64){
        nbloques=1;
    }else{
        do{
            n=n-25;
            nbloques++;
        }while(n>64);
        nbloques++;
    }
    return  n;
}
SuperBloque Administrador::crearSuperBloque(int bit_inicio, int tamano, NodoParticion *part,int tipo){
    SuperBloque sb;

    int nEstructuras = numeroEstructuras(tamano,tipo);
    //Llenar superboot

    sb.s_filesystem_type=tipo;
    sb.s_inodes_count=0;
    sb.s_blocks_count=0;
    sb.s_free_inodes_count=nEstructuras;
    sb.s_free_blocks_count=3*nEstructuras;
    sb.s_mtime=time(0);
    sb.s_umtime=time(0);
    sb.s_mnt_count=1;
    sb.s_inode_size=sizeof (iNodo);
    sb.s_block_size=sizeof (BloqueArchivo);
    if(tipo==3){

        sb.s_bm_inode_start=bit_inicio+sizeof (SuperBloque)+nEstructuras*sizeof (Journal);
        sb.s_journal_start=bit_inicio+sizeof (SuperBloque);
    }else{
        sb.s_bm_inode_start=bit_inicio+sizeof (SuperBloque);
        sb.s_journal_start=-1;
    }

    sb.s_bm_block_start=sb.s_bm_inode_start+nEstructuras;
    sb.s_inode_start=sb.s_bm_block_start+3*nEstructuras;
    sb.s_block_start=sb.s_inode_start+nEstructuras*sizeof (iNodo);

    sb.s_first_blo=sb.s_block_start;
    sb.s_first_ino=sb.s_inode_start;
    return  sb;
}
int Administrador::getNumeroBloques(int size){
    int n = size;
    int nbloques =0;
    if(n<64){
        nbloques=1;
    }else{
        do{
            n=n-64;
            nbloques++;
        }while(n>64);
        nbloques++;
    }
    return  nbloques;
}
void Administrador::formatear(Funcion *funcion){
    if(true/*fs=3*/){
        if(funcion->opciones[8]==1){
            int format = 1;
            int fs =2;
            if(funcion->fs!=-1){
                fs = funcion->fs;
            }
            if(funcion->opciones[4]==1){
                if(funcion->eliminar.compare("fast")==0)
                    format=2;

            }
            NodoParticion* part = listaDisco->existeId(funcion->id[0]);
            if(part!=nullptr){
                char comando [100];
                strcpy(comando,part->path);
                cout<<"particion en path a formatear"<<part->path<<endl;
                FILE* archivo = fopen(comando,"rb+");
                if(archivo!=NULL){
                    int nEstructuras =numeroEstructuras(part->tamano,funcion->fs);
                    SuperBloque sb = crearSuperBloque(part->byteInicio,part->tamano,part,funcion->fs);
                    //Escribir superboot
                    fseek(archivo,part->byteInicio,SEEK_SET);
                    fwrite(&sb,sizeof (SuperBloque),1,archivo);
                    //Escribir bitmap Inodo
                    MBR mbr = leerMBR(archivo);
                    int posPart=-1;
                    for (int i=0;i<4;i++) {
                        if(strcmp(mbr.particiones[i].part_name,part->nombrePart)==0){
                            posPart = i;
                            break;
                        }
                    }
                    mbr.particiones[posPart].part_status='f';
                    int i;
                    //Escribe journal
                    int prueba ;
                    if(fs==3){
                        prueba = sb.s_journal_start;
                        Journal journal;
                        journal.log_tipo=-1;
                        journal.log_fecha = time(0);
                        journal.contenido = 1;
                        journal.log_tipo_operacion=2;
                        limpiarVar(journal.log_path,100);
                        limpiarVar(journal.log_nombre,24);
                        limpiarVar(journal.log_propietario,16);
                        for (i=0;i<nEstructuras;i++) {
                            fseek(archivo,prueba,SEEK_SET);
                            fwrite(&journal,sizeof (Journal),1,archivo);
                            prueba+=sizeof (Journal);
                        }
                    }
                    fclose(archivo);
                    archivo = fopen(part->path,"rb+");
                    if(format!=1){
                        //Fast
                        char users[100];
                        strcpy(users,"1,G,root\n1,U,root,root,201504002\n");
                    }else{
                        //Full
                        //Escribir bloques
                        BloqueArchivo b;
                        int i;
                        strcpy(b.b_content,"vacio");
                        fseek(archivo,sb.s_block_start,SEEK_SET);
                        int pruebB = sb.s_block_start;
                        for (i = 0;i<nEstructuras*3;i++) {
                            fseek(archivo,pruebB,SEEK_SET);
                            fwrite(&b,sizeof (BloqueArchivo),1,archivo);
                            pruebB+=sizeof (BloqueArchivo);
                        }
                        //Escribir inodos
                        iNodo inodo;
                        inodo.i_gid = 1;
                        inodo.i_uid=1;
                        inodo.i_perm_lectura=inodo.i_perm_ejecucion=inodo.i_perm_escritura=-1;
                        inodo.i_size=-1;
                        inodo.i_type=-1;
                        inodo.i_atime=-1;
                        inodo.i_ctime = -1;
                        inodo.i_mtime = -1;
                        for (int i=0;i<15;i++) {
                            inodo.i_block[i]=-1;
                        }
                        int pruebainodo = sb.s_inode_start;
                        fseek(archivo,sb.s_inode_start,SEEK_SET);
                        for (i=0;i<nEstructuras;i++) {
                            fseek(archivo,pruebainodo,SEEK_SET);
                            fwrite(&inodo,sizeof (iNodo),1,archivo);
                            pruebainodo+= sizeof (iNodo);
                        }
                        //
                    }
                    //escribirBitMap Inodo
                    escribirBitMap(sb.s_bm_inode_start,nEstructuras,archivo);
                    //Escribir bitmap DD
                    escribirBitMap(sb.s_bm_block_start,nEstructuras*3,archivo);
                    //Se crea la raíz del árbol de directorio
                    iNodo raiz;
                    raiz.i_ctime =raiz.i_mtime=raiz.i_atime= time(0);
                    //propietario
                    raiz.i_gid=1;
                    raiz.i_uid=1;
                    //permisos
                    raiz.i_perm_lectura=raiz.i_perm_ejecucion=raiz.i_perm_escritura=0;
                    for (i=0;i<15;i++) {
                        raiz.i_block[i]=-1;
                    }
                    raiz.i_size=0;
                    raiz.i_type=0;
                    raiz.i_block[0]=0;
                    //Se crea el bloque directorio donde se guardan los usuarios
                    BloqueCarpeta bcarpeta;
                    for (int i=0;i<4;i++) {
                        strcpy( bcarpeta.b_content[i].b_name,"");
                        bcarpeta.b_content[i].b_inodo=-1;
                    }
                    strcpy(bcarpeta.b_content[0].b_name,".");
                    bcarpeta.b_content[0].b_inodo=0;
                    strcpy(bcarpeta.b_content[1].b_name,"..");
                    bcarpeta.b_content[1].b_inodo=0;
                    strcpy(bcarpeta.b_content[2].b_name,"usuarios.txt");
                    bcarpeta.b_content[2].b_inodo=1;
                    //se crea el inodo para el archivo
                    iNodo i_usr;
                    i_usr.i_gid=1;
                    i_usr.i_uid=1;
                    i_usr.i_size=33;
                    i_usr.i_type=1;
                    i_usr.i_atime=i_usr.i_ctime=i_usr.i_mtime=time(0);
                    i_usr.i_perm_lectura=0;
                    i_usr.i_perm_ejecucion=0;
                    i_usr.i_perm_escritura=0;
                    for (int i = 0;i<15;i++) {
                        i_usr.i_block[i]=-1;
                    }
                    i_usr.i_block[0]=1;

                    BloqueArchivo b_usr;

                    strcpy(b_usr.b_content,"1,G,root\n1,U,root,root,201504002\n");
                    int cont = 0;
                    while (b_usr.b_content[cont]!=NULL) {
                        cont++;
                    }
                    int nbloques = getNumeroBloques(cont);
                    int nultimo = getUltimoNBloques(cont);
                    //Se llenan los bloques de información
                    sb.s_first_blo-=2;
                    sb.s_first_ino-=2;
                    sb.s_blocks_count+=2;
                    sb.s_inodes_count+=2;

                    //Bitacora

                    Journal j_raiz;
                    Journal j_usuarios;
                    if(fs==3){
                        j_raiz.log_fecha = j_usuarios.log_fecha = time(0);
                        strcpy(j_raiz.log_path,"/");
                        strcpy(j_usuarios.log_path,"/");
                        strcpy(j_usuarios.log_nombre,"usuarios.txt");
                        strcpy(j_raiz.log_nombre,"root");
                        j_raiz.log_tipo_operacion=0;
                        j_usuarios.log_tipo_operacion=0;
                        j_raiz.log_tipo=0;
                        j_usuarios.log_tipo=1;
                        j_raiz.contenido = 1;
                        j_usuarios.contenido=2;
                        fseek(archivo,sb.s_journal_start,SEEK_SET);
                        fwrite(&j_raiz,sizeof (Journal),1,archivo);
                        fwrite(&j_usuarios,sizeof (Journal),1,archivo);
                    }

                    char b1 ='1';
                    //escribir bitmap inodo
                    fseek(archivo,sb.s_bm_inode_start,SEEK_SET);
                    fwrite(&b1,1,1,archivo);
                    fwrite(&b1,1,1,archivo);
                    //escribir bitmap bloque
                    fseek(archivo,sb.s_bm_block_start,SEEK_SET);
                    fwrite(&b1,1,1,archivo);
                    fwrite(&b1,1,1,archivo);

                    //escribir inodos
                    fseek(archivo,sb.s_inode_start,SEEK_SET);
                    fwrite(&raiz,sizeof (iNodo),1,archivo);
                    fwrite(&i_usr,sizeof (iNodo),1,archivo);
                    //Escribir bloques
                    int iniciobb = sb.s_block_start;
                    fseek(archivo,iniciobb,SEEK_SET);
                    fwrite(&bcarpeta,sizeof (BloqueCarpeta),1,archivo);
                    fwrite(&b_usr,sizeof (BloqueArchivo),1,archivo);
                    fclose(archivo);
                    if(fs==3){
                        escribirJournal(j_raiz,part->path,sb,nEstructuras);
                        escribirJournal(j_usuarios,part->path,sb,nEstructuras);
                    }
                    //fin
                }
            }
        }
    }
}

void Administrador::escribirJournal(Journal journal, char *path, SuperBloque sb, int nEstructuras){
    FILE *archivo = fopen(path,"rb+");
    int bitacoralibre = getPosJournal(sb.s_journal_start,nEstructuras,archivo);
    fseek(archivo,bitacoralibre,SEEK_SET);
    fwrite(&journal,sizeof (Journal),1,archivo);
    fclose(archivo);
}
int Administrador::getPosJournal(int inicio, int nEstruct, FILE *archivo){
    int posActual =-1;
    fseek(archivo,inicio,SEEK_SET);
    Journal bit;
    for (int i =0;i<nEstruct;i++) {
        fread(&bit,sizeof(Journal),1,archivo);
        if(bit.contenido==0){
            posActual=ftell(archivo);
            break;
        }
    }
    return posActual-sizeof (Journal);
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
            mbr.disk_fit=fit;
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
            if(funcion->opciones[0]==1&&funcion->opciones[5]==-1&&funcion->opciones[7]==-1){
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
                int tamanoPart=-1;
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
                            byteInicio = getBFByte(archivo,tamanoPart);/*getBFByte CHECK*/
                        }else{
                            byteInicio = getWFByte(archivo,tamanoPart);/*getWFByte CHECK*/
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
                            byteInicio = getBFByte(archivo,tamanoPart);/*CHECK getBFByte*/
                        }else{
                            byteInicio = getWFByte(archivo,tamanoPart);/*CHECK getWFByte*/
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
                                strcpy(ebr.part_name,nombreFunc);
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
                                 int byteInicio = mbr.particiones[posicion].part_start;
                                 fseek(archivo,byteInicio,SEEK_SET);
                                 fseek(fRaid,byteInicio,SEEK_SET);
                                 fread(&ebr,sizeof (EBR),1,archivo);
                                 if(ebr.part_size>0){
                                     /*Ya existe una lógica*/
                                     while(ebr.part_next!=-1){
                                         fseek(archivo,ebr.part_next,SEEK_SET);
                                         fread(&ebr,sizeof (EBR),1,archivo);
                                     }
                                     int inicio = ebr.part_start+ebr.part_size;
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
                                         cout<<"*********************Partición lógica creada*************************"<<endl;
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
                                     cout<<"******************Particion lógica creada con éxito********************";
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
            else if(funcion->opciones[0]==-1&&funcion->opciones[5]==1&&funcion->opciones[7]==-1){
                eliminarParticion(funcion);
            }
            /*****************************MODIFICAR TAMANO PARTICION*************************/
            else if(funcion->opciones[0]==-1&&funcion->opciones[5]==-1&&funcion->opciones[7]==1){
                addParticion(funcion);
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

int Administrador::getFFByteLogica(FILE *file, int tamano,int startExtendida,int finExtendida){
    int i = startExtendida;
    fseek(file,sizeof(MBR),SEEK_SET);
    int actual = ftell(file);
    int espacioLibre=0;
    char comp;
    while(i<finExtendida&&espacioLibre<tamano){
        comp = fgetc(file);
        if(comp=='y')
            espacioLibre++;
        else{
            espacioLibre=0;
            actual = ftell(file);
        }
        if(espacioLibre>=tamano){
            return actual;
        }
        i++;
    }
    return -1;
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
            string respuesta;
            cout<<"Esta seguro que quiere eliminar esta partición? s/n"<<endl;
            cin>>respuesta;
            if(respuesta.compare("s")==0||respuesta.compare("S")==0){
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
                cout<<"No se eliminó la partición"<<endl;
            }

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
    string raidName = funcion->getRaidName();
    char comando[100];
    char comando2[100];
    strcpy(comando,funcion->getAbsPath().data());
    strcpy(comando2,funcion->getAbsPath().data());
    strcat(comando,raidName.data());
    strcat(comando2,funcion->fileName.data());
    FILE *archivo = fopen(comando,"rb+");
    FILE *fRaid = fopen(comando2,"rb+");
    int add;
    if(funcion->unit=='b'){
        add = funcion->add;
    }else if(funcion->unit=='k'){
        add = funcion->add*TAMANOBYTE;
    }else {
        add = funcion->add*TAMANOBYTE*TAMANOBYTE;
    }
    int j = -1;
    if(funcion->add>0){
        MBR mbr = leerMBR(archivo);
        int i;
        for (i = 0;i<4;i++) {

            if(funcion->nombre.compare((mbr.particiones[i].part_name))==0){

                j=i;
                break;
            }
        }
        if(j!=-1){
            int espacio = getEspacio(mbr.particiones[j],archivo,add);

            if(espacio !=-1){
                char c;
                mbr.particiones[j].part_size+=add;
                if(mbr.particiones[j].part_type=='p')
                    c = 'x';
                else
                    c = 'y';
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
                cout<<"No hay espacio suficiente después de la partición"<<endl;
            }

        }else{
            //Buscar en las lógicas CHECK
            cout<<"No hay partición con ese nombre"<<endl;
        }
    }else{
        MBR mbr=leerMBR(archivo);
        bool logica=false;
        int j;
        for (int i =0;i<4;i++) {
            if(funcion->nombre.compare((mbr.particiones[i].part_name))==0){
                j=i;
                break;
            }
        }
        if(mbr.particiones[j].part_size+add>0){
            char c='0';
            fseek(archivo,mbr.particiones[j].part_size+add,SEEK_SET);
            fseek(fRaid,mbr.particiones[j].part_size+add,SEEK_SET);

            for (int i = 0;i<add*-1;i++) {
                fwrite(&c,1,1,archivo);
                fwrite(&c,1,1,fRaid);
            }
        }else{
            cout<<"La partición no puede quedar vacía después de eliminar tamaño"<<endl;
        }
        escribirMBR(archivo,mbr);
        escribirMBR(fRaid,mbr);

    }
    fclose(archivo);
    fclose(fRaid);
}

int Administrador::getEspacio(Particion particion, FILE *archivo, int add){
    int espacio = 1;
    char a;
    fseek(archivo,particion.part_start+particion.part_size,SEEK_SET);
    for (int i=0;i<add;i++) {
        fread(&a,1,1,archivo);
        if(a!='0'){
            espacio = -1;
             break;
        }
    }

    return espacio;
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
        string respuesta;
        cout<<"Esta seguro que quiere eliminar este disco? s/n"<<endl;
        cin>>respuesta;
        if(respuesta.compare("s")==0||respuesta.compare("S")==0){
            const char* dosPuntosUve=funcion->path.data();
            if(remove(dosPuntosUve)==0){
                cout<<"Se eliminó "<<funcion->fileName<<" con exito"<<endl;
            }else{
                cout<<"Error al eliminar disco en: "<<funcion->path<<endl;
            }
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
    char hora[128];
    horaAString(hora,mbr.mbr_fecha_creacion);
    fprintf(archivo,"Digraph g{ \n node[shape = record];\n");
    fprintf(archivo, "struct1[shape = record, label = \"{");
    fprintf(archivo,"{  MBR  |    }|");
    fprintf(archivo,"{mbr_tamaño | %d }|",mbr.mbr_tamano);
    fprintf(archivo,"{mbr_fecha_creacion | %s}|",hora);
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
void Administrador::horaAString(char* output,time_t t){
    struct tm *tlocal = localtime(&t);
    char autput[128];
    strftime(autput,128,"%d/%m/%y %H:%M:%S",tlocal);
    strcpy(output,autput);
}
void Administrador::reporteDisco(Funcion* funcion,MBR mbr){
    FILE* archivo = fopen("/home/mia/Reportes/discoRep.dot","w");
    NodoParticion* part = listaDisco->existeId(funcion->id[0]);
    if(archivo!=nullptr){
        fprintf (archivo, "digraph g { \n subgraph sub { \n node [shape=rectangle style=filled color=black fillcolor=white];\n");
        fprintf (archivo, "titulo[fontsize=40, label=\"Reporte disco\", shape=plain, style=\"\"];\n");
        fprintf (archivo, "titulo->mbr;\n");
        fprintf (archivo, "mbr [label=\"MBR\"];\n");
        discoInterno(sizeof (MBR),mbr,archivo,part);
        fprintf ( archivo, "}\n}\n");
        fclose(archivo);

        /**************************************************************************************/
        generarReporte(funcion,"discoRep");
    }else{
        //no sirve el archivón
    }
}
void Administrador::repBMInode(Funcion *funcion){

    char path[100];
    strcpy(path,funcion->path.data());
    FILE* archivo = fopen(path,"w");
    char ide[5];
    strcpy(ide,funcion->id[0].c_str());
    SuperBloque sb = getSuperBloque(ide);
    NodoParticion* part = listaDisco->existeId(ide);
    FILE* archivoBitmap = fopen(part->path,"rb+");
    if(archivo!=nullptr&&archivoBitmap!=nullptr){
        escribirFileBitmap(archivo,archivoBitmap,numeroEstructuras(part->tamano,funcion->fs),sb.s_bm_inode_start);
        fclose(archivo);
        fclose(archivoBitmap);
    }
}
void Administrador::repBMBlock(Funcion *funcion){
    char path[100];
    strcpy(path,funcion->path.data());
    FILE* archivo = fopen(path,"w");
    char ide[5];
    strcpy(ide,funcion->id[0].c_str());
    SuperBloque sb = getSuperBloque(ide);
    NodoParticion* part = listaDisco->existeId(ide);
    FILE* archivoBitmap = fopen(part->path,"rb+");
    if(archivo!=nullptr&&archivoBitmap!=nullptr){
        escribirFileBitmap(archivo,archivoBitmap,numeroEstructuras(part->tamano,funcion->fs)*3,sb.s_bm_block_start);
        fclose(archivo);
        fclose(archivoBitmap);
    }
}
void Administrador::repSuperBloque(Funcion *funcion){
    char ide[5];
    char hora1[128],hora2[128];
    strcpy(ide,funcion->id[0].c_str());
    SuperBloque sb = getSuperBloque(ide);
    FILE* archivo = fopen("/home/mia/Reportes/superbloque.dot","w");
    if(archivo!=nullptr){
        horaAString(hora1,sb.s_mtime);
        horaAString(hora2,sb.s_umtime);
        fprintf(archivo,"Digraph g{ \n");
        fprintf(archivo,"node1[shape=none\nlabel = <\n"
                   "<table border=\'0\' cellborder=\'1\' color=\'blue\' cellspacing=\'0\'>");
        fprintf(archivo,"<tr><td>SUPER BLOQUE</td></tr>\n");
        fprintf(archivo,"<tr><td>Nombre</td><td>Valor</td></tr>\n");
        fprintf(archivo,"<tr><td>s_inodes_count</td><td>%d</td></tr>\n",sb.s_inodes_count);
        fprintf(archivo,"<tr><td>s_blocks_count</td><td>%d</td></tr>\n",sb.s_blocks_count);
        fprintf(archivo,"<tr><td>s_free_inodes_count</td><td>%d</td></tr>\n",sb.s_free_inodes_count);
        fprintf(archivo,"<tr><td>s_free_blocks_count</td><td>%d</td></tr>\n",sb.s_free_blocks_count);
        fprintf(archivo,"<tr><td>s_mtime</td><td>%s</td></tr>\n",hora1);
        fprintf(archivo,"<tr><td>s_utime</td><td>%s</td></tr>\n",hora2);
        fprintf(archivo,"<tr><td>s_magic</td><td>%x</td></tr>\n",sb.s_magic);
        fprintf(archivo,"<tr><td>s_inode_size</td><td>%d</td></tr>\n",sb.s_inode_size);
        fprintf(archivo,"<tr><td>s_block_size</td><td>%d</td></tr>\n",sb.s_block_size);
        fprintf(archivo,"<tr><td>s_first_ino</td><td>%d</td></tr>\n",sb.s_first_ino);
        fprintf(archivo,"<tr><td>s_first_blo</td><td>%d</td></tr>\n",sb.s_first_blo);
        fprintf(archivo,"<tr><td>s_bm_inode_start</td><td>%d</td></tr>\n",sb.s_bm_inode_start);
        fprintf(archivo,"<tr><td>s_bm_block_start</td><td>%d</td></tr>\n",sb.s_bm_block_start);
        fprintf(archivo,"<tr><td>s_inode_start</td><td>%d</td></tr>\n",sb.s_inode_start);
        fprintf(archivo,"<tr><td>s_block_start</td><td>%d</td></tr>\n",sb.s_block_start);
        fprintf(archivo, "</table>>\n]\n}\n");
        fclose(archivo);
        generarReporte(funcion,"superbloque");
    }
}
void Administrador::escribirFileBitmap(FILE *archivo, FILE *archivoBM, int nEstructuras, int inicio){
    int contador = 0;
    int contNoditos = 0;
    fseek(archivoBM,inicio,SEEK_SET);
    char b;
    int start = ftell(archivoBM);
    for (int i =0;i<nEstructuras;i++) {

        if(contador>=20){
            contador = 0;
            contNoditos++;
        }
        if(contador==0){
        }
        if(contador==19||i==nEstructuras-1)
        {
            fread(&b,1,1,archivoBM);
            fprintf(archivo,"%c",b);
            fprintf(archivo,"\n");
        }else{
            fread(&b,1,1,archivoBM);
            fprintf(archivo,"%c | ",b);
        }
        contador++;
    }
    cout<<"Numero de estructuras:"<<nEstructuras<<"Inicio en:"<< start<<" El puntero de bitmap termina en:"<<ftell(archivoBM)<<endl;

}
void Administrador::discoInterno(int bitActual, MBR mbr, FILE *archivo, NodoParticion *part){

    int bandera = 0;
    EBR ebr;
    int i;
    int tamanoMBR = mbr.mbr_tamano;
    //Escribir particion
    for (i = 0;i<4;i++) {
        if(mbr.particiones[i].part_status!='d'){
            if(mbr.particiones[i].part_start == bitActual){
                bitActual+=mbr.particiones[i].part_size;
                bandera=10;
                if(mbr.particiones[i].part_type=='e'){
                    if(hayLogicas(part->path)){
                        fprintf(archivo,"subgraph sub{\n");
                        fprintf ( archivo, "extendida [label=\"Extendida(logicas ) \n %f \"];",(double(mbr.particiones[i].part_size)/double(mbr.mbr_tamano))*100.00);
                        FILE* f = fopen(part->path,"rb+");
                        fseek(f,mbr.particiones[i].part_start,SEEK_SET);
                        fread (&ebr, sizeof(ebr), 1,f);
                        fclose(f);
                        discoEBR(i,mbr,archivo,part,ebr);
                        fprintf(archivo,"}");
                    }else{
                        fprintf ( archivo, "extendida [label=\"Extendida(vacia) \n %f \% \"];",(double(mbr.particiones[i].part_size)/double(mbr.mbr_tamano))*100.00);
                    }
                }else{
                    fprintf(archivo,"primaria%d [label=\"primaria(%s)(%f \%)\n %d  \"];\n",i,mbr.particiones[i].part_name,((double(mbr.particiones[i].part_size)/double(mbr.mbr_tamano))*100.00),mbr.particiones[i].part_size);
                }
                break;
            }
        }
    }
    //Escribir bloque libre
    int respaldo = 2147483647;
    if(bandera==0){
        fprintf(archivo,"libre[label=\"libre\"];\n");
        respaldo = bitActual;
        for (i = 0;i<4;i++) {
            if(bitActual<mbr.particiones[i].part_start){
                respaldo = mbr.particiones[i].part_start;
                fprintf(archivo,"libre%d [label=\"libre \\n %f \% \"];\n",i,(double(mbr.particiones[i].part_size)/double(mbr.mbr_tamano))*100);
                bandera=11;
            }else if(((bitActual<mbr.particiones[i].part_start)&&(respaldo>mbr.particiones[i].part_start))||(bandera==0)){
                respaldo = mbr.particiones[i].part_start;
                fprintf(archivo,"libre%d [label=\"libre \\n %f \% \"];\n",i,(double(mbr.particiones[i].part_size)/double(mbr.mbr_tamano))*100);
                bandera=11;
            }
        }
    }
    if((bandera==10)){
        discoInterno(bitActual,mbr,archivo,part);
    }else if(bitActual<respaldo){
        discoInterno(respaldo,mbr,archivo,part);
    }
}

void Administrador::discoEBR(int posicion, MBR mbr, FILE *archivo, NodoParticion *part, EBR ebr){
    float tam = mbr.mbr_tamano;
    if(ebr.part_status!='d'){
        fprintf(archivo,"logica%s [label=\"logica(%s)(%f \%)\n %d  \"];\n",ebr.part_name,ebr.part_name,((double(mbr.particiones[posicion].part_size)/double(mbr.mbr_tamano))*100.00),ebr.part_size);
        if(ebr.part_next > 0)
        {
            FILE* f = fopen(part->path,"rb+");
            fseek(f,ebr.part_next,SEEK_SET);
            fread (&ebr, sizeof(ebr), 1,f);
            fclose(f);
            discoEBR(posicion,mbr,archivo,part,ebr);
        }
    }
}

void Administrador::repTree(Funcion *funcion){
    char ide[5];
    char hora1[128];
    strcpy(ide,funcion->id[0].data());
    SuperBloque sb = getSuperBloque(ide);
    NodoParticion* part = listaDisco->existeId(ide);
    int i;
    FILE* archivo = fopen("/home/mia/Reportes/arbol.dot","w");

    int n = numeroEstructuras(part->tamano,sesion->tipo);
    Bitmap bm_inodo[n];
    leerBMInodo(sb.s_bm_inode_start,n,part->path,bm_inodo);

    FILE* archivoTree = fopen(part->path,"rb+");
    if(archivo!=NULL&&archivoTree!=NULL){
        fseek(archivoTree,sb.s_inode_start,SEEK_SET);
        fprintf(archivo,"digraph g{\nnode [shape=record];\n");
        fprintf(archivo,"node [style=filled];");
        fprintf(archivo,"edge [color=\"#A30015\"];\n");
        fprintf(archivo,"rankdir=LR;\n");



        iNodo inodo;
        fread(&inodo,sizeof (iNodo),1,archivoTree);
        horaAString(hora1,inodo.i_ctime);

        fprintf(archivo,"inodo%d[label=\"",0);
        fprintf(archivo,"inodo=%d|tamaño=%d|tipo=%c|idusr=%d",0,inodo.i_size,inodo.i_type,inodo.i_uid);
        /*Recorrer punteritos*/
        int j;
        for (j = 0;j<12;j++) {
            if(inodo.i_block[j]!=-1){
                fprintf(archivo,"|{AD%d|<N%d>}",j,j);
            }
        }
        for (j = 12;j<15;j++) {
            if(inodo.i_block[j]!=-1){
                fprintf(archivo,"|{AI%d|<N%d>}",j-11,j);
            }
        }
        fprintf(archivo, "\",color=white,fillcolor=\"#A30015\",fontcolor=white];\n");

        for(j=0;j<12;j++){
            if(inodo.i_block[j]!=-1){
                fprintf(archivo,"inodo%d:N%d->bloque%d;\n",0,j,inodo.i_block[j]);
                definirCarpetaTree(archivo,archivoTree,inodo.i_block[j],sb);
            }
        }
        for (j=12;j<15;j++) {
            if(inodo.i_block[j]!=-1){
                fprintf(archivo,"inodo%d:N%d->bloque%d;\n",0,j,inodo.i_block[j]);
                definirIndirectoTree(archivo,archivoTree,inodo.i_block[j],sb,j-11,0);
            }
        }


        fprintf(archivo, "}\n");
        fclose(archivo);
        fclose(archivoTree);
        generarReporte(funcion,"arbol");
    }
}

void Administrador::definirInodoTree(FILE *archivo, FILE *archivoTree, int apActual, SuperBloque sb){
    char* hora1;
    iNodo inodo;
    fseek(archivoTree,sb.s_inode_start+apActual*sizeof (inodo),SEEK_SET);
    fread(&inodo,sizeof (iNodo),1,archivoTree);
    horaAString(hora1,inodo.i_ctime);

    fprintf(archivo,"inodo%d[label=\"",0);
    fprintf(archivo,"inodo=%d|tamaño=%d|tipo=%c|idusr=%d",0,inodo.i_size,inodo.i_type,inodo.i_uid);
    /*Recorrer punteritos*/
    int j;
    for (j = 0;j<12;j++) {
        if(inodo.i_block[j]!=-1){
            fprintf(archivo,"|{AD%d|<N%d>}",j,j);
        }
    }
    for (j = 12;j<15;j++) {
        if(inodo.i_block[j]!=-1){
            fprintf(archivo,"|{AI%d|<N%d>}",j-11,j);
        }
    }
    fprintf(archivo, "\",color=white,fillcolor=\"#A30015\",fontcolor=white];\n");
    if(inodo.i_type==0){
        for(j=0;j<12;j++){
            if(inodo.i_block[j]!=-1){
                fprintf(archivo,"inodo%d:N%d->bloque%d;\n",0,j,inodo.i_block[j]);
                definirCarpetaTree(archivo,archivoTree,inodo.i_block[j],sb);
            }
        }
    }
    else{
        for(j=0;j<12;j++){
            if(inodo.i_block[j]!=-1){
                fprintf(archivo,"inodo%d:N%d->bloque%d;\n",0,j,inodo.i_block[j]);
                definirArchivoTree(archivo,archivoTree,inodo.i_block[j],sb);
            }
        }
    }

    for (j=12;j<15;j++) {
        if(inodo.i_block[j]!=-1){
            fprintf(archivo,"inodo%d:N%d->bloque%d;\n",0,j,inodo.i_block[j]);
            if(inodo.i_type==0)
                definirIndirectoTree(archivo,archivoTree,inodo.i_block[j],sb,j-11,0);
            else
                definirIndirectoTree(archivo,archivoTree,inodo.i_block[j],sb,j-11,1);

        }
    }
}

void Administrador::definirCarpetaTree(FILE *archivo, FILE *archivoTree, int apActual, SuperBloque sb){
    BloqueCarpeta carpeta = getBloqueCarpeta(archivoTree,sb.s_block_start,apActual);
    fprintf(archivo,"bloque%d[label=\"bloque%d",apActual,apActual);
    int i;
    for (i=0;i<4;i++) {
        if(carpeta.b_content[i].b_inodo!=-1){
            fprintf(archivo,"|{%s|<N%d>%i}",carpeta.b_content[i].b_name,i,carpeta.b_content[i].b_inodo);

        }else{
            fprintf(archivo,"|{--|<N%d>-1}",i);
        }
    }
    fprintf(archivo, "\",color=white,fillcolor=\"#7A6C5D\",fontcolor=white];\n");
    for (i = 0; i < 4; i++) {
        if (carpeta.b_content[i].b_inodo != -1) {
            fprintf(archivo, "\tbloque%i:N%i->I%i;\n", apActual, i, carpeta.b_content[i].b_inodo);
            definirInodoTree(archivo,archivoTree,carpeta.b_content[i].b_inodo,sb);
        }
    }
}

void Administrador::definirArchivoTree(FILE *archivo, FILE *archivoTree, int apActual, SuperBloque sb){
    BloqueArchivo bloque = getBloqueArchivo(archivoTree,sb.s_block_start,apActual);
    fprintf(archivo,"bloque%d[label\"Bloque%d",apActual,apActual);
    char blocazo[65];
    strcpy(blocazo,bloque.b_content);
    fprintf(archivo,"|%s",blocazo);
    fprintf(archivo, "\",color=white,fillcolor=#2A3D45,fontcolor=white];\n");
}

void Administrador::definirIndirectoTree(FILE *archivo, FILE *archivoTree, int apActual, SuperBloque sb, int tipoIndirecto, int tipoBloque){

    fprintf(archivo, "\",color=white,fillcolor=#FFCF99,fontcolor=white];\n");
}

BloqueArchivo Administrador::getBloqueArchivo(FILE *archivo, int inicio, int pos){
    BloqueArchivo b;
    b.b_content[0]='0';
    if(archivo){
        fseek(archivo,inicio+pos*sizeof (BloqueArchivo),SEEK_SET);
        fread(&b,sizeof (BloqueArchivo),1,archivo);
    }
    return b;
}

BloqueCarpeta Administrador::getBloqueCarpeta(FILE* archivo,int inicio,int pos){
    BloqueCarpeta b;
    b.b_content[0].b_inodo=-1;
    if(archivo){
        fseek(archivo,inicio+pos*sizeof (BloqueCarpeta),SEEK_SET);
        fread(&b,sizeof (BloqueCarpeta),1,archivo);
    }
    return b;
}

void Administrador::leerBMInodo(int inicio, int n, char *path, Bitmap *bmInodo){
    FILE *f;
    if((f = fopen(path,"rb+"))!=NULL){
        int j;
        fseek(f,inicio,SEEK_SET);
        for (j=0;j<n;j++) {
            fread(&bmInodo[j],sizeof (Bitmap),1,f);
        }
        fclose(f);
    }else{
        cout<<"Error al abrir ruta"<<endl;
    }
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
                reporteDisco(funcion,mbr);
            }
            else
                cout<<"No se encontró partición montada con ese id"<<endl;
        }else if(funcion->nombre.compare("journaling")==0){
            repJournaling(funcion);
        }else if(funcion->nombre.compare("bm_inode")==0){
            repBMInode(funcion);
        }else if(funcion->nombre.compare("bm_block")==0){
            repBMBlock(funcion);
        }else if(funcion->nombre.compare("file")==0){
            repFile(funcion);
        }else if(funcion->nombre.compare("inode")==0){
            repInodo(funcion);
        }else if(funcion->nombre.compare("block")==0){
            repBlock(funcion);
        }else if(funcion->nombre.compare("tree")==0){      //Primero
            repTree(funcion);
        }else if(funcion->nombre.compare("sb")==0){        //fresh
            repSuperBloque(funcion);
        }else if(funcion->nombre.compare("ls")==0){        //Hard
            repLS(funcion);
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

int Administrador::getBFByte(FILE *file, int tamano){
    MBR mbr;
    fseek(file,0,SEEK_SET);
    fread(&mbr,sizeof (MBR),1,file);
    int retorno = -1;
    int menorEspacio=mbr.mbr_tamano;
    int actual = ftell(file);
    int espacioLibre=0;
    char comp;
    while(!feof(file)){
        comp = fgetc(file);
        if(comp=='0')
            espacioLibre++;
        else{
            espacioLibre=0;
            actual = ftell(file);
        }
        if(espacioLibre>=tamano){
            if(menorEspacio>espacioLibre){
                menorEspacio = espacioLibre;
                retorno = actual;
            }
        }
    }
    return retorno;
}

int Administrador::getWFByte(FILE *file, int tamano){
    MBR mbr;
    fseek(file,0,SEEK_SET);
    fread(&mbr,sizeof (MBR),1,file);
    int retorno = -1;
    int mayorEspacio=-1;
    int actual = ftell(file);
    int espacioLibre=0;
    char comp;
    while(!feof(file)){
        comp = fgetc(file);
        if(comp=='0')
            espacioLibre++;
        else{
            espacioLibre=0;
            actual = ftell(file);
        }
        if(espacioLibre>=tamano){
            if(mayorEspacio<espacioLibre){
                mayorEspacio = espacioLibre;
                retorno = actual;
            }
        }
    }
    return retorno;
}

void Administrador::escribirMBR(FILE *file, MBR mbr){
    rewind(file);
    fwrite(&mbr,sizeof (MBR),1,file);
}

MBR Administrador::leerMBR(FILE *file){
   MBR mbr;
   fseek(file,0,SEEK_SET);
   fread(&mbr,sizeof (MBR),1,file);
   return mbr;
}

EBR Administrador::getLogica(FILE *file, char *nombre){
    EBR ebr;
    ebr.part_size=-1;
    ebr.part_status = 'd';
    MBR mbr;
    fseek(file,0,SEEK_SET);
    fread(&mbr,sizeof (MBR),1,file);
    int indiceEBR=-1;
    indiceEBR= existeEBR(mbr);
    if(indiceEBR!=-1){
        int indice = mbr.particiones[indiceEBR].part_start;
        fseek(file,indice,SEEK_SET);
        fread(&ebr,sizeof (EBR),1,file);
        while(ebr.part_next!=-1){
            if(strcmp(nombre,ebr.part_name)==0){
                return ebr;
            }
            fseek(file,ebr.part_next,SEEK_SET);
            fread(&ebr,sizeof (EBR),1,file);
        }
    }
    ebr.part_size=-1;
    ebr.part_status = 'd';
    return ebr;
}

Particion Administrador::getParticion(char *path, char *nombre){
    FILE * archivo = fopen(path,"rb+");
    Particion p;
    if(archivo!=nullptr){
        MBR mbr;
        fseek(archivo,0,SEEK_SET);
        fread(&mbr,sizeof (MBR),1,archivo);
        int i;
        for (i = 0;i<4;i++) {
            if(strcmp(nombre,mbr.particiones[i].part_name)==0){
                fclose(archivo);
                return mbr.particiones[i];
            }
        }
        EBR ebr = getLogica(archivo,nombre);
        if(ebr.part_size>0){
            p.part_fit=ebr.part_fit;
            p.part_size = ebr.part_size;
            strcpy(p.part_name,ebr.part_name);
            p.part_start = ebr.part_start;
            p.part_type='l';
            p.part_status = ebr.part_status;
            return p;
        }
        fclose(archivo);
        p.part_start=-1;
        p.part_status='d';
        return p;
    }
}

int Administrador::existeEBR(MBR mbr){
    for (int i=0;i<4;i++) {
        if(mbr.particiones[i].part_type=='e'){
            return i;
        }
    }
    return -1;
}

bool Administrador::hayLogicas(char *path){
    EBR ebr;
    MBR mbr;
    FILE* archivo;
    archivo=fopen(path,"rb+");
    fseek(archivo,0,SEEK_SET);
    fread(&mbr,sizeof (MBR),1,archivo);
    int indiceEBR=-1;
    indiceEBR= existeEBR(mbr);
    if(indiceEBR!=-1){
        int indice = mbr.particiones[indiceEBR].part_start;
        fseek(archivo,indice,SEEK_SET);
        fread(&ebr,sizeof (EBR),1,archivo);
        if(ebr.part_size>0){
            return true;
        }
    }
    return false;
}

MBR Administrador::getListaMBR2(Funcion *funcion){
    MBR mbr;
    char id[4];
    strcpy(id,funcion->id[0].data());
    NodoParticion *part = listaDisco->existeId(id);
    if(part!=nullptr){
        FILE* archivo;
        archivo = fopen(part->path,"rb+");
        if(archivo!=nullptr){
            fseek(archivo,0,SEEK_SET);
            fread(&mbr,sizeof (MBR),1,archivo);
            return mbr;
        }
        fclose(archivo);
    }else{
        cout<<"Error, path de particion montada no abrió";
    }

    mbr.mbr_disk_signature=-1;
    return mbr;
}

void Administrador::crearDirectorioLinux(string path, Funcion *funcion){
    string absPath;
    char comando1[200];
    string tokenActual;
    strcpy(comando1,"mkdir '");
    absPath=funcion->getAbsPath();
    strcat(comando1,absPath.data());
    strcat(comando1,"' -p");
    cout<<"Comando a ejecutar: "<<comando1<<endl;
    system(comando1);
}

