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
        /* ���̴��ļ������������ҵ�������򷵻�֮ */
        if(this->ProcessOpenFileTable[i] == NULL)
        {
            /* ���ú���ջ�ֳ��������е�EAX�Ĵ�����ֵ����ϵͳ���÷���ֵ */
            u.u_ar0[User::EAX] = i;
            return i;
        }
    }

    u.u_ar0[User::EAX] = -1;   /* Open1����Ҫһ����־�������ļ��ṹ����ʧ��ʱ�����Ի���ϵͳ��Դ*/
    u.u_error = User::U_EMFILE;
    return -1;
}

int OpenFiles::Clone(int fd) {
    return 0;
}

File *OpenFiles::GetF(int fd) {
    File* pFile;
    User& u = Kernel::Instance().GetUser();

    /* ������ļ���������ֵ�����˷�Χ */
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

    return pFile;	/* ��ʹpFile==NULLҲ���������ɵ���GetF�ĺ������жϷ���ֵ */
}

void OpenFiles::SetF(int fd, File *pFile) {
    if(fd < 0 || fd >= OpenFiles::NOFILES)
    {
        return;
    }
    /* ���̴��ļ�������ָ��ϵͳ���ļ�������Ӧ��File�ṹ */
    this->ProcessOpenFileTable[fd] = pFile;
}


IOParameter::IOParameter() {
    this->m_Base = nullptr;
    this->m_Count = 0;
    this->m_Offset = 0;
}

IOParameter::~IOParameter() {

}
