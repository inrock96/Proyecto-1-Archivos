#include "administrador.h"
#define TAMANOBYTE 1024
Administrador::Administrador()
{
    noDiscos = 0;
    this->listaDisco = new ListaDisco();
    this->sesion = new Sesion();

}

int Administrador::numeroEstructuras(int tamanoPart){
    double n;
    n = (tamanoPart-(sizeof (SuperBloque)))/
            (27+sizeof (Journal)+sizeof (iNodo)
            +(20*64));

    return floor(n);
}

SuperBloque Administrador::crearSuperBloque(int bit_inicio, int tamano, NodoParticion *part){
    SuperBoot sb;
    int nEstructuras = numeroEstructuras(tamano);
    //Llenar superboot
    strcpy(sb.sb_nombre_hd,part->nombrePart);
    sb.sb_arbol_virtual_count=1;
    sb.sb_detalle_directorio_count = 1;
    sb.sb_inodos_count = 1;
    sb.sb_bloques_count = 2;
    sb.sb_arbol_virtual_free = nEstructuras - 1;
    sb.sb_detalle_directorio_free = nEstructuras -1;
    sb.sb_inodos_free = (5*nEstructuras)-1;
    sb.sb_bloques_free = (20*nEstructuras)-3;
    sb.sb_date_creacion = time(0);
    sb.sb_magic_num=201504002;
    sb.sb_size_struct_bloque=sizeof (Bloque);
    sb.sb_size_struct_tabla_inodo=sizeof (i_nodo);
    sb.sb_size_struct_arbol_directorio=sizeof (Arbol_Virtual_Directorio);
    sb.sb_size_struct_detalle_directorio=sizeof (Detalle_Directorio);
    sb.sb_montajes_count = 0;
    sb.sb_part_start = bit_inicio;
    sb.sb_ap_bitmap_arbol_directorio = bit_inicio+sizeof (SuperBoot);
    sb.sb_ap_arbol_directorio = sb.sb_ap_bitmap_arbol_directorio+nEstructuras;
    sb.sb_ap_bitmpa_detalle_directorio = sb.sb_ap_arbol_directorio + (nEstructuras*sizeof (Arbol_Virtual_Directorio));
    sb.sb_ap_detalle_directorio = sb.sb_ap_bitmpa_detalle_directorio + nEstructuras;
    sb.sb_ap_bitmap_tabla_inodo = sb.sb_ap_detalle_directorio + (nEstructuras*sizeof (Detalle_Directorio));
    sb.sb_ap_tabla_inodo = sb.sb_ap_bitmap_tabla_inodo + nEstructuras*5;
    sb.sb_ap_bitmap_bloques = sb.sb_ap_tabla_inodo + (nEstructuras*5*sizeof (i_nodo));
    sb.sb_ap_bloques = sb.sb_ap_bitmap_bloques +(nEstructuras*20);
    sb.sb_ap_log = sb.sb_ap_bloques + (nEstructuras*20*sizeof (Bloque));
    sb.sb_first_free_bit_bloques=2;
    sb.sb_first_free_bit_tabla_inodo=1;
    sb.sb_first_free_bit_arbol_directorio=1;
    sb.sb_first_free_bit_detalle_directorio=1;

    return  sb;
}

