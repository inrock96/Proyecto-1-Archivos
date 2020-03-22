#include "interprete.h"

Interprete::Interprete()
{
    admin = new Administrador();
}

void Interprete::ejecutar(string entrada){
    int tamanoEntrada = entrada.length()+1;
    char comando[tamanoEntrada];
    bool bandera = false;
    funcion = new Funcion();
    string tokenActual;
    int contador = 0;
    int idCount = 0;
    int fileCount = 0;
    strcpy(comando,entrada.c_str());
    //Pasar todo a lowercase
    while(!esEspacio(comando[contador])&&contador<tamanoEntrada){
        if(esLetra(comando[contador]))
            comando[contador]=tolower(comando[contador]);
        contador++;
    }
    contador = 0;
    cout<<"Comando a ejecutar: "<<comando<<endl<<endl;
    funcion->mostrar();

    /*************************INICIA LECTURA*************************/
    while(contador<tamanoEntrada&&comando[contador]!=NULL){
        /*Viene un comentario*/
        if(comando[contador]=='#'){
            //cout<<"Reconoció comentario: ";
            while (comando[contador]!=NULL) {
                tokenActual+=comando[contador];
                contador++;
            }
            //cout<<tokenActual<<endl;

            break;
         /*Viene un espacio*/
        }
        /*Viene un espacio*/
        else if(esEspacio(comando[contador])){
            cout<<"Reconoció espacio"<<endl;
            contador++;
        /*Viene una letra*/
        }else if(esLetra(comando[contador])){
            /*Viene una instrucción*/
            if(!bandera){
                tokenActual = "";
                tokenActual="";
                while (esLetra(comando[contador])) {
                    tokenActual+=comando[contador];
                    contador++;
                }
                funcion->funcion = getOpcion(tokenActual);
                if(funcion->funcion<0)
                 {
                    cout<<"Error, esa función no existe"<<endl;
                    break;
                }
                if(funcion->funcion==26){
                        system("read -p 'Estamos en pausa, presione enter para continuarx|...' var");
                    try {
                        int c = getchar();
                    } catch (exception) {
                        cout<<"No se qué pasó :V"<<endl;
                    }
                    break;
                }
                bandera=!bandera;
            }else{
                cout<<"Error, no se pueden ingresar dos funciones en la misma linea"<<endl;
                break;
            }
        }else if(comando[contador]=='-'){
            int contador2 = contador;
            while(!esEspacio(comando[contador2])&&comando[contador2]!='='){
                if(esLetra(comando[contador2]))
                    comando[contador2]=tolower(comando[contador2]);
                else if(contador2>100)
                    break;
                contador2++;
            }

            tokenActual = "";
            contador++;
            //Lee el id del parametro
            if(comando[contador]=='p'||comando[contador]=='r'){
                if(!esLetra(comando[contador+1])){
                    tokenActual = comando[contador];
                }else{
                    while (comando[contador]!='=') {
                        if(esLetra(comando[contador]))
                            tokenActual+=comando[contador];
                        contador++;
                    }
                }
            }else{
                while (comando[contador]!='=') {
                    if(esLetra(comando[contador]))
                        tokenActual+=comando[contador];
                    contador++;
                }
            }
            /*Se aumenta uno por el contador+1*/
            contador++;
            int opActual = getParam(tokenActual);
            /*Si la opción es valida, se guarda*/

            if(opActual>=0)
                funcion->opciones[opActual]=1;

            //contador++;
            while(esEspacio(comando[contador]))
                contador++;
            cout<<"Se reconoció el parametro: "<<tokenActual<<endl<<endl;
            tokenActual="";

            /*********************************************************/
            /************Ahora guarda el valor del parametro**********/
            /*********************************************************/
            /*ahora conseguimos el valor de ese parametro*/
            if(opActual!=13&&opActual!=14){
                if(comando[contador]=='\"'){
                    /*entoeees verifico si puede venir una cadena*/
                    /*path, name, id, usr,pwd,grp, cont, dest,fs, file*/
                    if(opActual==3||opActual==8||opActual==6||opActual==9||opActual==10||opActual==11||opActual==15||opActual==16||opActual==17||opActual==18||opActual==19){
                        contador++;
                        do {
                            if(comando[contador]=='/'){
                                funcion->fileName.clear();
                                tokenActual+=comando[contador];
                                contador++;
                            }
                            if(opActual==3)
                                funcion->fileName+=comando[contador];
                            tokenActual+=comando[contador];
                            contador++;
                        } while (comando[contador]!='\"'&&contador<100&&comando[contador-1]!='\"');
                        contador++;
                        cout<<"Se reconoció la cadena: "<<tokenActual<<endl;
                        if(opActual==3){
                            funcion->path=tokenActual;
                        }else if (opActual==6) {
                            funcion->nombre=tokenActual;
                        }else if(opActual==8){
                            funcion->id[idCount++] = tokenActual;
                        }else if(opActual==9){
                            funcion->usr = tokenActual;
                        }else if(opActual==10){
                            funcion->pwd = tokenActual;
                        }else if(opActual==11){
                            funcion->grp = tokenActual;
                        }else if(opActual==15){
                            funcion->cont = tokenActual;
                        }else if(opActual==16){
                            funcion->file[fileCount++] = tokenActual;
                        }else if(opActual==17){
                            funcion->dest = tokenActual;
                        }else if(opActual==19){
                            funcion->ruta = tokenActual;
                        }
                    }
                }else if(comando[contador]=='/'){
                    /*verificamos que sea una path*/
                    if(opActual==3||opActual==15||opActual==16||opActual==17||opActual==20){
                        while (comando[contador]!='\0'&&comando[contador]!=' '&&comando[contador]!=NULL) {
                            if(comando[contador]=='/')
                                funcion->fileName.clear();
                            funcion->fileName+=comando[contador];
                            tokenActual+=comando[contador];
                            contador++;
                        }
                        switch (opActual) {
                        case 3:
                            funcion->path=tokenActual;
                            cout<<"se guardo el path: "<<funcion->path<<endl;
                            break;
                        case 15:
                            funcion->cont=tokenActual;
                            cout<<"se guardo el cont: "<<funcion->cont<<endl;
                            break;
                        case 16:
                            funcion->file[fileCount++]=tokenActual;
                            cout<<"se guardo el file: "<<funcion->file[fileCount-1]<<endl;
                            break;
                        case 17:
                            funcion->dest=tokenActual;
                            cout<<"se guardo el dest: "<<funcion->dest<<endl;
                            break;
                        case 20:
                            funcion->ruta=tokenActual;
                            cout<<"se guardo el ruta: "<<funcion->ruta<<endl;
                            break;
                        default:
                            break;
                        }
                    }else{
                        cout<<"Error, este parametro no tiene path"<<endl;
                    }
                    }else if(esLetra(comando[contador])){
                    /*verificamos si lo que queremos es un identificador*/
                    if(opActual==1||opActual==2||opActual==4||opActual==5||opActual==6||opActual==8||opActual==9||opActual==10||opActual==11||opActual==18){
                        tokenActual+=comando[contador];
                        contador++;
                        while(esId(comando[contador])){
                            tokenActual+=comando[contador];
                            contador++;
                        }
                        /* 0 size
                         * 1 fit
                         * 2 unit
                         * 3 path
                         * 4 type
                         * 5 delete
                         * 6 name
                         * 7 add
                         * 8 id
                         */
                        switch (opActual) {
                        case 1:
                            /*verificamos que sea fit*/
                            std::transform(tokenActual.begin(), tokenActual.end(), tokenActual.begin(),
                                [](unsigned char c){ return std::tolower(c); });
                            if(tokenActual.compare("bf")==0||tokenActual.compare("wf")==0||tokenActual.compare("ff")==0){
                                funcion->fit=tokenActual[0];
                            }else{
                                cout<<"Error, fit no puede tener ese valor"<<endl;
                            }
                            break;
                        case 2:
                            std::transform(tokenActual.begin(), tokenActual.end(), tokenActual.begin(),
                                [](unsigned char c){ return std::tolower(c); });
                            if(tokenActual.compare("k")==0||tokenActual.compare("m")==0||tokenActual.compare("b")){
                                funcion->unit=tokenActual[0];
                                cout<<"se guardo unit: "<<funcion->unit<<endl;
                            }else{
                                cout<<"Error, unit no puede tener ese valor"<<endl;
                            }
                            /*verifica si es unit*/
                            break;
                        case 4:
                            /*verificamos el tipo*/
                            std::transform(tokenActual.begin(), tokenActual.end(), tokenActual.begin(),
                                [](unsigned char c){ return std::tolower(c); });
                            if(tokenActual.compare("e")==0||tokenActual.compare("p")==0||tokenActual.compare("l")==0){
                                funcion->type=tokenActual[0];
                                cout<<"se guardo type: "<<funcion->type<<endl;
                            }else if(tokenActual.compare("fast")||tokenActual.compare("full")){
                                funcion->eliminar = tokenActual;
                                cout<<"se guardo type: "<<funcion->eliminar<<endl;
                            }else{
                                cout<<"Error, tipo no puede tener ese valor"<<endl;
                            }
                            break;
                        case 5:
                            /*fast o full*/
                            std::transform(tokenActual.begin(), tokenActual.end(), tokenActual.begin(),
                                [](unsigned char c){ return std::tolower(c); });
                            if(tokenActual.compare("full")==0||tokenActual.compare("fast")==0){
                                funcion->eliminar=tokenActual;
                                cout<<"El valor de eliminar es: "<<funcion->eliminar<<endl;
                            }else{
                                cout<<"Error, el parametro elimiar recibe \"full\" o \"\fast"<<endl;
                            }
                            break;
                        case 6:
                            funcion->nombre = tokenActual;
                            cout<<"El valor de nombre es: "<<funcion->nombre<<endl;
                            //Todo
                            break;
                        case 8:
                            funcion->id[idCount++] = tokenActual;
                            cout<<"El valor de id"<<idCount-1<<"es: "<<funcion->id[idCount-1]<<endl;
                            //Todo
                            break;
                        case 9:
                            funcion->usr = tokenActual;
                            cout<<"El valor de usuario es: "<<funcion->usr<<endl;
                            break;
                        case 10:
                            funcion->pwd = tokenActual;
                            cout<<"El valor de password es: "<<funcion->pwd<<endl;
                            break;
                        case 11:
                            funcion->grp = tokenActual;
                            cout<<"El valor de grupo es: "<<funcion->grp<<endl;
                            break;
                        case 15:
                            funcion->cont = tokenActual;
                            cout<<"El valor de cont es: "<<funcion->cont<<endl;
                            break;

                        case 17:
                            funcion->dest = tokenActual;
                            cout<<"El valor de dest es: "<<funcion->id<<endl;
                            break;
                        case 19:
                            funcion->ruta = tokenActual;
                            break;
                        default:
                            cout<<"valor raro"<<endl;
                            break;
                        }
                    }else{
                        cout<<"Error, el valor de este parametro es un identificador"<<endl;
                    }
                }else if(esNumero(comando[contador])){
                    /*verificamos si lo que queremos sea una número*/
                    if(opActual==0||opActual==7||opActual==12||opActual==10||opActual==18){
                        /*Si puede ser número*/
                        while(esNumero(comando[contador])){
                            tokenActual+=comando[contador];
                            contador++;
                        }
                        if(opActual==18){
                            while (esLetra(comando[contador])) {
                                contador++;
                            }
                            funcion->fs= stoi(tokenActual);
                            cout<<"El parametro fs tiene el valor: "<<funcion->fs<<endl;
                        }
                        if(opActual==0){
                            funcion->size = stoi(tokenActual);
                            cout<<"El parametro size tiene el valor: "<<funcion->size<<endl;
                        }else if (opActual==7){
                            funcion->add = stoi(tokenActual);
                            cout<<"El parametro add tiene el valor: "<<funcion->add<<endl;
                        }else if(opActual==12){
                            funcion->ugo = stoi(tokenActual);
                            cout<<"El parametro ugo tiene el valor: "<<funcion->ugo<<endl;
                        }else if(opActual==10){
                            funcion->pwd = tokenActual;
                            cout<<"El parametro pwd tiene el valor: "<<funcion->pwd<<endl;
                        }
                    }else{
                        //error no puede ser número
                        cout<<"Error, este parametro no puede ser numerico"<<endl;
                    }
                }else{
                    /*Reconoció un espacio o algo así*/
                    int prueba = comando[contador];
                    cout<<"Se reconoció caracter con codigo: "<<prueba<<endl;
                    contador++;
                }
            }else{
                //Ahora come caracteres hasta llegar al otro parametro
                while (comando[contador]!='-') {
                    contador++;
                }
            }

        }else{
            cout<<"Reconoció: "<<comando[contador]<<endl;
            contador++;
        }
    }
    /*************************TERMINA LECTURA************************/
    /*Una vez se lee el comando se ejecuta la acción*/
    switch (funcion->funcion){
                case 0:
                    admin->crearDisco(funcion);     //Done
                    break;
                case 1:
                    admin->eliminarDisco(funcion);     //Done
                    break;
                case 2:
                    admin->crearParticion(funcion);     //Done
                    break;
                case 3:
                    //montarDisco();
                    admin->montarDisco(funcion);     //Done
                    break;
                case 4:
                  admin->  desmontarDisco(funcion);     //Done
                    break;
                case 5:
                    admin->reportes(funcion);
                    break;
                case 6:
                    ejecutarComando(funcion);     //Done
                    break;
                case 7:
                    admin->formatear(funcion);     //Done
                    break;
                case 8:
                    admin->login(funcion);        //Mínimo
                    break;
                case 9:
                    admin->logout();       //Mínimo
                    break;
                case 10:
                    //admin->crearGrupo(funcion);   //Mínimo
                    break;
                case 11:
                    //admin->eliminarGrupo(funcion);
                    break;
                case 12:
                    //admin->crearUsr(funcion);     //Mínimo
                    break;
                case 13:
                    //admin->eliminarUsr(funcion);
                    break;
                case 14:
                    //admin->chmod(funcion);
                    break;
                case 15:
                    admin->crearArchivo(funcion); //Mínimo
                    break;
                case 16:
                    //admin->catArchivo(funcion);//Mínimo
                    break;
                case 17:
                    //admin->eliminarArchivo(funcion);
                    break;
                case 18:
                    //admin->editarArchivo(funcion);
                    break;
                case 19:
                    //admin->renombrarArchivo(funcion);
                    break;
                case 20:
                    admin->crearDirectorio(funcion);//Mínimo
                    break;
                case 21:
                    //admin->copiarDir(funcion);
                    break;
                case 22:
                    //admin->moverDir(funcion);
                    break;
                case 23:
                    //admin->encontrarDir(funcion);
                    break;
                case 24:
                  //  admin->cambiarDueno(funcion);
                    break;
                case 25:
                  //  admin->cambiarGrp(funcion);
                    break;
                case 26:
                    break;
                case 27:
                    admin->recuperar(funcion);    //Mínimo
                    break;
                case 28:
                    admin->perdida(funcion);      //Mínimo
                    break;
                default:
                    cout<<"Error, No se ingresó una opción válida"<<endl;
                    break;
            }
}



