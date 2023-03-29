//
// Created by roy on 27/03/2023.
//

#include "SystemCall.h"
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;
//检查并将path写入u.u_dbuf

const string passlocation="/bin/passwd.ps";

bool SystemCall::checkpath(string path) {
    User &u=Kernel::Instance().GetUser();
    pointer=new char[path.length()+1];
    u.u_dirp=pointer;
    strcpy(u.u_dirp,path.c_str());
    return true;
}

void SystemCall::creat(string filename){
    if (!checkpath(filename))
        return;
    User &u=Kernel::Instance().GetUser();
    FileManager &fileManager = Kernel::Instance().GetFileManager();
    u.u_arg[1] = (INode::IREAD | INode::IWRITE);//存放当前系统调用参数
    fileManager.Creat();
    cleanpointer();
    if (show_error()){
        return;
    }

    cout<<"success to create "<<filename<<endl;
}

void SystemCall::Open(string filename) {
    int fd = open(filename);
    if(fd==-1)
        return;
    cout << "open file success, the handle of word is : " << fd << endl;
}



void SystemCall::Read(string sfd, string outFile, string size)
{
    char* buffer = read(stoi(sfd),stoi(size));
    if(buffer==NULL)
        return;
    int length=strlen(buffer);
    if(buffer==NULL)
        return;
    //输出
    cout << "success to read " << length << " Bytes" << endl;
    if (outFile == "std") {
        for (int i = 0; i < length; ++i)
            cout << (char)buffer[i];
        cout << endl;
        delete[] buffer;
        return;
    }
    else {
        fstream fout(outFile, ios::out | ios::binary);
        if (!fout) {
            cout << "fall to open" << outFile << endl;
            return;
        }
        fout.write(buffer, length);
        fout.close();
        delete[] buffer;
        return;
    }
}

void SystemCall::Write(string sfd, string inFile, string size)
{
    string buffer;
    int fd = stoi(sfd), usize = 0;
    //char *buffer;
    if(inFile=="std")
    {
        buffer=size;
        usize=size.length();
    }
    else {

        if (size.empty() || (usize = stoi(size)) < 0) {
            cout << "参数必须大于等于零 ! \n";
            return;
        }

        fstream fin(inFile, ios::in | ios::binary);
        if (!fin.is_open()) {
            cout << "fall to open " << inFile << endl;
            return;
        }

        buffer.reserve(usize);
        fin.read(&buffer[0], usize);
        buffer.resize(fin.gcount());
        fin.close();
    }
    int num=write(fd, buffer, usize);
    if(num==-1)
        return;
    cout << "success to write " << num << " Bytes" << endl;

}

string SystemCall::get_now_dic() {
    User &u=Kernel::Instance().GetUser();
    return string(u.name)+"@"+u.u_curdir;
}

bool SystemCall::show_error(int show) {
    User &u=Kernel::Instance().GetUser();
    if (u.u_error != User::U_NOERROR&&show) {
        switch (u.u_error) {
            case User::U_ENOENT:
                cout << "cannot find file" << endl;
                break;
            case User::U_EBADF:
                cout << "cant find file handle" << endl;
                break;
            case User::U_EACCES:
                cout << "authoity is not enough" << endl;
                break;
            case User::U_ENOTDIR:
                cout << "folder is not exist" << endl;
                break;
            case User::U_ENFILE:
                cout << "文件表溢出" << endl;
                break;
            case User::U_EMFILE:
                cout << "open too many files" << endl;
                break;
            case User::U_EFBIG:
                cout << "too big" << endl;
                break;
            case User::U_ENOSPC:
                cout << "disk is full" << endl;
                break;
            default:
                break;
        }

        u.u_error = User::U_NOERROR;
        return true;
    }
    return false;
}

void SystemCall::cleanpointer() {
    if(pointer==NULL)
        return;
    delete[] pointer;
    pointer=NULL;
}

void SystemCall::Seek(string sfd, string offset, string origin) {
    User &u=Kernel::Instance().GetUser();
    FileManager &fileManager = Kernel::Instance().GetFileManager();
    u.u_arg[0] = stoi(sfd);
    u.u_arg[1] = stoi(offset);
    u.u_arg[2] = stoi(origin);
    fileManager.Seek();
    show_error();
}

