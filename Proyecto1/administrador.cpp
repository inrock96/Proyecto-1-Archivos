#include "administrador.h"
#define TAMANOBYTE 1024
Administrador::Administrador()
{
    noDiscos = 0;
    this->listaDisco = new ListaDisco();
    this->sesion = new Sesion();

}

void Administrador::crearUsr(Funcion *funcion){
    if(funcion->opciones[9]==1&&funcion->opciones[10]==1&&funcion->opciones[11]==1){
        if(sesion->usrid!=-1){
            NodoParticion *part = listaDisco->existeId(sesion->idPart);
            if(part!=nullptr){
                if(sesion->groupid==1){
                    SuperBloque sb = getSuperBloque(sesion->idPart);
                    string contenido = getContenidoArchivo(part->path,1,sb);
                    vector <string> grupos = split(contenido,10);
                    if(!existeGrupo(grupos,funcion->grp)){
                        int idGrp = setIDGrupo(grupos);
                        string grp;
                        char grupoNuevo[100];
                        grp = "\n"+to_string(idGrp)+",G,"+funcion->grp;
                        contenido+=grp;
                        insertarUsers(contenido,sb,part->path,numeroEstructuras(part->tamano,sesion->tipo));
                    }else{
                        cerr<<"ERROR, AL CREAR GRUPO, YA EXISTE ESE GRUPO"<<endl;
                    }
                }else{
                    cerr<<"ERROR, AL CREAR GRUPO, NO TIENE PERMISOS"<<endl;
                }

            } else {
                cerr<<"ERROR, AL CREAR GRUPO, NO HAY PARTICION MONTADA CON ESE ID"<<endl;
            }
        }else{
            cerr<<"ERROR, ERROR AL CREAR GRUPO, NO HA INICIADO SESION"<<endl;
        }

    }else{
        cerr<<"ERROR, AL CREAR GRUPO, FALTAN PARAMETROS"<<endl;
    }
}

void Administrador::crearGrupo(Funcion *funcion){
    if(funcion->opciones[6]==1){
        if(sesion->usrid!=-1){
            NodoParticion *part = listaDisco->existeId(sesion->idPart);
            if(part!=nullptr){
                if(sesion->groupid==1){
                    SuperBloque sb = getSuperBloque(sesion->idPart);
                    string contenido = getContenidoArchivo(part->path,1,sb);
                    vector <string> grupos = split(contenido,10);
                    if(!existeGrupo(grupos,funcion->nombre)){
                        int idGrp = setIDGrupo(grupos);
                        string grp;
                        grp = "\n"+to_string(idGrp)+",G,"+funcion->grp;
                        contenido+=grp;
                        insertarUsers(contenido,sb,part->path,numeroEstructuras(part->tamano,sesion->tipo));
                    }else{
                        cerr<<"ERROR, AL CREAR GRUPO, YA EXISTE ESE GRUPO"<<endl;
                    }
                }else{
                    cerr<<"ERROR, AL CREAR GRUPO, NO TIENE PERMISOS"<<endl;
                }

            } else {
                cerr<<"ERROR, AL CREAR GRUPO, NO HAY PARTICION MONTADA CON ESE ID"<<endl;
            }
        }else{
            cerr<<"ERROR, ERROR AL CREAR GRUPO, NO HA INICIADO SESION"<<endl;
        }

    }else{
        cerr<<"ERROR, AL CREAR GRUPO, FALTAN PARAMETROS"<<endl;
    }
}

bool Administrador::existeGrupo(vector<string> grupos, string grupo){
    return false;//CHECK
}

bool Administrador::existeUsuario(vector<string> usuarios, string usuario){
    return false;//CHECK
}

int Administrador::setIDGrupo(vector<string> grupos){
    return 2;//CHECK
}
int Administrador::setIDUsuario(vector<string> usuarios){
    return 2;//CHECK
}

void Administrador::insertarUsers(string contenido, SuperBloque sb, char *path,int nEstructuras){

    iNodo inodo_users = getInodo(path,sb.s_inode_start,1);
    int tamano = contenido.size();
    int numero_bloques = getNumeroBloques(tamano);
    vector<BloqueArchivo> bloques;
    int fin_contenido=10;
    if(contenido.size()>10){
        fin_contenido=tamano;
    }
    int i;
    int actual= tamano;
    int contador_contenido=0;
    for (i=0;i<numero_bloques;i++) {
        BloqueArchivo nuevo;
        for (int j=0;j<64;j++) {
            if(actual>0){
                if(fin_contenido<=contador_contenido)
                    contador_contenido=0;
                nuevo.b_content[j]=contenido[contador_contenido];
                contador_contenido++;
                actual--;
            }else{
                if(j<64 )
                    nuevo.b_content[j]='\0';
                break;
            }
        }
        bloques.push_back(nuevo);
    }
    inodo_users.i_size = tamano;
    for (i=0;i<12;i++) {
        if(inodo_users.i_block[i]!=-1){
            escribirBloqueArchivo(bloques[i],path,sb,inodo_users.i_block[i]);
        }else{
            inodo_users.i_block[i]=sb.s_first_blo;
            escribirBloqueArchivo(bloques[i],path,sb,sb.s_first_blo);
            escribirPosBitmap(sb.s_bm_block_start,sb.s_first_blo,path,'1');
            sb.s_first_blo = getFirstFreeBit(sb.s_bm_block_start,nEstructuras*3,path);

        }
    }
    escribirInodo(inodo_users,path,sb,1);
    escribirSuperBloque(path,sb,listaDisco->existeId(sesion->idPart)->byteInicio);

}

std::vector<std::string>Administrador:: split(const std::string& s, char delimiter)
{
   std::vector<std::string> tokens;
   std::string token;
   std::istringstream tokenStream(s);
   while (std::getline(tokenStream, token, delimiter))
   {
      tokens.push_back(token);
   }
   return tokens;
}

string Administrador::getContenidoArchivo(char *path, int apArchivo, SuperBloque sb){
    string retorno = "";
    int i;
    iNodo inodo_archivo = getInodo(path,sb.s_inode_start,apArchivo);
    int tamano = inodo_archivo.i_size;
    for (i=0;i<12;i++) {
        if(inodo_archivo.i_block[i]!=-1){
            BloqueArchivo b_archivo = getBloqueArchivo(path,sb.s_block_start,inodo_archivo.i_block[i]);
            for (int j=0;j<64;j++) {
                if(tamano>0){
                    retorno+=b_archivo.b_content[j];
                    tamano--;
                }else{
                    break;
                }
            }
        }
    }
    for (i=12;i<15;i++) {
        if(inodo_archivo.i_block[i]!=-1){
            retorno += getContenidoArchivoIndirecto(path,inodo_archivo.i_block[i],sb,tamano,i-11);
        }
    }
    return  retorno;

}

string Administrador::getContenidoArchivoIndirecto(char *path, int apBloque, SuperBloque sb, int tamano, int tipoIndirecto){
    string retorno="";
    int i;
    BloqueApuntador bloque_apuntador = getBloqueApuntador(path,sb.s_block_start,apBloque);
    if(tipoIndirecto==1){
        for (i=0;i<16;i++) {
            if(bloque_apuntador.b_pointers[i]!=-1){
                BloqueArchivo b_archivo = getBloqueArchivo(path,sb.s_block_start,bloque_apuntador.b_pointers[i]);
                for (int j=0;j<64;j++) {
                    if(tamano>0){
                        retorno+=b_archivo.b_content[j];
                        tamano--;
                    }else{
                        break;
                    }
                }
            }
        }
    }else{
        for (i=0;i<16;i++) {
            if(bloque_apuntador.b_pointers[i]=!-1){

                retorno+=getContenidoArchivoIndirecto(path,bloque_apuntador.b_pointers[i],sb,tamano,tipoIndirecto-1);

            }
        }
    }
    return retorno;
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
    sb.s_inodes_count=nEstructuras;
    sb.s_blocks_count=3*nEstructuras;
    sb.s_free_inodes_count=nEstructuras-2;
    sb.s_free_blocks_count=3*nEstructuras-2;
    sb.s_mtime=time(0);
    sb.s_umtime=time(0);
    sb.s_mnt_count=1;
    sb.s_inode_size=sizeof (iNodo);
    sb.s_block_size=sizeof (BloqueArchivo);
    if(tipo==3){

        sb.s_bm_inode_start=bit_inicio+sizeof (SuperBloque)+nEstructuras*sizeof (Journal);

    }else{
        sb.s_bm_inode_start=bit_inicio+sizeof (SuperBloque);
    }

    sb.s_bm_block_start=sb.s_bm_inode_start+nEstructuras;
    sb.s_inode_start=sb.s_bm_block_start+3*nEstructuras;
    sb.s_block_start=sb.s_inode_start+nEstructuras*sizeof (iNodo);

    sb.s_first_blo=2;
    sb.s_first_ino=2;
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
                    int nEstructuras =numeroEstructuras(part->tamano,fs);
                    SuperBloque sb = crearSuperBloque(part->byteInicio,part->tamano,part,fs);
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
                    if(posPart!=-1)
                        mbr.particiones[posPart].part_status='f';
                    int i;
                    //Escribe journal
                    int prueba ;
                    if(fs==3){
                        prueba = part->byteInicio+sizeof (SuperBloque);
                        Journal journal;
                        journal.log_tipo=-1;
                        journal.log_fecha = time(0);
                        journal.contenido = 1;
                        journal.log_propietario=-1;
                        journal.log_tipo_operacion=2;
                        limpiarVar(journal.log_path,100);
                        limpiarVar(journal.log_nombre,24);
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
                        inodo.i_gid = -1;
                        inodo.i_uid=-1;
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
                    i_usr.i_size=32;
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

                    strcpy(b_usr.b_content,"1,G,root\n1,U,root,root,123");
                    int cont = 0;
                    while (b_usr.b_content[cont]!=NULL) {
                        cont++;
                    }
                    int nbloques = getNumeroBloques(cont);
                    int nultimo = getUltimoNBloques(cont);
                    //Se llenan los bloques de información
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
                        fseek(archivo,part->byteInicio+sizeof (SuperBloque),SEEK_SET);
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
                    fseek(archivo,sb.s_block_start,SEEK_SET);
                    fwrite(&bcarpeta,sizeof (BloqueCarpeta),1,archivo);
                    fwrite(&b_usr,sizeof (BloqueArchivo),1,archivo);
                    fclose(archivo);
                    if(fs==3){
                        escribirJournal(j_raiz,part->path,sb,nEstructuras,part->byteInicio+sizeof (SuperBloque));
                        escribirJournal(j_usuarios,part->path,sb,nEstructuras,part->byteInicio+sizeof (SuperBloque));
                    }
                    //fin
                }else{
                    cout<<"ERROR(MKFS), No se pudo abrir el archivo: "<<comando<<endl;
                }
            }else{
                cout<<"ERROR (MKFS), no hay particion montada con este id:"<<funcion->id[0]<<endl;
            }
        }
    }
}