void Interprete::ejecutarComando(Funcion *funcion){
    string line;
    char comando[500];
    bool bandera= false;
    string entrada="";
        if(funcion->opciones[3]==1){
            ifstream fichero(funcion->path);
            if(fichero.is_open()){
                while(getline(fichero,line)){
                    std::copy(line.begin(), line.end(), comando);
                    comando[line.size()] = '\0';
                    if(comando[0]!='#')
                        this->ejecutar(comando);
                }
                fichero.close();
            }else{
                cout<<"Error al abrir ejecutable"<<endl;
            }
        }else {
            cout<<"Error al ejecutar script, no están todos los campos necesarios"<<endl;
        }
}

bool Interprete::esEspacio(char caracter){
    if(caracter==' '||caracter=='\t'||caracter==10||caracter=='\t')
            return true;
        return false;
}

bool Interprete::esLetra(char caracter){
    if((caracter>='a'&&caracter<='z')||(caracter>='A'&&caracter<='Z'))
        return true;
    return false;
}

bool Interprete::esNumero(char caracter){
    if((caracter>='0'&&caracter<='9')||caracter=='-')
        return true;
    return false;
}

bool Interprete::esId(char caracter){
    if(((caracter>='a'&&caracter<='z')||(caracter>='A'&&caracter<='Z'))||(caracter>='0'&&caracter<='9')||(caracter=='_'))
        return true;
    return false;
}

