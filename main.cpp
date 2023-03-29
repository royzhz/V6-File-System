//
// Created by roy on 26/03/2023.
//

#include "SystemCall.h"
#include <iostream>
#include <vector>
#include <sstream>
using namespace std;

void split(string& cmd,vector<string>& code){
    stringstream s(cmd);
    while(!s.eof()){
        string tmp;
        s>>tmp;
        if(tmp.empty())
            break;

        code.push_back(tmp);
        s.clear();
    }
}

void test(SystemCall& systemCall){
    systemCall.creat("test.txt");
    systemCall.creat("test2.txt");

    systemCall.Open("test.txt");
    systemCall.Open("test2.txt");

    systemCall.Write("2","std","test1");
    systemCall.Seek("2","0","0");
    systemCall.Read("2","std","8");
    systemCall.Close("2");
    systemCall.Read("2","std","8");



    systemCall.Write("3","std","test2");
    systemCall.Seek("3","0","0");
    systemCall.Read("3","std","8");

    systemCall.MakeDir("test");
    systemCall.Ls();

    systemCall.Cd("test");

    systemCall.creat("test3.txt");
    systemCall.Open("test3.txt");

    systemCall.Write("5","std","test3");
    systemCall.Seek("5","0","0");
    systemCall.Read("5","std","8");
}


int main(){
    SystemCall systemCall;
    vector<string> code;
    //test(systemCall);

    cout<<"please login first"<<endl;
    while(true){
        string username,password;
        cout<<"username: ";
        getline(cin,username);
        cout<<"password: ";
        getline(cin,password);
        if(systemCall.Login(username,password))
            break;
        else
            cout<<"login failed,please try again"<<endl;
    }
    cout<<"login success"<<endl;
    while(true){
        code.clear();
        cout<<systemCall.get_now_dic()<<"> ";
        string cmd;
        getline(cin,cmd);
        split(cmd,code);

        if(code[0]=="create" && code.size()==2){
            systemCall.creat(code[1]);
        }
        else if(code[0]=="open"&& code.size()==2){
            systemCall.Open(code[1]);
        }
        else if(code[0]=="write"&& code.size()==4){
            systemCall.Write(code[1],code[2],code[3]);
        }
        else if(code[0]=="read"&& code.size()==4){
            systemCall.Read(code[1],code[2],code[3]);
        }
        else if(code[0]=="seek" && code.size()==4){
            systemCall.Seek(code[1],code[2],code[3]);
        }
        else if(code[0]=="cd"&& code.size()==2){
            systemCall.Cd(code[1]);
        }
        else if(code[0]=="mkdir"&& code.size()==2){
            systemCall.MakeDir(code[1]);
        }
        else if(code[0]=="close"&& code.size()==2){
            systemCall.Close(code[1]);
        }
        else if(code[0]=="ls"&& code.size()==1){
            systemCall.Ls();
        }
        else if(code[0]=="rm"&& code.size()==2){
            systemCall.Rm(code[1]);
        }
        else if(code[0]=="exit"&& code.size()==1){
            break;
        }
        else if(code[0]=="reset"&& code.size()==1){

            systemCall.Reset();

        }
        else{
            cout<<"cannot recognize commond"<<endl;
        }
    }
    return 0;
}