void Administrador::login(Funcion *funcion){
    if(funcion->opciones[8]==1&&funcion->opciones[9]==1&&funcion->opciones[10]==1){
        NodoParticion* part = listaDisco->existeId(funcion->id[0]);
        Usuario*usr = getUsuario(funcion->usr,part);
        if(sesion->usrid==-1){
            usr = getUsuario(funcion->usr,part);
            if(part!=nullptr){
                if(usr->contrasena.compare(funcion->pwd)==0){
                    sesion->iniciarSesion(usr->nombre,usr->contrasena,funcion->id[0]);
                    sesion->usrid = usr->usuario;
                    sesion->groupid = usr->grupo;
                    cout<<"         Sesion iniciada con exito"<<endl;
                }else{
                    cout<<"ERROR AL INICIAR SESION, CONTRA O USR INCORRECTO"<<endl;
                }
            }else{
                cout<<"ERROR AL INICIAR SESION, PARTICION NO MONTADA"<<endl;
            }
        }else{
            cout<<"ERROR AL INICIAR SESION, YA INICIO SESION"<<endl;
        }
    }else{
        cout<<"ERROR AL INICIAR SESION, FALTAN PARAMETROS"<<endl;
    }
}

void Administrador::limpiarVar(char *var, int n){
    for (int i = 0;i<n;i++) {
        var[i]='\0';
    }
}
void Administrador::escribirBitMap(int inicio,int n,FILE* archivo){
    char b = '0';
    fseek(archivo,inicio,SEEK_SET);
    //int c = ftell(archivo);
    //int a = ftell(archivo);
    for (int i = 0;i<n;i++) {
        fseek(archivo,inicio,SEEK_SET);
        fwrite(&b,sizeof (char),1,archivo);
        inicio+=sizeof (char);
    }
    //cout<<"Numero de estructuras:"<<n<<"Inicio en:"<< a<<" El puntero de bitmap termina en:"<<ftell(archivo)<<endl;
    //cout<<c<<endl;
    int bloques = 64;
}

void Administrador::logout(){
    if(sesion->groupid<1)
            cout<<"No hay sesión iniciada"<<endl<<endl;
        else
        {
            sesion->cerrarSesion();
            cout <<"Sesión cerrada con éxito"<<endl<<endl;
        }
}

Usuario *Administrador::getUsuario(string usr, NodoParticion* part){
    Usuario* usuario = new Usuario();
    if(usr.compare("root")==0){
        usuario->nombre = "root";
        usuario->contrasena ="123";
        usuario->grupo=1;
        usuario->usuario=1;
        return usuario;
    }
    /*Buscar usuario en particion*/
    return  usuario;
}

iNodo Administrador::getInodoUsuarios(Sesion sesion){
    iNodo inodo;
    //check
    return  inodo;
}

void Administrador::escribirJournal(Journal journal, char *path, SuperBloque sb, int nEstructuras, int inicio){
    FILE *archivo = fopen(path,"rb+");
    if(archivo){
        int bitacoralibre = getPosJournal(inicio,nEstructuras,archivo);
        fseek(archivo,bitacoralibre,SEEK_SET);
        fwrite(&journal,sizeof (Journal),1,archivo);
        fclose(archivo);
    }
}
int Administrador::getPosJournal(int inicio, int nEstruct, FILE *archivo){
    int posActual =-1;
    fseek(archivo,inicio,SEEK_SET);
    Journal bit;
    for (int i =0;i<nEstruct;i++) {
        fread(&bit,sizeof(Journal),1,archivo);
        if(bit.log_tipo==-1){
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
                cout<<"ERROR FDISK, ADD, DELETE, CREAR SO NEXCLUYENTES"<<endl;
            }

            fclose(archivo);
            fclose(fRaid);
        }else{
            cout<<"ERROR, FDISK NO SE ENCONTRO UNO DE LOS DISCOS EN: "<<funcion->path<<endl;
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
            }else{
                cout<<"ERROR, NO EXISTE ESA PARTICION"<<endl;
            }
            fclose(file);
            cout<<"Fin, mount"<<endl;
        }else{
            cout<<"ERROR, NO SE PUDO ABRIR EL ARCHIVO"<<endl;
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
    crearDirectorioLinux(funcion->path,funcion);
    char path[100];
    strcpy(path,funcion->path.data());
    FILE* archivo = fopen(path,"w");
    char ide[5];
    strcpy(ide,funcion->id[0].c_str());
    SuperBloque sb = getSuperBloque(ide);
    NodoParticion* part = listaDisco->existeId(ide);
    FILE* archivoBitmap = fopen(part->path,"rb+");
    if(archivo!=nullptr&&archivoBitmap!=nullptr){
        escribirFileBitmap(archivo,archivoBitmap,numeroEstructuras(part->tamano,sb.s_filesystem_type),sb.s_bm_inode_start);
        fclose(archivo);
        fclose(archivoBitmap);
    }
}
void Administrador::repBMBlock(Funcion *funcion){
    crearDirectorioLinux(funcion->path,funcion);
    char path[100];
    strcpy(path,funcion->path.data());
    FILE* archivo = fopen(path,"w");
    char ide[5];
    strcpy(ide,funcion->id[0].c_str());
    SuperBloque sb = getSuperBloque(ide);
    NodoParticion* part = listaDisco->existeId(ide);
    FILE* archivoBitmap = fopen(part->path,"rb+");
    if(archivo!=nullptr&&archivoBitmap!=nullptr){
        escribirFileBitmap(archivo,archivoBitmap,numeroEstructuras(part->tamano,sb.s_filesystem_type)*3,sb.s_bm_block_start);
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
        fprintf(archivo,"<tr><td>s_filesystem_type</td><td>%d</td></tr>\n",sb.s_filesystem_type);
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

SuperBloque Administrador::getSuperBloque(char ide[]){
    SuperBloque sb;
    NodoParticion *part;
    if(sesion->usrid!=-1)
        part= listaDisco->existeId(sesion->idPart);
    else
        part = listaDisco->existeId(ide);
    FILE *archivo = fopen(part->path,"rb+");
    if(archivo!=nullptr){
        fseek(archivo,part->byteInicio,SEEK_SET);
        fread(&sb, sizeof (SuperBloque),1,archivo);
        cout<<"CONSIGUIO EXITOSAMENTE EL SUPERBLOQUE"<<endl;
        fclose(archivo);
    }else {
        cout<<"ERROR ABRIR ARCHIVO GET SUPERBLOQUE"<<endl;
    }
    return sb;
}

void Administrador::repTree(Funcion *funcion){
    char ide[5];
    char hora1[128];
    strcpy(ide,funcion->id[0].data());
    SuperBloque sb = getSuperBloque(ide);
    NodoParticion* part = listaDisco->existeId(ide);
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
        fclose(archivoTree);
        definirInodoTree(archivo,part->path,0,sb);

        fprintf(archivo, "}\n");
        fclose(archivo);
        generarReporte(funcion,"arbol");
    }
}

void Administrador::definirInodoTree(FILE *archivo, char path[], int apActual, SuperBloque sb){
    char hora1[128];
    char hora2[128];
    char hora3[128];
    iNodo inodo;
    inodo = getInodo(path,sb.s_inode_start,apActual);
    horaAString(hora1,inodo.i_atime);
    horaAString(hora2,inodo.i_ctime);
    horaAString(hora3,inodo.i_mtime);

    fprintf(archivo,"inodo%d[label=\"",apActual);
    fprintf(archivo,"inodo%d|tamaño=%d|tipo=%d|idusr=%d|gid=%d|atime=%s|ctime=%s|mtime=%s|ugo=%d%d%d",
            apActual,
            inodo.i_size,
            inodo.i_type,
            inodo.i_uid,
            inodo.i_gid,
            hora1,
            hora2,
            hora3,
            inodo.i_perm_lectura,
            inodo.i_perm_escritura,
            inodo.i_perm_ejecucion);
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
                fprintf(archivo,"inodo%d:N%d->bloque%d;\n",apActual,j,inodo.i_block[j]);
                definirCarpetaTree(archivo,path,inodo.i_block[j],sb);
            }
        }
    }
    else{
        for(j=0;j<12;j++){
            if(inodo.i_block[j]!=-1){
                fprintf(archivo,"inodo%d:N%d->bloque%d;\n",apActual,j,inodo.i_block[j]);
                definirArchivoTree(archivo,path,inodo.i_block[j],sb);
            }
        }
    }

    for (j=12;j<15;j++) {
        if(inodo.i_block[j]!=-1){
            fprintf(archivo,"inodo%d:N%d->bloque%d;\n",apActual,j,inodo.i_block[j]);
            if(inodo.i_type==0)
                definirIndirectoTree(archivo,path,inodo.i_block[j],sb,j-11,0);
            else
                definirIndirectoTree(archivo,path,inodo.i_block[j],sb,j-11,1);
        }
    }
}