int Interprete::getParam(string token){
    /* 0 size   int
         * 1 fit    char[2]
         * 2 unit   char
         * 3 path   string
         * 4 type   char
         * 5 delete string
         * 6 name   string
         * 7 add    int
         * 8 id     string
         */
        if(token.compare("size")==0)
            return 0;
        if(token.compare("fit")==0)
            return 1;
        if(token.compare("unit")==0)
            return 2;
        if(token.compare("path")==0)
            return 3;
        if(token.compare("type")==0)
            return 4;
        if(token.compare("delete")==0)
            return 5;
        if(token.compare("name")==0)
            return 6;
        if(token.compare("add")==0)
            return 7;
        if(token.compare("id")==0)
            return 8;
        if(token.compare("usr")==0)
            return 9;
        if(token.compare("pwd")==0)
            return 10;
        if(token.compare("grp")==0)
            return 11;
        if(token.compare("ugo")==0)
            return 12;
        if(token.compare("r")==0)
            return 13;
        if(token.compare("p")==0)
            return 14;
        if(token.compare("cont")==0)
            return 15;
        if(token.compare("file")==0)
            return 16;
        if(token.compare("dest")==0)
            return 17;
        if(token.compare("fs")==0)
            return 18;
        if(token.compare("ruta")==0)
            return 19;

        return -1;
}

