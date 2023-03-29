//
// Created by roy on 27/03/2023.
//

#include "File.h"
#include "Kernel.h"
#include "User.h"

File::File() {
    f_count = 0;
    f_inode = NULL;
    f_offset = 0;
}
File::~File() { }

void File::Reset()
{
    f_count = 0;
    f_inode = NULL;
    f_offset = 0;
}

OpenFiles::OpenFiles() {
    memset(ProcessOpenFileTable, 0, sizeof(ProcessOpenFileTable));
}

OpenFiles::~OpenFiles() {

}

int OpenFiles::AllocFreeSlot() {
    int i;
    User& u = Kernel::Instance().GetUser();

    for(i = 0; i < OpenFiles::NOFILES; i++)
    {
        /* 进程打开文件描述符表中找到空闲项，则返回之 */
        if(this->ProcessOpenFileTable[i] == NULL)
        {
            /* 设置核心栈现场保护区中的EAX寄存器的值，即系统调用返回值 */
            u.u_ar0[User::EAX] = i;
            return i;
        }
    }

    u.u_ar0[User::EAX] = -1;   /* Open1，需要一个标志。当打开文件结构创建失败时，可以回收系统资源*/
    u.u_error = User::U_EMFILE;
    return -1;
}

int OpenFiles::Clone(int fd) {
    return 0;
}

File *OpenFiles::GetF(int fd) {
    File* pFile;
    User& u = Kernel::Instance().GetUser();

    /* 如果打开文件描述符的值超出了范围 */
    if(fd < 0 || fd >= OpenFiles::NOFILES)
    {
        u.u_error = User::U_EBADF;
        return NULL;
    }

    pFile = this->ProcessOpenFileTable[fd];
    if(pFile == NULL)
    {
        u.u_error = User::U_EBADF;
    }

    return pFile;	/* 即使pFile==NULL也返回它，由调用GetF的函数来判断返回值 */
}

void OpenFiles::SetF(int fd, File *pFile) {
    if(fd < 0 || fd >= OpenFiles::NOFILES)
    {
        return;
    }
    /* 进程打开文件描述符指向系统打开文件表中相应的File结构 */
    this->ProcessOpenFileTable[fd] = pFile;
}


IOParameter::IOParameter() {
    this->m_Base = nullptr;
    this->m_Count = 0;
    this->m_Offset = 0;
}

IOParameter::~IOParameter() {

}