void Administrador::formatear(Funcion *funcion){
    if(true/*fs=3*/){
        if(funcion->opciones[8]==1){
            int format = 1;
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
                    int nEstructuras =numeroEstructuras(part->tamano);
                    SuperBloque sb = crearSuperBloque(part->byteInicio,part->tamano,part);
                    //Escribir superboot
                    fseek(archivo,part->byteInicio,SEEK_SET);
                    fwrite(&sb,sizeof (SuperBloque),1,archivo);
                    //Escribir copia superboot
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
                    Journal journal;
                    journal.log_tipo=-1;
                    journal.log_fecha = time(0);
                    journal.contenido = 1;
                    journal.log_tipo_operacion=2;
                    limpiarVar(journal.log_path,100);
                    limpiarVar(journal.log_nombre,24);
                    limpiarVar(journal.log_propietario,16);
                    int prueba = sb.s_block_start;
                    for (i=0;i<nEstructuras;i++) {
                        fseek(archivo,prueba,SEEK_SET);
                        fwrite(&journal,sizeof (Journal),1,archivo);
                        prueba+=sizeof (Journal);
                    }
                    fclose(archivo);
                    archivo = fopen(part->path,"rb+");
                    prueba = sb.s_journal_start;
                    for (i=0;i<nEstructuras;i++) {
                        Journal jour;
                        fseek(archivo,prueba,SEEK_SET);
                        fread(&jour,sizeof (Journal),1,archivo);
                        prueba+=sizeof (Journal);
                    }
                    fclose(archivo);
                    archivo = fopen(part->path,"rb+");
                    if(format!=1){
                        //Fast
                    }else{
                        //Full
                        //Escribir bloques
                        BloqueArchivo b;
                        int i;
                        strcpy(b.db_data,"vacio");
                        fseek(archivo,super.sb_ap_bloques,SEEK_SET);
                        int pruebB = super.sb_ap_bloques;
                        for (i = 0;i<nEstructuras*20;i++) {
                            fseek(archivo,pruebB,SEEK_SET);
                            fwrite(&b,sizeof (Bloque),1,archivo);
                            pruebB+=sizeof (Bloque);
                        }
                        //Escribir inodos
                        i_nodo inodo;
                        strcpy(inodo.i_id_proper,"root");
                        inodo.i_count_inodo=0;
                        inodo.i_ap_indirecto=-1;
                        inodo.i_size_archivo=0;
                        inodo.i_array_bloques[0]=0;
                        inodo.i_array_bloques[1]=0;
                        inodo.i_array_bloques[2]=0;
                        inodo.i_array_bloques[3]=0;
                        inodo.i_count_bloques_asignados=0;
                        int pruebainodo = super.sb_ap_tabla_inodo;
                        fseek(archivo,super.sb_ap_tabla_inodo,SEEK_SET);
                        for (i=0;i<nEstructuras*5;i++) {
                            fseek(archivo,pruebainodo,SEEK_SET);
                            fwrite(&inodo,sizeof (i_nodo),1,archivo);
                            pruebainodo+= sizeof (i_nodo);
                        }
                        //
                    }
                    escribirBitMap(sb.sb_ap_bitmap_arbol_directorio,nEstructuras,archivo);
                    //Escribir bitmap DD
                    escribirBitMap(sb.sb_ap_bitmpa_detalle_directorio,nEstructuras,archivo);
                    //Escribir bitmap Inodo
                    escribirBitMap(sb.sb_ap_bitmap_tabla_inodo,nEstructuras*5,archivo);
                    //Escribir bitmap Bloque
                    escribirBitMap(sb.sb_ap_bitmap_bloques,nEstructuras*20,archivo);


                    //Se crea la raíz del árbol de directorio
                    Arbol_Virtual_Directorio raiz;
                    raiz.avd_fecha_creacion = time(0);
                    strcpy(raiz.avd_nombre_directorio,"raiz");
                    for (i=0;i<6;i++) {
                        raiz.avd_ap_array_subdirectorios[i]=-1;
                    }
                    raiz.avd_ap_detalle_directorio=0;
                    raiz.avd_ap_arbol_virtual_directorio=-1;
                    strcpy(raiz.avd_proper,"root");
                    raiz.lectura = 0;
                    raiz.escritura = 0;
                    raiz.exe = 0;
                    //Se crea el detalle de directorio del avd donde se guardan los usuarios
                    Detalle_Directorio d_raiz;
                    d_raiz.dd_ap_detalle_directorio=-1;
                    for (int j=0;j<5;j++) {
                        d_raiz.dd_array_files[j].dd_file_ap_inodo=-1;
                    }
                    strcpy(d_raiz.dd_array_files[0].dd_file_nombre,"users.txt");
                    d_raiz.dd_array_files[0].dd_file_date_creation=time(0);
                    d_raiz.dd_array_files[0].dd_file_date_modification=time(0);
                    d_raiz.dd_array_files[0].dd_file_ap_inodo=0;

                    //se crea el inodo para el archivo
                    char users[100];
                    strcpy(users,"1,G,root\n1,U,root,root,201504002\n");
                    int cont = 0;
                    while (users[cont]!=NULL) {
                        cont++;
                    }
                    int nbloques = getNumeroBloques(cont);
                    int nultimo = getUltimoNBloques(cont);
                    i_nodo iraiz;
                    strcpy(iraiz.i_id_proper,"root");
                    iraiz.i_count_inodo=0;
                    iraiz.i_size_archivo = cont;
                    iraiz.i_count_bloques_asignados=nbloques;
                    iraiz.i_ap_indirecto=-1;
                    for (i=0;i<4;i++) {
                        iraiz.i_array_bloques[i]=-1;
                    }
                    iraiz.exe=0;
                    iraiz.lectura=0;
                    iraiz.excritura=0;
                    //Se llenan los bloques de información
                    int ind = 0;
                    int posObtenidaDeBitmap=0;
                    for (i = 0;i<nbloques;i++) {
                        int inicioBloques = super.sb_ap_bloques;
                        Bloque bloque;
                        for (int i=0;i<posObtenidaDeBitmap;i++) {
                            inicioBloques= inicioBloques+sizeof (Bloque);
                        }
                        fseek(archivo,inicioBloques,SEEK_SET);
                        fread(&bloque,sizeof (Bloque),1,archivo);

                        if(i==nbloques-1){
                            for(int i = 0;i<nultimo;i++){
                                bloque.db_data[i]=users[ind];
                                ind++;
                            }
                            bloque.db_data[nultimo]='\0';
                        }else{
                            for (int i = 0;i<24;i++) {
                                bloque.db_data[i]=users[ind];
                                ind++;
                            }
                            bloque.db_data[24]='\0';
                        }
                        //Escribir bloque en el disco
                        inicioBloques = super.sb_ap_bloques;
                        for(i = 0; i<posObtenidaDeBitmap;i++){
                            inicioBloques = inicioBloques+sizeof (Bloque);
                        }
                        for (i=0;i<4;i++) {
                            if(iraiz.i_array_bloques[i]==-1){
                                iraiz.i_array_bloques[i]=posObtenidaDeBitmap;
                                i=4;
                            }
                        }

                        fseek(archivo,inicioBloques,SEEK_SET);
                        fwrite(&bloque,sizeof (Bloque),1,archivo);
                        cout<<ftell(archivo)<<endl;
                        int inicioBitmap = super.sb_ap_bitmap_bloques;
                        char b1='1';
                        for (i=0;i<posObtenidaDeBitmap;i++) {
                            inicioBitmap = inicioBitmap+sizeof(char);
                        }
                        fseek(archivo,inicioBitmap,SEEK_SET);
                        fwrite(&b1,sizeof (char),1,archivo);
                        posObtenidaDeBitmap++;
                    }
                    //Bitacora

                    //Log raiz
                    Bitacora braiz;
                    Bitacora busers;
                    braiz.log_tipo=0;

                    braiz.log_fecha = time(0);
                    strcpy(braiz.log_nombre,"raiz");
                    strcpy(braiz.log_propietario,"root");
                    braiz.contenido = 1;
                    braiz.log_tipo_operacion=0;
                    strcpy(braiz.log_path,"/");

                    //Log usuarios
                    busers.log_path[0]='/';
                    busers.log_fecha = time(0);
                    busers.log_tipo = 1;
                    busers.log_tipo_operacion=0;
                    strcpy(busers.log_nombre,"users.txt");
                    strcpy(busers.log_path,"/");
                    strcpy(busers.log_propietario,"root");
                    busers.contenido=2;
                    //escribir todo
                    //bitmap avd
                    char b1 ='1';
                    fseek(archivo,super.sb_ap_bitmap_arbol_directorio,SEEK_SET);
                    fwrite(&b1,1,1,archivo);
                    //escribir bitmap detalle
                    fseek(archivo,super.sb_ap_bitmpa_detalle_directorio,SEEK_SET);
                    fwrite(&b1,1,1,archivo);
                    //escribir bitmap inodo
                    fseek(archivo,super.sb_ap_bitmap_tabla_inodo,SEEK_SET);
                    fwrite(&b1,1,1,archivo);
                    //escribir raiz avd
                    fseek(archivo,super.sb_ap_arbol_directorio,SEEK_SET);
                    fwrite(&raiz,sizeof(Arbol_Virtual_Directorio),1,archivo);
                    //escribir detalle
                    fseek(archivo,super.sb_ap_detalle_directorio,SEEK_SET);
                    fwrite(&d_raiz,sizeof (Detalle_Directorio),1,archivo);
                    //escribir inodo
                    fseek(archivo,super.sb_ap_tabla_inodo,SEEK_SET);
                    fwrite(&iraiz,sizeof (i_nodo),1,archivo);

                    int iniciobb = super.sb_ap_bloques;
                    fseek(archivo,iniciobb,SEEK_SET);
    //                for (int i =0;i<(4*5*nEstructuras);i++) {
    //                    Bloque b;
    //                    iniciobb=+iniciobb+sizeof (Bloque);
    //                    cout<<"contenido bitmap: "<<b.db_data;
    //                }
                    fclose(archivo);
                    escribirJournal(braiz,part->path,super,nEstructuras);
                    escribirJournal(busers,part->path,super,nEstructuras);
                    //fin
                }
            }
        }
    }else{
        //ext2
    }
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