void Administrador::definirCarpetaTree(FILE *archivo, char path[], int apActual, SuperBloque sb){

    BloqueCarpeta carpeta = getBloqueCarpeta(path,sb.s_block_start,apActual);
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
        if (carpeta.b_content[i].b_inodo > -1) {

            if(strcmp(carpeta.b_content[i].b_name,".")!=0&&strcmp(carpeta.b_content[i].b_name,"..")!=0){
                fprintf(archivo, "bloque%i:N%i->inodo%i;\n", apActual, i, carpeta.b_content[i].b_inodo);
                definirInodoTree(archivo,path,carpeta.b_content[i].b_inodo,sb);
            }
        }
    }
}

void Administrador::definirArchivoTree(FILE *archivo, char path[], int apActual, SuperBloque sb){
    BloqueArchivo bloque = getBloqueArchivo(path,sb.s_block_start,apActual);
    fprintf(archivo,"bloque%d[label=\"Bloque%d",apActual,apActual);
    char blocazo[65];

    for (int i=0;i<64;i++) {
        blocazo[i]=bloque.b_content[i];
    }
    blocazo[64]='\0';
    fprintf(archivo,"|%s",blocazo);
    fprintf(archivo, "\",color=white,fillcolor=\"#2A3D45\",fontcolor=white];\n");
}

void Administrador::definirIndirectoTree(FILE *archivo, char path[], int apActual, SuperBloque sb, int tipoIndirecto, int tipoBloque){
    BloqueApuntador bap = getBloqueApuntador(path,sb.s_block_start,apActual);
    fprintf(archivo,"bloque%d[label=\"Bloque%d",apActual,apActual);
    for (int i = 0;i<16;i++) {
        if(bap.b_pointers[i]>-1){
            fprintf(archivo,"|{AP%d|<N%d>}",i+1,i);
        }
    }
    fprintf(archivo, "\",color=white,fillcolor=\"#FFCF99\",fontcolor=white];\n");
    int i;
    if(tipoIndirecto==1){
        if(tipoBloque==0){
            //Jalamos un bloque carpeta
            for (i = 0;i<16;i++) {
                if(bap.b_pointers[i]>-1){
                    fprintf(archivo,"bloque%d:N%d->bloque%d;\n",apActual,i,bap.b_pointers[i]);
                    definirCarpetaTree(archivo,path,bap.b_pointers[i],sb);
                }
            }
        }else{
            //Jalamos el bloque archivo
            for (i = 0;i<16;i++) {
                if(bap.b_pointers[i]>-1){
                    fprintf(archivo,"bloque%d:N%d->bloque%d;\n",apActual,i,bap.b_pointers[i]);
                    definirArchivoTree(archivo,path,bap.b_pointers[i],sb);
                }
            }
        }
    }else{
        for (i =0;i<16;i++) {
            if(bap.b_pointers[i]!=-1){
                fprintf(archivo,"bloque%d:N%d->bloque%d;\n",apActual,i,bap.b_pointers[i]);
                definirIndirectoTree(archivo,path,bap.b_pointers[i],sb,tipoIndirecto-1,tipoBloque);
            }
        }
    }
}

BloqueArchivo Administrador::getBloqueArchivo(char path[], int inicio, int pos){
    BloqueArchivo b;
    b.b_content[0]='0';
    FILE* archivo = fopen(path,"rb+");
    if(archivo){
        fseek(archivo,inicio+pos*sizeof (BloqueArchivo),SEEK_SET);
        fread(&b,sizeof (BloqueArchivo),1,archivo);
        fclose(archivo);
    }
    return b;
}

BloqueCarpeta Administrador::getBloqueCarpeta(char path[],int inicio,int pos){
    BloqueCarpeta b;
    b.b_content[0].b_inodo=-2;
    FILE* archivo = fopen(path,"rb+");
    if(archivo){
        fseek(archivo,inicio+pos*sizeof (BloqueCarpeta),SEEK_SET);
        fread(&b,sizeof (BloqueCarpeta),1,archivo);
        fclose(archivo);
    }
    return b;
}

