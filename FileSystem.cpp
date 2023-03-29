//
// Created by roy on 26/03/2023.
//

#include "FileSystem.h"
#include "User.h"
#include "Kernel.h"
#include "DiskManager.h"
#include "OpenFileManager.h"

#include <ctime>

INode* FileSystem::IAlloc()
{
    SuperBlock* sb;
    Buf* pBuf;
    INode* pNode;
    User& u = Kernel::Instance().GetUser();
    int ino;	/* ���䵽�Ŀ������Inode��� */

    /* ��ȡ��Ӧ�豸��SuperBlock�ڴ渱�� */
    sb = &Kernel::Instance().GetSuperBlock();

    /*
     * SuperBlockֱ�ӹ���Ŀ���Inode�������ѿգ�
     * ���뵽��������������Inode���ȶ�inode�б�������
     * ��Ϊ�����³����л���ж��̲������ܻᵼ�½����л���
     * ���������п��ܷ��ʸ����������ᵼ�²�һ���ԡ�
     */
    if(sb->s_ninode <= 0)
    {
        /* ����Inode���������� */
        //sb->s_ilock++;

        /* ���Inode��Ŵ�0��ʼ���ⲻͬ��Unix V6�����Inode��1��ʼ��� */
        ino = -1;

        /* ���ζ������Inode���еĴ��̿飬�������п������Inode���������Inode������ */
        for(int i = 0; i < sb->s_isize; i++)
        {
            pBuf = this->m_BufferManager->Bread(FileSystem::INODE_START_SECTOR + i);

            /* ��ȡ��������ַ */
            int* p = (int *)pBuf->b_addr;

            /* ���û�������ÿ�����Inode��i_mode != 0����ʾ�Ѿ���ռ�� */
            for(int j = 0; j < FileSystem::INODE_NUMBER_PER_SECTOR; j++)
            {
                ino++;

                int mode = *( p + j * sizeof(DiskINode)/sizeof(int) );

                /* �����Inode�ѱ�ռ�ã����ܼ������Inode������ */
                if(mode != 0)
                {
                    continue;
                }

                /*
                 * ������inode��i_mode==0����ʱ������ȷ��
                 * ��inode�ǿ��еģ���Ϊ�п������ڴ�inodeû��д��
                 * ������,����Ҫ���������ڴ�inode���Ƿ�����Ӧ����
                 */
                if( g_InodeTable.IsLoaded(ino) == -1 )
                {
                    /* �����Inodeû�ж�Ӧ���ڴ濽��������������Inode������ */
                    sb->s_inode[sb->s_ninode++] = ino;

                    /* ��������������Ѿ�װ�����򲻼������� */
                    if(sb->s_ninode >= 100)
                    {
                        break;
                    }
                }
            }

            /* �����Ѷ��굱ǰ���̿飬�ͷ���Ӧ�Ļ��� */
            this->m_BufferManager->Brelse(pBuf);

            /* ��������������Ѿ�װ�����򲻼������� */
            if(sb->s_ninode >= 100)
            {
                break;
            }
        }
//        /* ����Կ������Inode�����������������Ϊ�ȴ�����˯�ߵĽ��� */
//        sb->s_ilock = 0;
//        Kernel::Instance().GetProcessManager().WakeUpAll((unsigned long)&sb->s_ilock);

        /* ����ڴ�����û���������κο������Inode������NULL */
        if(sb->s_ninode <= 0)
        {
            //Diagnose::Write("No Space On %d !\n", dev);
            u.u_error = User::U_ENOSPC;
            return NULL;
        }
    }

    /*
     * ���沿���Ѿ���֤������ϵͳ��û�п������Inode��
     * �������Inode�������бض����¼�������Inode�ı�š�
     */
    while(true)
    {
        /* ��������ջ������ȡ�������Inode��� */
        ino = sb->s_inode[--sb->s_ninode];

        /* ������Inode�����ڴ� */
        pNode = g_InodeTable.IGet(ino);
        /* δ�ܷ��䵽�ڴ�inode */
        if(NULL == pNode)
        {
            return NULL;
        }

        /* �����Inode����,���Inode�е����� */
        if(0 == pNode->i_mode)
        {
            pNode->Clean();
            /* ����SuperBlock���޸ı�־ */
            sb->s_fmod = 1;
            return pNode;
        }
        else	/* �����Inode�ѱ�ռ�� */
        {
            g_InodeTable.IPut(pNode);
            continue;	/* whileѭ�� */
        }
    }
    return NULL;	/* GCC likes it! */
}

Buf *FileSystem::Alloc() {
    int blkno;
    SuperBlock *sb;
    sb = &Kernel::Instance().GetSuperBlock();
    Buf* pCache;
    User& u = Kernel::Instance().GetUser();
    //��������ջ������ȡ���д��̿���
    blkno = sb->s_free[--sb->s_nfree];

    //����ȡ���̿���Ϊ�㣬���ʾ�ѷ��価���еĿ��д��̿�
    if (blkno <= 0) {
        sb->s_nfree = 0;
        u.u_error = User::U_ENOSPC;
        return NULL;
    }

    //ջ�ѿգ��·��䵽���д��̿��м�¼����һ����д��̿�ı��
    //����һ����д��̿�ı�Ŷ���sb�Ŀ��д��̿�������s_free[100]��
    if (sb->s_nfree <= 0) {
        pCache = this->m_BufferManager->Bread(blkno);
        int* p = (int*)pCache->b_addr;
        sb->s_nfree = *p++;
        memcpy(sb->s_free, p, sizeof(sb->s_free));
        this->m_BufferManager->Brelse(pCache);
    }
    pCache = this->m_BufferManager->GetBlk(blkno);
    if (pCache)
        this->m_BufferManager->Bclear(pCache);
    sb->s_fmod = 1;
    return pCache;
}

