//
// Created by roy on 26/03/2023.
//

#include "User.h"
#include "Kernel.h"
#include "FileManager.h"

char *User::Pwd() {
    return u_curdir;
}

char *User::getName() {
    return name;
}

User::User() {
    strcpy(name,"roy");
    u_curdir[0]='/';//系统调用参数(一般用于Pathname)的指针
    u_dirp=u_curdir;
    u_error = U_NOERROR;  //存放错误码
    FileManager &fileManager = Kernel::Instance().GetFileManager();

    u_cdir = fileManager.rootDirInode;//指向当前目录的Inode指针
    u_pdir= NULL;                     //指向父目录的Inode指针
    memset(u_arg, 0, sizeof(u_arg));      //指向核心栈现场保护区中EAX寄存器
}

void User::setname(const char * newname) {
    strcpy(name,newname);
}
