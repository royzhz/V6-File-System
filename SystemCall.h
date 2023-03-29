//
// Created by roy on 27/03/2023.
//

#ifndef OSFILE_SYSTEMCALL_H
#define OSFILE_SYSTEMCALL_H

#include "Kernel.h"
#include "User.h"
#include "FileManager.h"
#include <cstring>
using namespace std;

class SystemCall {
public:

    void creat(string filename);
    void Open(string fileName);

    void Write(string sfd, string inFile, string size);
    void Read(string sfd, string outFile, string size);
    void Seek(string sfd, string offset, string origin);
    void Cd(string path);
    void MakeDir(string dirname);
    void Close(string sfd);
    void Ls();
    void Rm(string filename);
    string get_now_dic();
    void Reset();
    bool Login(string username,string password);
private:
    char* pointer=NULL;
    void cleanpointer();
    bool show_error(int show=1);
    bool checkpath(string path);
    int open(string fileName,int mode=1);
    char* read(int sfd, int size,int mode=1);
    int write(int sfd, string buffer, int size,int mode=1);
};


#endif //OSFILE_SYSTEMCALL_H
