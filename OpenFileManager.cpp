//
// Created by roy on 26/03/2023.
//

#include "OpenFileManager.h"
#include "Kernel.h"
#include <ctime>

INodeTable g_InodeTable;
OpenFileTable g_OpenFileTable;

INodeTable::INodeTable() {
    fileSystem=& Kernel::Instance().GetFileSystem();
}

INodeTable::~INodeTable() {

}

INode *INodeTable::IGet(int inumber) {
    INode* pINode;
    User& u=Kernel::Instance().GetUser();
    BufferManager& myCacheManager=Kernel::Instance().GetBufferManager();
    int index = IsLoaded(inumber);
    if (index >= 0) {
        pINode = m_INodeTable + index;
        ++pINode->i_count;
        return pINode;
    }

    pINode = GetFreeINode();
    if (NULL == pINode) {
        //cout << "�ڴ� INode �����!" << endl;
        u.u_error = User::U_ENFILE;
        return NULL;
    }

    pINode->i_number = inumber;
    pINode->i_count++;
    Buf* pCache = myCacheManager.Bread(FileSystem::INODE_START_SECTOR + inumber / FileSystem::INODE_NUMBER_PER_SECTOR);
    pINode->ICopy(pCache, inumber);
    myCacheManager.Brelse(pCache);
    return pINode;
}

void INodeTable::IPut(INode *pNode) {
    //��ǰ����Ϊ���ø��ڴ�INode��Ψһ���̣���׼���ͷŸ��ڴ�INode
    if (pNode->i_count == 1) {
        //���ļ��Ѿ�û��Ŀ¼·��ָ����
        if (pNode->i_nlink <= 0) {
            //�ͷŸ��ļ�ռ�ݵ������̿�
            pNode->ITrunc();
            pNode->i_mode = 0;
            //�ͷŶ�Ӧ�����INode
            this->fileSystem->IFree(pNode->i_number);
        }
        //�������INode��Ϣ
        pNode->IUpdate((int)time(NULL));
        //����ڴ�INode�����б�־λ
        pNode->i_flag = 0;
        //�����ڴ�inode���еı�־֮һ����һ����i_count == 0
        pNode->i_number = -1;
    }

    pNode->i_count--;
}

void INodeTable::UpdateINodeTable() {
    for (int i = 0; i < INodeTable::NINODE; ++i)
        if (this->m_INodeTable[i].i_count)
            this->m_INodeTable[i].IUpdate((int)time(NULL));
}

int INodeTable::IsLoaded(int inumber) {
    for (int i = 0; i < NINODE; ++i)
        if (m_INodeTable[i].i_number == inumber && m_INodeTable[i].i_count)
            return i;
    return -1;
}

INode *INodeTable::GetFreeINode() {
    for (int i = 0; i < INodeTable::NINODE; i++)
        if (this->m_INodeTable[i].i_count == 0)
            return m_INodeTable + i;
    return NULL;
}

void INodeTable::Reset() {
    INode emptyINode;
    for (int i = 0; i < INodeTable::NINODE; ++i)
        m_INodeTable[i].Reset();
}

OpenFileTable::OpenFileTable() {

}

OpenFileTable::~OpenFileTable() {

}

File *OpenFileTable::FAlloc() {
    User& u=Kernel::Instance().GetUser();
    int fd = u.u_ofiles.AllocFreeSlot();
    if (fd < 0)
        return NULL;
    for (int i = 0; i < OpenFileTable::NFILE; ++i) {
        //count == 0��ʾ�������
        if (this->m_File[i].f_count == 0) {
            u.u_ofiles.SetF(fd, &this->m_File[i]);
            this->m_File[i].f_count++;
            this->m_File[i].f_offset = 0;
            return (&this->m_File[i]);
        }
    }
    u.u_error = User::U_ENFILE;
    return NULL;
}

void OpenFileTable::CloseF(File *pFile) {
    pFile->f_count--;
    if (pFile->f_count <= 0)
        g_InodeTable.IPut(pFile->f_inode);
}

void OpenFileTable::Reset() {
    for(int i = 0; i < OpenFileTable::NFILE; ++i)
        m_File[i].Reset();
}