void SystemCall::Cd(string path) {
    if (!checkpath(path))
        return;
    User &u=Kernel::Instance().GetUser();
    u.u_arg[0]= (int)(u.u_dirp);
    FileManager &fileManager = Kernel::Instance().GetFileManager();
    fileManager.ChDir();
    show_error();
}

void SystemCall::MakeDir(string dirname) {
    if (!checkpath(dirname))
        return;
    User &u=Kernel::Instance().GetUser();
    FileManager &fileManager = Kernel::Instance().GetFileManager();
    u.u_arg[1] = INode::IFDIR;//存放当前系统调用参数 文件类型：目录文件
    fileManager.Creat();
    show_error();
}

void SystemCall::Close(string sfd)
{
    User &u=Kernel::Instance().GetUser();
    FileManager &fileManager = Kernel::Instance().GetFileManager();
    u.u_arg[0] = stoi(sfd);//存放当前系统调用参数
    fileManager.Close();
    show_error();
}

void SystemCall::Rm(string filename) {
    if (!checkpath(filename))
        return;
    User &u=Kernel::Instance().GetUser();
    FileManager &fileManager = Kernel::Instance().GetFileManager();
    fileManager.UnLink();
    show_error();
}

void SystemCall::Ls() {
    User &u=Kernel::Instance().GetUser();
    FileManager &fileManager = Kernel::Instance().GetFileManager();

    fileManager.Ls();
    char* p =(char*) u.u_ar0[User::EAX];
    if(p!=NULL) {
        cout << p << endl;
        delete []p;
    }
    else
        cout<<endl;
    show_error();
}

void SystemCall::Reset() {
   Kernel::Instance().Reset();

    MakeDir("bin");
    MakeDir("etc");
    MakeDir("home");
    MakeDir("dev");

    creat(passlocation);
    int handle=open(passlocation,0);
    string pass="roy mypass";
    write(handle,pass,pass.length(),0);
    Close(to_string(handle));

    Cd("home");
    MakeDir("texts");
    MakeDir("reports");
    MakeDir("photos");
    Cd("..");

}

bool SystemCall::Login(string username, string password) {
    int handle=open(passlocation,0);
    string buffer=read(handle,100,0);
    string all;

    while(!buffer.empty()){
        all+=buffer;
        buffer=read(handle,100,0);
    }
    stringstream ss(all);
    string tname,tpass;
    ss>>tname>>tpass;
    if(tname==username&& tpass==password){
        User &u =Kernel::Instance().GetUser();
        u.setname(tname.c_str());
        return true;
    }
    return false;

}

int SystemCall::open(string filename,int mode) {
    User &u=Kernel::Instance().GetUser();
    FileManager &fileManager = Kernel::Instance().GetFileManager();

    if (!checkpath(filename))
        return -1;

    u.u_arg[1] = (File::FREAD | File::FWRITE);//存放当前系统调用参数
    fileManager.Open();
    cleanpointer();

    if (show_error(mode))
        return -1;

    return u.u_ar0[User::EAX];
}

char *SystemCall::read(int sfd, int size, int mode) {
    int fd = sfd;
    int usize = size;
    char* buffer = new char[usize+1];
    memset(buffer,0,usize+1);

    User &u=Kernel::Instance().GetUser();
    FileManager &fileManager = Kernel::Instance().GetFileManager();

    u.u_arg[0] = fd;
    u.u_arg[1] = (int)buffer;
    u.u_arg[2] = usize;
    fileManager.Read();
    cleanpointer();
    if (show_error(mode))
        return NULL;
    return buffer;
}

int SystemCall::write(int fd, string buffer, int size, int mode) {
    User &u=Kernel::Instance().GetUser();
    FileManager &fileManager = Kernel::Instance().GetFileManager();
    u.u_arg[0] = fd;
    u.u_arg[1] = (int)buffer.c_str();
    u.u_arg[2] = size;
    fileManager.Write();
    cleanpointer();
    if (show_error(mode))
        return -1;
    return u.u_ar0[User::EAX];
}