BloqueApuntador Administrador::getBloqueApuntador(char path[], int inicio, int pos){
    BloqueApuntador b;

    b.b_pointers[0]=-1;
    FILE* archivo = fopen(path,"rb+");
    if(archivo){
        fseek(archivo,inicio+pos*sizeof (BloqueApuntador),SEEK_SET);
        fread(&b,sizeof (BloqueApuntador),1,archivo);
        fclose(archivo);
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
            //repJournaling(funcion);
        }else if(funcion->nombre.compare("bm_inode")==0){
            repBMInode(funcion);
        }else if(funcion->nombre.compare("bm_block")==0){
            repBMBlock(funcion);
        }else if(funcion->nombre.compare("file")==0){
            //repFile(funcion);
        }else if(funcion->nombre.compare("inode")==0){
            //repInodo(funcion);
        }else if(funcion->nombre.compare("block")==0){
            //repBlock(funcion);
        }else if(funcion->nombre.compare("tree")==0){      //Primero
            repTree(funcion);
        }else if(funcion->nombre.compare("sb")==0){        //fresh
            repSuperBloque(funcion);
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

void Administrador::crearArchivo(Funcion *funcion){
    if(funcion->opciones[3]==1&&(funcion->opciones[0]==1||funcion->opciones[15])){
        if(sesion->usrid!=-1){
            int size=-1;
            string contenido="";
            if(funcion->opciones[15]==1){
                ifstream fichero(funcion->cont);
                contenido ="";
                string line;
                if(fichero.is_open()){
                    while(getline(fichero,line)){
                        contenido += line;
                    }
                    fichero.close();
                }else{
                    cerr<<"ERROR NO SE PUEDE ABRIR EL ARCHIVO CONT EN : "<<funcion->cont<<endl;
                    return;
                }

                size = contenido.size();
            }
            if(size==-1){
               size = funcion->size;
               contenido = "0123456789";
            }
            string pathDir=funcion->getAbsPath();
            vector<string> array_directorios;
            int contadorActual = 0;
            int cont = 0;
            funcion->getName();
            string nombre_archivo = funcion->fileName;
            bool banderaPath = false;
            char pDirectorios[300];
            strcpy(pDirectorios,pathDir.data());
            int numero_directorios = numeroDirectorios(pDirectorios);
            while(pathDir[cont]!=NULL&&cont<funcion->path.size()+1){
                if(pathDir[cont]=='/')
                {
                    cont++;
                    string tokenActual = "";
                    while(pathDir[cont]!=NULL&&pathDir[cont]!='/'){
                        tokenActual+=pathDir[cont];
                        cont++;
                    }
                    array_directorios.push_back(tokenActual);
                    contadorActual++;
                }
                else{
                    banderaPath=true;
                    break;
                }
            }

        SuperBloque sb = getSuperBloque(sesion->idPart);
        string idParticion = sesion->idPart;
        NodoParticion *part = listaDisco->existeId(idParticion);
        if(banderaPath==false){
            if(funcion->opciones[14]==1){
                //opcion -p
                crearPadres(sb,0,array_directorios,0,numero_directorios,part,numeroEstructuras(part->tamano,sb.s_filesystem_type));
                crearFile(sb,0,nombre_archivo,array_directorios,0,numero_directorios,part,numeroEstructuras(part->tamano,sb.s_filesystem_type),size,contenido);
            }else{
                //Solo crear la última carpeta
                crearFile(sb,0,nombre_archivo,array_directorios,0,numero_directorios,part,numeroEstructuras(part->tamano,sb.s_filesystem_type),size,contenido);
            }
        }else{
            cout<<"ERROR MKFILE, PATH MALFORMADO"<<endl;
        }
        }else{
            cout<<"ERROR, SESION NO INICIADA"<<endl;
        }
    }else{
        cout<<"ERROR, FALTAN PARAMETROS PARA MKFILE"<<endl;
    }
}

void Administrador::crearFile(SuperBloque sb,
                              int posBitmap, string nombre_archivo,
                              vector<string> array_directorios,
                              int posActualCarpeta,
                              int numero_directorios,
                              NodoParticion *part,
                              int nEstructuras,
                              int tamano,
                              string contenido){
    char path[200];
    strcpy(path,part->path);
    //Obtenemos la carpeta actual
    iNodo inodo_actual = getInodo(part->path,sb.s_inode_start,posBitmap);

    if(posActualCarpeta==numero_directorios){
        cerr<<"ERROR, ESTA REPETIDO PERRO:"<<array_directorios[numero_directorios-1]<<endl;
        return;
    }
    int encontro_directorio ;
    if(posActualCarpeta+1==numero_directorios){
        encontro_directorio = getBloqueCarpetaNombre(path,sb,inodo_actual,nombre_archivo);
    }else{
        encontro_directorio= getBloqueCarpetaNombre(path,sb,inodo_actual,array_directorios[posActualCarpeta]);
    }
    if(encontro_directorio==-1){
        if(numero_directorios==posActualCarpeta+1){
            insertarFile(sb,posBitmap,array_directorios,nombre_archivo,posActualCarpeta,numero_directorios,part,nEstructuras,path,inodo_actual,tamano,contenido);
        }else{
            cerr<<"ERROR, EL PATH NO EXISTE"<<endl;
        }
    }else{
        BloqueCarpeta carpeta = getBloqueCarpeta(path,sb.s_block_start,encontro_directorio);
        for (int i = 0;i<4;i++) {
            if(strcmp(carpeta.b_content[i].b_name,array_directorios.at(posActualCarpeta).data())==0){
                if(carpeta.b_content[i].b_inodo!=-1){
                    posBitmap = carpeta.b_content[i].b_inodo;
                    break;
                }
            }
        }
        crearFile(sb,posBitmap,nombre_archivo,array_directorios,posActualCarpeta+1,numero_directorios,part,nEstructuras,tamano,contenido);
    }
}

void Administrador::insertarFile(SuperBloque sb,
                                 int posBitmap,
                                 vector<string> array_directorios,
                                 string nombre_archivo,
                                 int posActualCarpeta,
                                 int numero_directorios,
                                 NodoParticion *part,
                                 int nEstructuras,
                                 char *path,
                                 iNodo nodo,
                                 int tamano,
                                 string contenido){
    if(posBitmap>=0){
        char uno = '1';
        int i;
        iNodo inodo_actual = getInodo(path,sb.s_inode_start,posBitmap);
        bool tiene_permiso = false;
        if(sesion->groupid==1){
            tiene_permiso = true;
        }
        if(!tiene_permiso){
            tiene_permiso = validarPermisoEscritura(inodo_actual);
        }
        if(tiene_permiso){
            int primer_inodo_libre = sb.s_first_ino;
            int libre_en_subdir = -1;
            int libre_en_carpeta=-1;
            for (i=0;i<12;i++) {
                if(inodo_actual.i_block[i]!=-1){
                    BloqueCarpeta carpeta = getBloqueCarpeta(path,sb.s_block_start,inodo_actual.i_block[i]);
                    int j;
                    for (j=0;j<4;j++) {
                        if(carpeta.b_content[j].b_inodo==-1){
                            libre_en_subdir=i;
                            libre_en_carpeta=j;

                        }
                    }
                    if(libre_en_carpeta!=-1)
                        break;
                }
                else{
                    libre_en_subdir=-2;
                }
            }
            if(libre_en_subdir!=-1){
                //Inserción en directos
                if(sb.s_filesystem_type==3){
                    Journal journal_archivo;
                    journal_archivo.log_tipo=1;
                    journal_archivo.log_tipo_operacion=0;
                    strcmp(journal_archivo.log_path,path); //funcion path
                    if(tamano>0)
                        journal_archivo.contenido=1;
                    else
                        journal_archivo.contenido=0;

                    journal_archivo.log_propietario=sesion->usrid;
                    journal_archivo.log_grupo = sesion->groupid;
                    strcpy(journal_archivo.log_nombre,nombre_archivo.data());
                    journal_archivo.ugo = 664;
                    escribirJournal(journal_archivo,path,sb,nEstructuras,part->byteInicio+sizeof (SuperBloque));
                }


                if(libre_en_subdir==-2){
                    for (i=0;i<12;i++) {
                        if(inodo_actual.i_block[i]==-1){
                            libre_en_subdir = i;
                            break;
                        }
                    }
                    int pos_nuevo_bcarpeta = sb.s_first_blo;
                    inodo_actual.i_block[libre_en_subdir]=pos_nuevo_bcarpeta;

                    escribirPosBitmap(sb.s_bm_block_start,sb.s_first_blo,path,uno);
                    sb.s_free_block_count--;
                    sb.s_first_blo = getFirstFreeBit(sb.s_bm_block_start,nEstructuras*3,path);
                    BloqueCarpeta carpeta_nueva;
                    for (i=0;i<4;i++) {
                        carpeta_nueva.b_content[i].b_inodo=-1;
                    }
                    libre_en_carpeta=0;
                    escribirBloqueCarpeta(carpeta_nueva,path,sb,pos_nuevo_bcarpeta);
                }
                BloqueCarpeta carpeta_escritura = getBloqueCarpeta(path,sb.s_block_start,inodo_actual.i_block[libre_en_subdir]);

                //Iniciamos a crear el inodo del archivo
                iNodo inodo_archivo = nuevoInodo(tamano,1);
                inodo_actual.i_mtime=time(0);
                strcpy(carpeta_escritura.b_content[libre_en_carpeta].b_name,nombre_archivo.data());
                carpeta_escritura.b_content[libre_en_carpeta].b_inodo=primer_inodo_libre;
                escribirInodo(inodo_archivo,path,sb,primer_inodo_libre);
                escribirInodo(inodo_actual,path,sb,posBitmap);
                escribirBloqueCarpeta(carpeta_escritura,path,sb,inodo_actual.i_block[libre_en_subdir]);
                escribirPosBitmap(sb.s_bm_inode_start,primer_inodo_libre,path,uno);
                sb.s_free_inode_count-=1;
                sb.s_first_ino=getFirstFreeBit(sb.s_bm_inode_start,nEstructuras,path);
                escribirSuperBloque(path,sb,part->byteInicio);
                SuperBloque sb1 = getSuperBloque(sesion->idPart);
                //Luego insertamos los bloques
                crearBloques(sb1,primer_inodo_libre,inodo_archivo,nEstructuras,path,tamano,contenido);
            }else{
                for (i=12;i<15;i++) {
                    if(inodo_actual.i_block[i]!=-1){
                        if(insertarFileApuntador(sb,posBitmap,nombre_archivo,part,nEstructuras,path,inodo_actual.i_block[i],i-11,tamano,contenido)==true){
                            break;
                        }
                    }else{
                        //Crear indirecto e insertar un nuevo bloque ahí
                        inodo_actual.i_block[i]=sb.s_first_blo;
                        escribirInodo(inodo_actual,path,sb,posBitmap);
                        crearBloqueApuntador(path,0,sb,i-11,inodo_actual.i_type,nEstructuras);
                        SuperBloque sb1 = getSuperBloque(sesion->idPart);
                        if(insertarFileApuntador(sb1,posBitmap,nombre_archivo,part,nEstructuras,path,inodo_actual.i_block[i],i-11,tamano,contenido))
                            break;
                    }
                }
            }

        }else{
            cerr<<"ERROR MkFILE, NO TIENE PERMISOS PARA ESCRIBIR"<<endl;
        }
    }else{
        cerr<<"POSBITMAP ES -1"<<endl;
    }
}

void Administrador::crearBloques(SuperBloque sb, int posBitmap, iNodo inodo_archivo, int nEstructuras, char *path, int tamano, string contenido){
    int numero_bloques = getNumeroBloques(tamano);
    vector<BloqueArchivo> bloques_archivos;
    int fin_contenido=10;
    if(contenido.size()>10){
        fin_contenido=tamano;
    }
    int i;
    int actual= tamano;
    int contador_contenido=0;
    for (i=0;i<numero_bloques;i++) {
        BloqueArchivo nuevo;
        for (int j=0;j<64;j++) {
            if(actual>0){
                if(fin_contenido<=contador_contenido)
                    contador_contenido=0;
                nuevo.b_content[j]=contenido[contador_contenido];
                contador_contenido++;
                actual--;
            }else{
                if(j<64 )
                    nuevo.b_content[j]='\0';
                break;
            }
        }
        bloques_archivos.push_back(nuevo);
    }
    for (i=0;i<numero_bloques;i++) {
        SuperBloque sb1 = getSuperBloque(sesion->idPart);
        inodo_archivo = getInodo(path,sb1.s_inode_start,posBitmap);
        insertarBloqueArchivo(sb1,posBitmap,inodo_archivo,nEstructuras,path,bloques_archivos[i]);
    }
}

void Administrador::insertarBloqueArchivo(SuperBloque sb,int posBitmap ,iNodo inodo_archivo, int nEstructuras, char *path,BloqueArchivo bloque_actual){
    char uno = '1';
    int i;
    int primer_bloque_libre = sb.s_first_blo;
    int libre_en_subdir = -1;

    for (i=0;i<12;i++) {
        if(inodo_archivo.i_block[i]==-1){
            libre_en_subdir = i;
            break;
        }
    }
    if(libre_en_subdir!=-1){
        //Directo
        inodo_archivo.i_block[libre_en_subdir]=primer_bloque_libre;
        inodo_archivo.i_mtime=time(0);
        escribirInodo(inodo_archivo,path,sb,posBitmap);
        escribirBloqueArchivo(bloque_actual,path,sb,primer_bloque_libre);
        escribirPosBitmap(sb.s_bm_block_start,primer_bloque_libre,path,uno);
        sb.s_first_blo = getFirstFreeBit(sb.s_bm_block_start,nEstructuras*3,path);
        sb.s_free_block_count-=1;
        NodoParticion*part = listaDisco->existeId(sesion->idPart);
        escribirSuperBloque(path,sb,part->byteInicio);

    }else{
        //Indirecto
        for(i=12;i<15;i++){
            if(inodo_archivo.i_block[i]!=-1){
                if(insertarBloqueArchivoApuntador(sb,posBitmap,nEstructuras,path,inodo_archivo.i_block[i],i-11,bloque_actual)==true){
                    break;
                }
            }else{
                //crear indirecto e insertar un nuevo bloque
                inodo_archivo.i_block[i]=sb.s_first_blo;
                escribirInodo(inodo_archivo,path,sb,posBitmap);
                crearBloqueApuntador(path,0,sb,i-11,inodo_archivo.i_type,nEstructuras);
                SuperBloque sb1 = getSuperBloque(sesion->idPart);
                if(insertarBloqueArchivoApuntador(sb1,posBitmap,nEstructuras,path,inodo_archivo.i_block[i],i-11,bloque_actual)==true)
                    break;
            }
        }
    }
}

bool Administrador::insertarBloqueArchivoApuntador(SuperBloque sb,int posInodo, int nEstructuras, char *path, int posApuntador, int tipoIndirecto,BloqueArchivo bloque_archivo){
    BloqueApuntador apuntador_actual = getBloqueApuntador(path,sb.s_block_start,posApuntador);
    iNodo inodo_archivo = getInodo(path,sb.s_inode_start,posInodo);
    if(sb.s_first_blo==365){
        cout<<"QUEPEDAL"<<endl;
    }
    int i = 0;
    int libre_en_subdir = -1;
    int primer_bloque_libre = sb.s_first_blo;
    char uno ='1';
    if(tipoIndirecto==1){
        for (i=0;i<16;i++) {
            if(apuntador_actual.b_pointers[i]==-1){
                libre_en_subdir=i;
                break;
            }
        }
        if(libre_en_subdir!=-1){
            //Inseracion en indirectos
            //inodo_archivo.i_mtime=time(0);
            apuntador_actual.b_pointers[libre_en_subdir]=primer_bloque_libre;
            //escribirInodo(inodo_archivo,path,sb,posInodo);
            escribirBloqueArchivo(bloque_archivo,path,sb,primer_bloque_libre);
            escribirPosBitmap(sb.s_bm_block_start,primer_bloque_libre,path,uno);
            sb.s_first_blo = getFirstFreeBit(sb.s_bm_block_start,nEstructuras*3,path);
            sb.s_free_block_count--;
            NodoParticion * part = listaDisco->existeId(sesion->idPart);
            escribirBloqueApuntador(apuntador_actual,path,sb,posApuntador);
            escribirSuperBloque(path,sb,part->byteInicio);
            return true;
        }
    }else{
        bool pudoEscribir = false;
        for (i=0;i<16;i++) {
            if(apuntador_actual.b_pointers[i]!=-1){
                pudoEscribir = insertarBloqueArchivoApuntador(sb,posInodo,nEstructuras,path,apuntador_actual.b_pointers[i],tipoIndirecto-1,bloque_archivo);
                if(pudoEscribir)
                    return  pudoEscribir;
            }
        }
        int j = -1;
        for (i=0;i<16;i++) {
            if(apuntador_actual.b_pointers[i]==-1){

                j=i;
                break;
            }
        }
        if(j!=-1){
            apuntador_actual.b_pointers[j]=sb.s_first_blo;
            escribirBloqueApuntador(apuntador_actual,path,sb,posApuntador);
            crearBloqueApuntador(path,0,sb,tipoIndirecto-1,inodo_archivo.i_type,nEstructuras);
            SuperBloque sb1 = getSuperBloque(sesion->idPart);
            return  insertarBloqueArchivoApuntador(sb1,posInodo,nEstructuras,path,apuntador_actual.b_pointers[j],tipoIndirecto-1,bloque_archivo);
        }else{
            return false;
        }
    }
    return false;

}

bool Administrador::insertarFileApuntador(SuperBloque sb, int posBitmap, string nombre_archivo, NodoParticion *part, int nEstructuras, char *path, int posApuntador, int tipoIndirecto,int tamano, string contenido){
    BloqueApuntador apuntador_actual = getBloqueApuntador(path,sb.s_block_start,posApuntador);
    iNodo inodo_actual = getInodo(path,sb.s_inode_start,posBitmap);
    int i = 0;
    int libre_en_subdir = -1;
    int libre_en_carpeta=-1;
    int primer_inodo_libre = sb.s_first_ino;
    char uno = '1';
    if(tipoIndirecto==1){
        for (i=0;i<16;i++) {
            if(apuntador_actual.b_pointers[i]!=-1){
                BloqueCarpeta carpeta = getBloqueCarpeta(path,sb.s_block_start,apuntador_actual.b_pointers  [i]);

                int j = 0;
                for (j=0;j<4;j++) {
                    if(carpeta.b_content[j].b_inodo==-1){
                        libre_en_subdir = i;
                        libre_en_carpeta = j;
                        break;
                    }
                }
                if(libre_en_carpeta!=-1)
                    break;
            }else{
                libre_en_subdir=-2;
            }
        }

        if(libre_en_subdir!=-1){
            //Inserción en indirectos
            if(sb.s_filesystem_type==3){
                Journal journal_carpeta;
                journal_carpeta.log_tipo=1;
                journal_carpeta.log_tipo_operacion=0;
                strcmp(journal_carpeta.log_path,path); //CHECK
                journal_carpeta.contenido=1;
                journal_carpeta.log_propietario=sesion->usrid;
                journal_carpeta.log_grupo = sesion->groupid;
                strcpy(journal_carpeta.log_nombre,nombre_archivo.data());
                journal_carpeta.ugo = 664;
                escribirJournal(journal_carpeta,path,sb,nEstructuras,part->byteInicio+sizeof (SuperBloque));
            }

            BloqueCarpeta carpeta_actual = getBloqueCarpeta(path,sb.s_block_start,inodo_actual.i_block[0]);

            if(libre_en_subdir==-2){
                for (i=0;i<16;i++) {
                    if(apuntador_actual.b_pointers[i]==-1){
                        libre_en_subdir = i;
                        break;
                    }
                }
                if(libre_en_subdir==-2){
                    //ta lleno
                    return false;
                }
                int pos_nuevo_bcarpeta = sb.s_first_blo;
                apuntador_actual.b_pointers[libre_en_subdir]=pos_nuevo_bcarpeta;

                escribirPosBitmap(sb.s_bm_block_start,sb.s_first_blo,path,uno);
                sb.s_free_block_count--;
                sb.s_first_blo = getFirstFreeBit(sb.s_bm_block_start,nEstructuras*3,path);
                BloqueCarpeta carpeta_nueva;
                for (i=0;i<4;i++) {
                    carpeta_nueva.b_content[i].b_inodo=-1;
                }
                libre_en_carpeta=0;
                escribirBloqueCarpeta(carpeta_nueva,path,sb,pos_nuevo_bcarpeta);
            }

            iNodo inodo_nuevo = nuevoInodo(tamano,1);
            BloqueCarpeta carpeta_escritura=getBloqueCarpeta(path,sb.s_block_start,apuntador_actual.b_pointers[libre_en_subdir]);

            inodo_actual.i_mtime=time(0);
            strcpy(carpeta_escritura.b_content[libre_en_carpeta].b_name,nombre_archivo.data());
            carpeta_escritura.b_content[libre_en_carpeta].b_inodo=primer_inodo_libre;
            escribirInodo(inodo_actual,path,sb,posBitmap);
            escribirInodo(inodo_nuevo,path,sb,primer_inodo_libre);
            escribirBloqueCarpeta(carpeta_escritura,path,sb,apuntador_actual.b_pointers[libre_en_subdir]);

            escribirPosBitmap(sb.s_bm_inode_start,primer_inodo_libre,path,uno);
            sb.s_free_inode_count--;
            sb.s_first_ino = getFirstFreeBit(sb.s_bm_inode_start,nEstructuras,path);
            escribirBloqueApuntador(apuntador_actual,path,sb,posApuntador);
            escribirSuperBloque(path,sb,part->byteInicio);
            crearBloques(sb,primer_inodo_libre,inodo_nuevo,nEstructuras,path,tamano,contenido);
            return true;
    }
    }else{
        bool pudoEscribir = false;
        for (i=0;i<16;i++) {
            if(apuntador_actual.b_pointers[i]!=-1){
                pudoEscribir = insertarFileApuntador(sb,posBitmap,nombre_archivo,part,nEstructuras,path,apuntador_actual.b_pointers[i],tipoIndirecto-1,tamano,contenido);
                if(pudoEscribir){
                    return pudoEscribir;
                }
            }
        }
        int j=-1;
        for (i=0;i<16;i++) {
            if(apuntador_actual.b_pointers[i]==-1){
                j = i;
                break;
            }
        }
        if(j!=-1){
            //Si es un segundo o tercer indirecto entonces verificamos que no tenga que crear un nuevo bloque indirecto
            apuntador_actual.b_pointers[j]=sb.s_first_blo;
            crearBloqueApuntador(path,0,sb,tipoIndirecto-1,inodo_actual.i_type,nEstructuras);
            SuperBloque sb1 = getSuperBloque(sesion->idPart);
            return insertarFileApuntador(sb1,posBitmap,nombre_archivo,part,nEstructuras,path,apuntador_actual.b_pointers[j],tipoIndirecto-1,tamano,contenido);
        }else{
            return false;
        }
    }
    return false;
}

int Administrador::numeroDirectorios(char *path){
    int numeroDirectorios = 0;
    int i = 0;
    while(path[i]!=NULL){
        if(path[i]=='/'){
            numeroDirectorios++;
        }
        i++;
    }
    return numeroDirectorios;
}

void Administrador::crearDirectorio(Funcion *funcion){
    if(funcion->opciones[3]==1&&funcion->opciones[8]){
        NodoParticion *part = listaDisco->existeId(sesion->idPart);
        if(part!=nullptr){
            if(sesion->usrid!=-1){
                char pathDir[funcion->path.size()+1];
                strcpy(pathDir,funcion->path.data());
                int numero_directorios = numeroDirectorios(pathDir);

                vector<string> array_directorios;
                int contadorActual = 0;
                int cont = 0;
                bool banderaPath = false;

                while(pathDir[cont]!=NULL&&cont<funcion->path.size()+1){
                    if(pathDir[cont]=='/')
                    {
                        cont++;
                        string tokenActual = "";
                        while(pathDir[cont]!=NULL&&pathDir[cont]!='/'){
                            tokenActual+=pathDir[cont];
                            cont++;
                        }
                        array_directorios.push_back(tokenActual);
                        contadorActual++;
                    }
                    else{
                        banderaPath=true;
                        break;
                    }
                }

                SuperBloque sb = getSuperBloque(sesion->idPart);
                if(banderaPath==false){
                    if(funcion->opciones[14]==1){
                        //opcion -p
                        crearPadres(sb,0,array_directorios,0,numero_directorios,part,numeroEstructuras(part->tamano,sb.s_filesystem_type));
                    }else{
                        //Solo crear la última carpeta
                        crearCarpeta(sb,0,array_directorios,0,numero_directorios,part,numeroEstructuras(part->tamano,sb.s_filesystem_type));
                    }
                }else{
                    cout<<"ERROR MKDIR, PATH MALFORMADO"<<endl;
                }
            }else {
                cout<<"ERROR MKDIR, SESION NO INICIADA"<<endl;
            }
        }else{
            cerr<<"ERROR MKDIR, PARTICION NO MONTADA"<<endl;
        }
    }else{
        cerr<<"ERROR MKDIR, FALTAN PARAMETROS"<<endl;
    }
}

iNodo Administrador::getInodo(char path[],int inicio, int pos){
    iNodo inodo;
    inodo.i_gid=-1;
    FILE* archivo = fopen(path,"rb+");
    if(archivo){
        fseek(archivo,inicio+pos*sizeof (iNodo),SEEK_SET);
        fread(&inodo,sizeof (iNodo),1,archivo);
        fclose(archivo);
    }
    return inodo;
}
void Administrador::crearPadres(SuperBloque sb, int posBitmap, vector<string >array_directorios, int posActualCarpeta, int numero_directorios, NodoParticion *part, int nEstructuras){
    char path[200];

    strcpy(path,part->path);
    //Obtener carpeta actual
    iNodo inodo_actual,inodo_siguiente;
    int posicionInodo = sb.s_inode_start+posBitmap*sizeof (iNodo);

    inodo_actual = getInodo(path,sb.s_inode_start,posBitmap);
    if(posActualCarpeta==numero_directorios){
        cout<<"Terminó de crear cosas :v "<<array_directorios[numero_directorios-1]<<endl;
        return;
    }
    int encontro_directorio = getBloqueCarpetaNombre(path,sb,inodo_actual,array_directorios[posActualCarpeta]);

//    int encontro_directorio=-1;
//    int i;
//    for (i=0;i<12;i++) {
//        if(inodo_actual.i_block[i]!=-1){
//            //Si es diferente de 0 jalamos el bloque carpeta
//            BloqueCarpeta carpeta= getBloqueCarpeta(path,sb.s_block_start,inodo_actual.i_block[i]);
//            for (int j = 0;j<4;j++) {
//                if(strcmp(carpeta.b_content[j].b_name,array_directorios[posActualCarpeta].data())==0){
//                    //Si son iguales
//                    encontro_directorio=1;
//                    posBitmap = carpeta.b_content[j].b_inodo;
//                }
//            }
//        }
//    }

    if(encontro_directorio == -1){
        //Si no lo encontró, vemos si se crea en esta dirección

        if(numero_directorios==posActualCarpeta+1){

            insertarCarpeta(sb,posBitmap,array_directorios,posActualCarpeta,numero_directorios,part,nEstructuras,path,inodo_actual);

        }else{
            if(numero_directorios==posActualCarpeta){
                cout<<"Terminó de crear cosas"<<endl;
            }else{
                insertarCarpeta(sb,posBitmap,array_directorios,posActualCarpeta,numero_directorios,part,nEstructuras,path,inodo_actual);
                SuperBloque sb1 = getSuperBloque(sesion->idPart);

                crearPadres(sb1,posBitmap,array_directorios,posActualCarpeta,numero_directorios,part,nEstructuras);
            }
        }
    }else{
        BloqueCarpeta carpeta = getBloqueCarpeta(path,sb.s_block_start,encontro_directorio);
        for (int i = 0;i<4;i++) {
            if(strcmp(carpeta.b_content[i].b_name,array_directorios.at(posActualCarpeta).data())==0){
                posBitmap = carpeta.b_content[i].b_inodo;
                break;
            }
        }
        //Si lo encontró en los directorios
        if(posActualCarpeta<numero_directorios)
            crearPadres(sb,posBitmap,array_directorios,posActualCarpeta+1,numero_directorios,part,nEstructuras);
    }
}
int Administrador::getBloqueCarpetaNombre(char path[],SuperBloque sb,iNodo inodo_actual,string nombre_directorio){
    int encontro_directorio=-1;
    int i;
    for (i=0;i<12;i++) {
        if(inodo_actual.i_block[i]!=-1){
            //Si es diferente de 0 jalamos el bloque carpeta
            BloqueCarpeta carpeta= getBloqueCarpeta(path,sb.s_block_start,inodo_actual.i_block[i]);
            for (int j = 0;j<4;j++) {
                if(strcmp(carpeta.b_content[j].b_name,nombre_directorio.data())==0){
                    //Si son iguales
                    encontro_directorio=inodo_actual.i_block[i];
                    break;
                }
            }
        }
    }
    if(encontro_directorio!=-1)
        return  encontro_directorio;
    else{
        //Buscar en indirectos
        for (i=12;i<15;i++) {
            if(inodo_actual.i_block[i]!=-1){
                int directorio = getBloqueCarpetaNombreIndirecto(path,sb,inodo_actual.i_block[i],nombre_directorio,i-11);
                if(directorio!=-1){
                    return directorio;
                }
            }
        }
    }
    return -1;
}


int Administrador::getBloqueCarpetaNombreIndirecto(char path[],SuperBloque sb,int bloqueApuntador,string nombre_directorio,int tipoIndirecto){
    int i;
    if(tipoIndirecto==1){

        //Sacamos un bloque directo
        BloqueApuntador bloque =  getBloqueApuntador(path,sb.s_block_start,bloqueApuntador);
        for (i=0;i<16;i++) {
            if(bloque.b_pointers[i]!=-1){
                BloqueCarpeta carpeta = getBloqueCarpeta(path,sb.s_block_start,bloque.b_pointers[i]);
                for (int j = 0;j<4;j++) {
                    if(strcmp(carpeta.b_content[j].b_name,nombre_directorio.data())==0){
                        return bloque.b_pointers[i];
                    }
                }
            }
        }
    }else{
        BloqueApuntador bloque = getBloqueApuntador(path,sb.s_block_start,bloqueApuntador);
        for (i=0;i<16;i++) {
            int test = getBloqueCarpetaNombreIndirecto(path,sb,bloque.b_pointers[i],nombre_directorio,tipoIndirecto-1);
            if(test!=-1){
                return test;
            }
        }
    }
    return -1;
}

void Administrador::crearCarpeta(SuperBloque sb, int posBitmap, vector<string> array_directorios, int posActualCarpeta, int numero_directorios, NodoParticion *part, int nEstructuras){
    char path[200];
    strcpy(path,part->path);
    //Obtener carpeta actual
    iNodo inodo_actual,inodo_siguiente;
    int posicionInodo = sb.s_inode_start+posBitmap*sizeof (iNodo);
    FILE* archivo = fopen(path,"rb+");
    fseek(archivo,posicionInodo,SEEK_SET);
    fread(&inodo_actual,sizeof (iNodo),1,archivo);
    fclose(archivo);
    if(posActualCarpeta==numero_directorios){
        cout<<"ERROR, ESTA REPETIDO PERRO:"<<array_directorios[numero_directorios-1]<<endl;
        return;
    }
    int encontro_directorio = getBloqueCarpetaNombre(path,sb,inodo_actual,array_directorios.at(posActualCarpeta));
//    int encontro_directorio=0;
//    int i;
//    for (i=0;i<12;i++) {
//        if(inodo_actual.i_block[i]!=-1){
//            //Si es diferente de 0 jalamos el bloque carpeta
//            BloqueCarpeta carpeta;
//            int indice = sb.s_block_start+inodo_actual.i_block[i]*sizeof (BloqueCarpeta);
//            archivo = fopen(path,"rb+");
//            fseek(archivo,indice,SEEK_SET);
//            fread(&carpeta,sizeof (BloqueCarpeta),1,archivo);
//            fclose(archivo);
//            for (int j = 0;j<4;j++) {
//                if(strcmp(carpeta.b_content[j].b_name,array_directorios[posActualCarpeta].data())==0){
//                    //Si son iguales
//                    encontro_directorio=1;
//                    posBitmap = carpeta.b_content[j].b_inodo;
//                }
//            }
//        }
//    }

    if(encontro_directorio == -1){
        //Si no lo encontró, vemos si se crea en esta dirección
        if(numero_directorios==posActualCarpeta+1){
            insertarCarpeta(sb,posBitmap,array_directorios,posActualCarpeta,numero_directorios,part,nEstructuras,path,inodo_actual);
        }else{
            cout<<"ERROR, EL PATH NO EXISTE"<<endl;
        }
    }else{
        //Actualizamos el posbitmap
        BloqueCarpeta carpeta = getBloqueCarpeta(path,sb.s_block_start,encontro_directorio);
        for (int i = 0;i<4;i++) {
            if(strcmp(carpeta.b_content[i].b_name,array_directorios.at(posActualCarpeta).data())==0){

                posBitmap = carpeta.b_content[i].b_inodo;
                if(posBitmap!=-1)
                    break;
            }
        }
        //Si lo encontró en los directorios
        crearCarpeta(sb,posBitmap,array_directorios,posActualCarpeta+1,numero_directorios,part,nEstructuras);
    }
}

void Administrador::insertarCarpeta(SuperBloque sb, int posBitmap, vector<string>array_directorios, int posActualCarpeta, int numero_directorios, NodoParticion *part, int nEstructuras, char *path, iNodo nodo){
    if(posBitmap>=0){
        char uno ='1';
        iNodo inodo_actual;
        int i;
        int inicio = sb.s_inode_start+posBitmap*sizeof (iNodo);
        FILE* archivo = fopen(path,"rb+");
        fseek(archivo,inicio,SEEK_SET);
        fread(&inodo_actual,sizeof (iNodo),1,archivo);
        fclose(archivo);

        bool tiene_permiso = false;
        if(sesion->groupid==1){
            tiene_permiso = true;
        }
        if(!tiene_permiso){
            tiene_permiso = validarPermisoEscritura(inodo_actual);
        }
        if(tiene_permiso){
            int primer_inodo_libre = sb.s_first_ino;
            int libre_en_subdir=-1;
            int libre_en_carpeta = -1;
           // archivo = fopen(path,"rb+");
            for (i=0;i<12;i++) {
                if(inodo_actual.i_block[i]!=-1){
                    BloqueCarpeta carpeta = getBloqueCarpeta(path,sb.s_block_start,inodo_actual.i_block[i]);

                    int j = 0;
                    for (j=0;j<4;j++) {
                        if(carpeta.b_content[j].b_inodo==-1){
                            libre_en_subdir = i;
                            libre_en_carpeta = j;
                            break;
                        }
                    }
                    if(libre_en_carpeta!=-1)
                        break;
                }else{
                    libre_en_subdir=-2;
                }
            }
            if(libre_en_subdir!=-1){
                //Inserción en directos
                if(sb.s_filesystem_type==3){
                    Journal journal_carpeta;
                    journal_carpeta.log_tipo=0;
                    journal_carpeta.log_tipo_operacion=0;
                    strcmp(journal_carpeta.log_path,path); //CHECK
                    journal_carpeta.contenido=0;
                    journal_carpeta.log_propietario=sesion->usrid;
                    journal_carpeta.log_grupo = sesion->groupid;
                    strcpy(journal_carpeta.log_nombre,array_directorios[posActualCarpeta].data());
                    journal_carpeta.ugo = 664;
                    escribirJournal(journal_carpeta,path,sb,nEstructuras,part->byteInicio+sizeof (SuperBloque));
                }

                BloqueCarpeta carpeta_actual = getBloqueCarpeta(path,sb.s_block_start,inodo_actual.i_block[0]);

                if(libre_en_subdir==-2){
                    for (i=0;i<12;i++) {
                        if(inodo_actual.i_block[i]==-1){
                            libre_en_subdir = i;
                            break;
                        }
                    }
                    int pos_nuevo_bcarpeta = sb.s_first_blo;
                    inodo_actual.i_block[libre_en_subdir]=pos_nuevo_bcarpeta;

                    escribirPosBitmap(sb.s_bm_block_start,sb.s_first_blo,path,uno);
                    sb.s_free_block_count--;
                    sb.s_first_blo = getFirstFreeBit(sb.s_bm_block_start,nEstructuras*3,path);
                    BloqueCarpeta carpeta_nueva;
                    for (i=0;i<4;i++) {
                        carpeta_nueva.b_content[i].b_inodo=-1;
                    }
                    libre_en_carpeta=0;
                    escribirBloqueCarpeta(carpeta_nueva,path,sb,pos_nuevo_bcarpeta);
                }



                iNodo inodo_nuevo = nuevoInodo(0,0);
                BloqueCarpeta carpeta_nueva= carpetaInicial(carpeta_actual.b_content[0].b_inodo,primer_inodo_libre);
                //archivo = fopen(path,"rb+");
                BloqueCarpeta carpeta_escritura=getBloqueCarpeta(path,sb.s_block_start,inodo_actual.i_block[libre_en_subdir]);
                //fclose(archivo);
                inodo_actual.i_mtime=time(0);
                strcpy(carpeta_escritura.b_content[libre_en_carpeta].b_name,array_directorios[posActualCarpeta].data());
                carpeta_escritura.b_content[libre_en_carpeta].b_inodo=primer_inodo_libre;
                inodo_nuevo.i_block[0]=sb.s_first_blo;
                escribirInodo(inodo_actual,path,sb,posBitmap);
                escribirInodo(inodo_nuevo,path,sb,primer_inodo_libre);
                escribirBloqueCarpeta(carpeta_escritura,path,sb,inodo_actual.i_block[libre_en_subdir]);
                escribirBloqueCarpeta(carpeta_nueva,path,sb,sb.s_first_blo);

                escribirPosBitmap(sb.s_bm_block_start,sb.s_first_blo,path,uno);
                sb.s_free_block_count-=1;
                escribirPosBitmap(sb.s_bm_inode_start,primer_inodo_libre,path,uno);
                sb.s_free_inode_count-=1;
                sb.s_first_blo = getFirstFreeBit(sb.s_bm_block_start,nEstructuras*3,path);
                sb.s_first_ino = getFirstFreeBit(sb.s_bm_inode_start,nEstructuras,path);

                escribirSuperBloque(path,sb,part->byteInicio);
            }else{
                //Inserción en indirectos;CHECK
                for (i=12;i<15;i++) {
                    if(inodo_actual.i_block[i]!=-1){
                        if(insertarCarpetaApuntador(sb,posBitmap,array_directorios,posActualCarpeta,numero_directorios,part,nEstructuras,path,inodo_actual.i_block[i],i-11)==true){
                            break;
                        }
                    }else{
                        //Crear indirecto e insertar un nuevo bloque ahí
                        inodo_actual.i_block[i]=sb.s_first_blo;
                        escribirInodo(inodo_actual,path,sb,posBitmap);
                        crearBloqueApuntador(path,0,sb,i-11,inodo_actual.i_type,nEstructuras);
                        SuperBloque sb1 = getSuperBloque(sesion->idPart);
                        if(insertarCarpetaApuntador(sb1,posBitmap,array_directorios,posActualCarpeta,numero_directorios,part,nEstructuras,path,inodo_actual.i_block[i],i-11))
                            break;
                    }
                }
            }
        }else{
            cout<<"ERROR MKDIR, NO TIENE PERMISOS PARA ESCRITURA"<<endl;
        }
    }else{
        cout<<"     POSBITMAP ES -1"<<endl;
    }

}


bool Administrador::insertarCarpetaApuntador(SuperBloque sb, int posBitmap, vector<string> array_directorios, int posActualCarpeta, int numero_directorios, NodoParticion *part, int nEstructuras, char *path, int posApuntador, int tipoIndirecto){
    BloqueApuntador apuntador_actual = getBloqueApuntador(path,sb.s_block_start,posApuntador);
    iNodo inodo_actual = getInodo(path,sb.s_inode_start,posBitmap);
    int i = 0;
    int libre_en_subdir = -1;
    int libre_en_carpeta=-1;
    int primer_inodo_libre = sb.s_first_ino;
    char uno = '1';
    if(tipoIndirecto==1){
        for (i=0;i<16;i++) {
            if(apuntador_actual.b_pointers[i]!=-1){
                BloqueCarpeta carpeta = getBloqueCarpeta(path,sb.s_block_start,apuntador_actual.b_pointers  [i]);

                int j = 0;
                for (j=0;j<4;j++) {
                    if(carpeta.b_content[j].b_inodo==-1){
                        libre_en_subdir = i;
                        libre_en_carpeta = j;
                        break;
                    }
                }
                if(libre_en_carpeta!=-1)
                    break;
            }else{
                libre_en_subdir=-2;
            }
        }

        if(libre_en_subdir!=-1){
            //Inserción en indirectos
            if(sb.s_filesystem_type==3){
                Journal journal_carpeta;
                journal_carpeta.log_tipo=0;
                journal_carpeta.log_tipo_operacion=0;
                strcmp(journal_carpeta.log_path,path); //CHECK
                journal_carpeta.contenido=0;
                journal_carpeta.log_propietario=sesion->usrid;
                journal_carpeta.log_grupo = sesion->groupid;
                strcpy(journal_carpeta.log_nombre,array_directorios[posActualCarpeta].data());
                journal_carpeta.ugo = 664;
                escribirJournal(journal_carpeta,path,sb,nEstructuras,part->byteInicio+sizeof (SuperBloque));
            }

            BloqueCarpeta carpeta_actual = getBloqueCarpeta(path,sb.s_block_start,inodo_actual.i_block[0]);

            if(libre_en_subdir==-2){
                for (i=0;i<16;i++) {
                    if(apuntador_actual.b_pointers[i]==-1){
                        libre_en_subdir = i;
                        break;
                    }
                }
                if(libre_en_subdir==-2){
                    //ta lleno
                    return false;
                }
                int pos_nuevo_bcarpeta = sb.s_first_blo;
                apuntador_actual.b_pointers[libre_en_subdir]=pos_nuevo_bcarpeta;

                escribirPosBitmap(sb.s_bm_block_start,sb.s_first_blo,path,uno);
                sb.s_free_block_count--;
                sb.s_first_blo = getFirstFreeBit(sb.s_bm_block_start,nEstructuras*3,path);
                BloqueCarpeta carpeta_nueva;
                for (i=0;i<4;i++) {
                    carpeta_nueva.b_content[i].b_inodo=-1;
                }
                libre_en_carpeta=0;
                escribirBloqueCarpeta(carpeta_nueva,path,sb,pos_nuevo_bcarpeta);
            }

            iNodo inodo_nuevo = nuevoInodo(0,0);
            BloqueCarpeta carpeta_nueva= carpetaInicial(carpeta_actual.b_content[0].b_inodo,primer_inodo_libre);
            BloqueCarpeta carpeta_escritura=getBloqueCarpeta(path,sb.s_block_start,apuntador_actual.b_pointers[libre_en_subdir]);

            inodo_actual.i_mtime=time(0);
            strcpy(carpeta_escritura.b_content[libre_en_carpeta].b_name,array_directorios[posActualCarpeta].data());
            carpeta_escritura.b_content[libre_en_carpeta].b_inodo=primer_inodo_libre;
            inodo_nuevo.i_block[0]=sb.s_first_blo;
            escribirInodo(inodo_actual,path,sb,posBitmap);
            escribirInodo(inodo_nuevo,path,sb,primer_inodo_libre);
            escribirBloqueCarpeta(carpeta_escritura,path,sb,apuntador_actual.b_pointers[libre_en_subdir]);
            escribirBloqueCarpeta(carpeta_nueva,path,sb,sb.s_first_blo);

            escribirPosBitmap(sb.s_bm_block_start,sb.s_first_blo,path,uno);
            sb.s_free_block_count--;
            escribirPosBitmap(sb.s_bm_inode_start,primer_inodo_libre,path,uno);
            sb.s_free_inode_count--;
            sb.s_first_blo = getFirstFreeBit(sb.s_bm_block_start,nEstructuras*3,path);
            sb.s_first_ino = getFirstFreeBit(sb.s_bm_inode_start,nEstructuras,path);
            escribirBloqueApuntador(apuntador_actual,path,sb,posApuntador);
            escribirSuperBloque(path,sb,part->byteInicio);
            return true;
    }
    }else{
        bool pudoEscribir = false;
        for (i=0;i<16;i++) {
            if(apuntador_actual.b_pointers[i]!=-1){
                pudoEscribir = insertarCarpetaApuntador(sb,posBitmap,array_directorios,posActualCarpeta,numero_directorios,part,nEstructuras,path,apuntador_actual.b_pointers[i],tipoIndirecto-1);
                if(pudoEscribir){
                    return pudoEscribir;
                }
            }
        }
        int j=-1;
        for (i=0;i<16;i++) {
            if(apuntador_actual.b_pointers[i]==-1){
                j = i;
                break;
            }
        }
        if(j!=-1){
            //Si es un segundo o tercer indirecto entonces verificamos que no tenga que crear un nuevo bloque indirecto
            apuntador_actual.b_pointers[j]=sb.s_first_blo;
            crearBloqueApuntador(path,0,sb,tipoIndirecto-1,inodo_actual.i_type,nEstructuras);
            SuperBloque sb1 = getSuperBloque(sesion->idPart);
            return insertarCarpetaApuntador(sb1,posBitmap,array_directorios,posActualCarpeta,numero_directorios,part,nEstructuras,path,apuntador_actual.b_pointers[j],tipoIndirecto-1);
        }else{
            return false;
        }
    }
    return false;
}

BloqueApuntador Administrador::getApuntadorLibre(char path[],int inicio,int posActual,int tipoIndirecto){
    BloqueApuntador actual = getBloqueApuntador(path,inicio,posActual);
    for (int i =0;i<16;i++) {
        if(actual.b_pointers[i]==-1){
            return actual;
        }else{
            //CHECK
            return getApuntadorLibre(path,inicio,actual.b_pointers[i],tipoIndirecto-1);
        }
    }
}


void Administrador::crearBloqueApuntador(char path[], int apActual, SuperBloque sb, int tipoIndirecto,int tipoBloque, int nEstructuras){
    BloqueApuntador nuevo;
    int i ;

    for (i=0;i<16;i++) {
        nuevo.b_pointers[i]=-1;
    }

    if(tipoIndirecto==1){
        escribirBloqueApuntador(nuevo,path,sb,sb.s_first_blo);
        escribirPosBitmap(sb.s_bm_block_start,sb.s_first_blo,path,'1');
        sb.s_free_block_count--;
        sb.s_first_blo=getFirstFreeBit(sb.s_bm_block_start,nEstructuras*3,path);
        escribirSuperBloque(path,sb,listaDisco->existeId(sesion->idPart)->byteInicio);

    }else{
        int actual = sb.s_first_blo;
        escribirPosBitmap(sb.s_bm_block_start,sb.s_first_blo,path,'1');
        sb.s_free_block_count--;
        sb.s_first_blo=getFirstFreeBit(sb.s_bm_block_start,nEstructuras*3,path);
        nuevo.b_pointers[0]=sb.s_first_blo;
        escribirBloqueApuntador(nuevo,path,sb,actual);
        escribirSuperBloque(path,sb,listaDisco->existeId(sesion->idPart)->byteInicio);
        SuperBloque sb1 = getSuperBloque(sesion->idPart);
        crearBloqueApuntador(path,apActual,sb1,tipoIndirecto-1,tipoBloque,nEstructuras);

    }

}
void Administrador::escribirBloqueApuntador(BloqueApuntador bloque_apuntador, char *path, SuperBloque sb, int posicion){
    FILE* archivo = fopen(path,"rb+");
    if(archivo){
        fseek(archivo,sb.s_block_start+posicion*sizeof (BloqueApuntador),SEEK_SET);
        fwrite(&bloque_apuntador,sizeof (BloqueApuntador),1,archivo);
        fclose(archivo);
    }
}

void Administrador::escribirBloqueArchivo(BloqueArchivo bloque_archivo, char *path, SuperBloque sb, int posicion){
    FILE* archivo = fopen(path,"rb+");
    if(archivo){
        fseek(archivo,sb.s_block_start+posicion*sizeof (BloqueArchivo),SEEK_SET);
        fwrite(&bloque_archivo,sizeof (BloqueArchivo),1,archivo);
        fclose(archivo);
    }else{
        cerr<<"ERROR, ESCRIBIR BLOQUE ARCHIVO, EL PATH NO EXISTE"<<endl;
    }
}

void Administrador::escribirSuperBloque(char *path, SuperBloque sb,int inicio){
    FILE* archivo = fopen(path,"rb+");
    if(archivo){
        fseek(archivo,inicio,SEEK_SET);
        fwrite(&sb,sizeof (SuperBloque),1,archivo);
        fclose(archivo);
    }
}

void Administrador::escribirPosBitmap(int inicio, int posicion,char path[],char valor){
    FILE* archivo = fopen(path,"rb+");
    cout<<"             POSICION A ESCRIBIR:" <<posicion<<endl;
    if(archivo){
        fseek(archivo,inicio+posicion,SEEK_SET);
        fwrite(&valor,1,1,archivo);
        fclose(archivo);
    }
}

void Administrador::escribirBloqueCarpeta(BloqueCarpeta carpeta,char*path,SuperBloque sb, int posicion){
    FILE* archivo = fopen(path,"rb+");
    int inicio = sb.s_block_start+posicion*sizeof (BloqueCarpeta);
    if(archivo){

        fseek(archivo,inicio,SEEK_SET);
        fwrite(&carpeta,sizeof (BloqueCarpeta),1,archivo);
        fclose(archivo);
    }else{
        cout<<"ERROR MKDIR, AL ESCRIBIR BLOQUECARPETA:"<<carpeta.b_content[0].b_inodo;
    }
}
void Administrador::escribirInodo(iNodo inodo,char* path, SuperBloque sb,int posicion){
    FILE* archivo = fopen(path,"rb+");
    int inicio = sb.s_inode_start+posicion*sizeof (iNodo);
    if(archivo){

        fseek(archivo,inicio,SEEK_SET);
        fwrite(&inodo,sizeof (iNodo),1,archivo);
        fclose(archivo);
    }else{
        cout<<"ERROR MKDIR, AL ESCRIBIR NODO:"<<(int)inodo.i_type;
    }
}
bool Administrador::validarPermisoEscritura(iNodo inodo_actual){
    return true;
}
BloqueCarpeta Administrador::carpetaInicial(int padre, int actual){
    BloqueCarpeta carpeta;
    strcpy(carpeta.b_content[0].b_name,".");
    carpeta.b_content[0].b_inodo = actual;
    strcpy(carpeta.b_content[1].b_name,"..");
    carpeta.b_content[1].b_inodo=padre;
    carpeta.b_content[2].b_inodo=-1;
    carpeta.b_content[3].b_inodo =-1;
    return  carpeta;
}
iNodo Administrador::nuevoInodo(int tamano,char tipo){
    iNodo nuevo;
    nuevo.i_gid=sesion->groupid;
    nuevo.i_uid = sesion->usrid;
    nuevo.i_size = tamano;
    nuevo.i_type = tipo;
    nuevo.i_atime = nuevo.i_ctime = nuevo.i_mtime = time(0);
    for (int i = 0;i<15;i++) {
        nuevo.i_block[i]=-1;
    }
    nuevo.i_perm_lectura=6;
    nuevo.i_perm_escritura=4;
    nuevo.i_perm_ejecucion=4;
    return nuevo;

}
void Administrador::recuperar(Funcion *funcion){

}

void Administrador::perdida(Funcion *funcion){

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
    }else if(path[tamano-1]=='v'){
        sprintf(cmd,"dot -Tsvg  '/home/mia/Reportes/%s.dot' -o %s",nombreRep.data(),funcion->path.data());
        system(cmd);
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    else{
        printf("formato no reconocido\n");
    }
}

int Administrador::getFirstFreeBit(int inicio, int nEstruct, char path[]){
    int posActual =-1;
    FILE* archivo = fopen(path,"rb+");
    if(archivo){
        fseek(archivo,inicio,SEEK_SET);
        char bit;
        for (int i =0;i<nEstruct;i++) {
            fread(&bit,sizeof(char),1,archivo);
            if(bit=='0'){
                posActual=i;
                cout<<"         POSICION LEIDA : "<<i<<endl;
                break;
            }
        }
        fclose(archivo);
    }
    return posActual;
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
    if(archivo){
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
        fclose(archivo);
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