void FileSystem::IFree(int number) {
    SuperBlock *sb;
    sb = &Kernel::Instance().GetSuperBlock();
    if (sb->s_ninode >= SuperBlock::MAX_NUMBER_INODE)
        return;
    sb->s_inode[sb->s_ninode++] = number;
    sb->s_fmod = 1;
}

void FileSystem::Free(int blkno) {
    Buf* pCache;
    SuperBlock *sb;
    sb = &Kernel::Instance().GetSuperBlock();
    if (sb->s_nfree >= SuperBlock::MAX_NUMBER_FREE) {
        pCache = this->m_BufferManager->GetBlk(blkno);
        int* p = (int*)pCache->b_addr;
        *p++ = sb->s_nfree;
        memcpy(p, sb->s_free, sizeof(int) * SuperBlock::MAX_NUMBER_FREE);
        sb->s_nfree = 0;
        this->m_BufferManager->Bwrite(pCache);
    }

    sb->s_free[sb->s_nfree++] = blkno;
    sb->s_fmod = 1;
}

void FileSystem::Update() {
    Buf* pCache;
    SuperBlock *sb;
    sb = &Kernel::Instance().GetSuperBlock();
    sb->s_fmod = 0;
    sb->s_time = (int)time(NULL);
    for (int j = 0; j < 2; j++) {
        int* p = (int*)sb + j * 128;
        pCache = this->m_BufferManager->GetBlk(FileSystem::SUPER_BLOCK_START_SECTOR + j);
        memcpy(pCache->b_addr, p, BLOCK_SIZE);
        this->m_BufferManager->Bwrite(pCache);
    }
    g_InodeTable.UpdateINodeTable();
    this->m_BufferManager->Bflush();

}
//ֱ�Ӵ��̶��뼴��
void FileSystem::LoadSuperBlock() {
    SuperBlock *sb=&Kernel::Instance().GetSuperBlock();
    DiskDriver* diskDriver = &Kernel::Instance().GetDiskDriver();
    fseek(diskDriver->diskPointer, 0, 0);
    diskDriver->read((uint8_t*)(sb), sizeof(SuperBlock), 0);
}

//��ʽ�������ļ�ϵͳ
void FileSystem::FormatDevice() {
    //��ʽ��������
    FormatSuperBlock();
    DiskDriver* diskDriver = &Kernel::Instance().GetDiskDriver();
    //д��(��ռλ)
    diskDriver->Construct();
    SuperBlock *sb=&Kernel::Instance().GetSuperBlock();
    diskDriver->write((uint8_t*)(sb), sizeof(SuperBlock), 0);
    //д���Ŀ¼
    DiskINode emptyDINode, rootDINode;
    rootDINode.i_mode |= INode::IALLOC | INode::IFDIR;
    diskDriver->write((uint8_t*)&rootDINode, sizeof(rootDINode));
    //д����������Inode
    for (int i = 1; i < FileSystem::INODE_NUMBER_ALL; ++i) {
        if (sb->s_ninode < SuperBlock::MAX_NUMBER_INODE)
            sb->s_inode[sb->s_ninode++] = i;
        diskDriver->write((uint8_t*)&emptyDINode, sizeof(emptyDINode));
    }
    //д����д��̿�
    char freeBlock[BLOCK_SIZE], freeBlock1[BLOCK_SIZE];
    memset(freeBlock, 0, BLOCK_SIZE);
    memset(freeBlock1, 0, BLOCK_SIZE);

    for (int i = 0; i < FileSystem::DATA_SECTOR_NUMBER; ++i) {
        if (sb->s_nfree >= SuperBlock::MAX_NUMBER_FREE) {
            //��¼���д��̿������
            memcpy(freeBlock1, &sb->s_nfree, sizeof(int) + sizeof(sb->s_free));
            diskDriver->write((uint8_t*)&freeBlock1, BLOCK_SIZE);
            sb->s_nfree = 0;
        }
        else
            diskDriver->write((uint8_t*)freeBlock, BLOCK_SIZE);
        sb->s_free[sb->s_nfree++] = i + FileSystem::DATA_START_SECTOR;
    }

    //ȷ��ʱ���д�볬����
    time((time_t*)&sb->s_time);
    diskDriver->write((uint8_t*)(sb), sizeof(SuperBlock), 0);
}

void FileSystem::FormatSuperBlock() {
    SuperBlock *sb=&Kernel::Instance().GetSuperBlock();
    sb->s_isize = FileSystem::INODE_SECTOR_NUMBER;
    sb->s_fsize = FileSystem::DISK_SECTOR_NUMBER;
    sb->s_nfree = 0;
    sb->s_free[0] = -1;
    sb->s_ninode = 0;
    sb->s_fmod = 0;
    time((time_t*)&sb->s_time);
}

FileSystem::FileSystem() {
    updlock=0;
    this->m_BufferManager = &Kernel::Instance().GetBufferManager();
    DiskDriver* diskDriver = &Kernel::Instance().GetDiskDriver();
    if (!diskDriver->Exists())
        FormatDevice();
    else
        LoadSuperBlock();
}

FileSystem::~FileSystem() {
    Update();
}
