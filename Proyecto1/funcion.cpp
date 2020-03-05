#include "funcion.h"

Funcion::Funcion()
{
    this->funcion=-1;
    for (int i = 0;i<21;i++) {
        opciones[i]=-1;
    }
    this->fs = -1;
    this->size=-1;
    this->ugo=-1;
    this->add=-1;
    this->fit='\0';
    this->unit='\0';
    this->type='\0';
    this->r=false;
    this->p=false;
    this->path="";
    this->eliminar="";
    this->nombre="";
    for (int i = 0;i<6;i++) {
        id[i]="-1";
    }
    this->usr="";
    this->pwd="";
    this->grp="";
    this->cont="";
    for (int i = 0;i<6;i++) {
        file[i]=-1;
    }
    this->dest="";
    this->fileName="";
    this->ruta="";
    this->rf = false;
}

void Funcion::mostrar(){
    cout<<"Funcion: "<<this->funcion<<endl;
    cout<<"Este comando tiene los siguientes parametros:"<<endl;
    for (int i = 0;i<20;i++) {

        if(this->opciones[i]==1&&i==0){
            cout<<"Size: "<<this->size<<endl;
        }else if(this->opciones[i]==1&&i==1){
            cout<<"fit: "<<this->fit<<endl;

        }else if(this->opciones[i]==1&&i==2){
            cout<<"unit: "<<this->unit<<endl;

        }else if(this->opciones[i]==1&&i==3){
            cout<<"path: "<<this->path<<endl;

        }else if(this->opciones[i]==1&&i==4){
            cout<<"type: "<<this->type<<endl;

        }else if(this->opciones[i]==1&&i==5){
            cout<<"delete: "<<this->eliminar<<endl;

        }else if(this->opciones[i]==1&&i==6){
            cout<<"name: "<<this->nombre<<endl;

        }else if(this->opciones[i]==1&&i==7){
            cout<<"add: "<<this->add<<endl;

        }else if(this->opciones[i]==1&&i==8){
            for (int j=0;j<6;j++) {
                if(id[j].compare("-1")!=0)
                    cout<<"id"<<j<<": "<<this->id[j]<<endl;
            }

        }else if(this->opciones[i]==1&&i==9){
            cout<<"usr: "<<this->usr<<endl;

        }else if(this->opciones[i]==1&&i==10){
            cout<<"pwd: "<<this->pwd<<endl;

        }else if(this->opciones[i]==1&&i==11){
            cout<<"grp: "<<this->grp<<endl;

        }else if(this->opciones[i]==1&&i==12){
            cout<<"ugo: "<<this->ugo<<endl;

        }else if(this->opciones[i]==1&&i==13){
            cout<<"-r: "<<this->r<<endl;

        }else if(this->opciones[i]==1&&i==14){
            cout<<"-p: "<<this->p<<endl;

        }else if(this->opciones[i]==1&&i==15){
            cout<<"cont: "<<this->cont<<endl;

        }else if(this->opciones[i]==1&&i==16){
            cout<<"file: "<<this->file<<endl;

        }else if(this->opciones[i]==1&&i==17){
            cout<<"dest: "<<this->dest<<endl;
        }else if(this->opciones[i]==1&&i==18){
            cout<<"RF: "<<this->rf<<endl;
        }else if(this->opciones[i]==1&&i==19){
            cout<<"ruta: "<<this->ruta<<endl;
        }else if(this->opciones[i]==1&&i==20)
            cout<<"Fs: "<<this->fs<<endl;

        else{
        }
    }
}

string Funcion::getRaidName(){
    string raidName="";
    int i = 0;
    while(this->fileName[i]!='.'){
        raidName+=this->fileName[i];
        i++;
    }
    return raidName+="_ra1.disk";
}

void Funcion::getName(){

}

string Funcion::getAbsPath(){
    int i = 0;
    string absPath="";
    string tokenActual="";
    while(this->path[i]!='.'){
        if(this->path[i]=='/'){
            absPath+=tokenActual+"/";
            tokenActual="";
        }else{
            tokenActual+=path[i];
        }
        i++;
    }
    return absPath;
}
