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
    u_curdir[0]='/';//ϵͳ���ò���(һ������Pathname)��ָ��
    u_dirp=u_curdir;
    u_error = U_NOERROR;  //��Ŵ�����
    FileManager &fileManager = Kernel::Instance().GetFileManager();

    u_cdir = fileManager.rootDirInode;//ָ��ǰĿ¼��Inodeָ��
    u_pdir= NULL;                     //ָ��Ŀ¼��Inodeָ��
    memset(u_arg, 0, sizeof(u_arg));      //ָ�����ջ�ֳ���������EAX�Ĵ���
}

void User::setname(const char * newname) {
    strcpy(name,newname);
}