int Interprete::getOpcion(string token){
    if(token.compare("mkdisk")==0)
            return 0;
        if(token.compare("rmdisk")==0)
            return 1;
        if(token.compare("fdisk")==0)
            return 2;
        if(token.compare("mount")==0)
            return 3;
        if(token.compare("unmount")==0)
            return 4;
        if(token.compare("rep")==0)
            return 5;
        if(token.compare("exec")==0)
            return 6;
        if(token.compare("mkfs")==0)
            return 7;
        if(token.compare("login")==0)
            return 8;
        if(token.compare("logout")==0)
            return 9;
        if(token.compare("mkgrp")==0)
            return 10;
        if(token.compare("rmgrp")==0)
            return 11;
        if(token.compare("mkusr")==0)
            return 12;
        if(token.compare("rmusr")==0)
            return 13;
        if(token.compare("chmod")==0)
            return 14;
        if(token.compare("mkfile")==0)
            return 15;
        if(token.compare("cat")==0)
            return 16;
        if(token.compare("rm")==0)
            return 17;
        if(token.compare("edit")==0)
            return 18;
        if(token.compare("ren")==0)
            return 19;
        if(token.compare("mkdir")==0)
            return 20;
        if(token.compare("cp")==0)
            return 21;
        if(token.compare("mv")==0)
            return 22;
        if(token.compare("find")==0)
            return 23;
        if(token.compare("chown")==0)
            return 24;
        if(token.compare("chgrp")==0)
            return 25;
        if(token.compare("pause")==0)
            return 26;
        if(token.compare("recovery")==0)
            return 27;
        if(token.compare("loss")==0)
            return 28;
        return 29;
}
