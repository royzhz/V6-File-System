//
// Created by roy on 26/03/2023.
//
#include "INode.h"
#include "Kernel.h"

int INode::rablock = 0;

void INode::IUpdate(int time) {
    Buf* pBuf;
    DiskINode dINode;
    FileSystem& filesys = Kernel::Instance().GetFileSystem();
    BufferManager& bufMgr = Kernel::Instance().GetBufferManager();
    //��IUPD��IACC��־֮һ�����ã�����Ҫ������ӦDiskINode
    //Ŀ¼����������������;����Ŀ¼�ļ���IACC��IUPD��־
    if (this->i_flag & (INode::IUPD | INode::IACC)) {
        pBuf = bufMgr.Bread(FileSystem::INODE_START_SECTOR + this->i_number / FileSystem::INODE_NUMBER_PER_SECTOR);
        dINode.i_mode = this->i_mode;
        dINode.i_nlink = this->i_nlink;
        dINode.i_size0 = this->i_size0;
        memcpy(dINode.i_addr, i_addr, sizeof(dINode.i_addr));
        if (this->i_flag & INode::IACC)
            dINode.i_atime = time;
        if (this->i_flag & INode::IUPD)
            dINode.i_mtime = time;

        unsigned char* p = pBuf->b_addr + (this->i_number % FileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskINode);
        DiskINode* pNode = &dINode;
        memcpy(p, pNode, sizeof(DiskINode));
        bufMgr.Bwrite(pBuf);
    }
}

//���ļ����߼����ת���ɶ�Ӧ�������̿��
int INode::Bmap(int lbn)
{
    Buf* pFirstBuf;
    Buf* pSecondBuf;
    int phyBlkno;	/* ת����������̿�� */
    int* iTable;	/* ���ڷ��������̿���һ�μ�ӡ����μ�������� */
    int index;
    User& u = Kernel::Instance().GetUser();
    BufferManager& bufMgr = Kernel::Instance().GetBufferManager();
    FileSystem& fileSys = Kernel::Instance().GetFileSystem();

    /* 
     * Unix V6++���ļ������ṹ��(С�͡����ͺ;����ļ�)
     * (1) i_addr[0] - i_addr[5]Ϊֱ���������ļ����ȷ�Χ��0 - 6���̿飻
     * 
     * (2) i_addr[6] - i_addr[7]���һ�μ�����������ڴ��̿�ţ�ÿ���̿�
     * �ϴ��128���ļ������̿�ţ������ļ����ȷ�Χ��7 - (128 * 2 + 6)���̿飻
     *
     * (3) i_addr[8] - i_addr[9]��Ŷ��μ�����������ڴ��̿�ţ�ÿ�����μ��
     * �������¼128��һ�μ�����������ڴ��̿�ţ������ļ����ȷ�Χ��
     * (128 * 2 + 6 ) < size <= (128 * 128 * 2 + 128 * 2 + 6)
     */

    if(lbn >= INode::HUGE_FILE_BLOCK)
    {
        u.u_error = User::U_EFBIG;
        return 0;
    }

    if(lbn < 6)		/* �����С���ļ����ӻ���������i_addr[0-5]�л�������̿�ż��� */
    {
        phyBlkno = this->i_addr[lbn];

        /* 
         * ������߼���Ż�û����Ӧ�������̿����֮��Ӧ�������һ������顣
         * ��ͨ�������ڶ��ļ���д�룬��д��λ�ó����ļ���С�����Ե�ǰ
         * �ļ���������д�룬����Ҫ�������Ĵ��̿飬��Ϊ֮�����߼����
         * �������̿��֮���ӳ�䡣
         */
        if( phyBlkno == 0 && (pFirstBuf = fileSys.Alloc()) != NULL )
        {
            /* 
             * ��Ϊ����ܿ������ϻ�Ҫ�õ��˴��·�������ݿ飬���Բ��������������
             * �����ϣ����ǽ�������Ϊ�ӳ�д��ʽ���������Լ���ϵͳ��I/O������
             */
            bufMgr.Bdwrite(pFirstBuf);
            phyBlkno = pFirstBuf->b_blkno;
            /* ���߼����lbnӳ�䵽�����̿��phyBlkno */
            this->i_addr[lbn] = phyBlkno;
            this->i_flag |= INode::IUPD;
        }
        /* �ҵ�Ԥ�����Ӧ�������̿�� */
        INode::rablock = 0;
        if(lbn <= 4)
        {
            /* 
             * i_addr[0] - i_addr[5]Ϊֱ�����������Ԥ�����Ӧ�����ſ��Դ�
             * ֱ���������л�ã����¼��INode::rablock�С������Ҫ�����I/O����
             * �����������飬���Եò�ֵ̫���ˡ�Ư����
             */
            INode::rablock = this->i_addr[lbn + 1];
        }

        return phyBlkno;
    }
    else	/* lbn >= 6 ���͡������ļ� */
    {
        /* �����߼����lbn��Ӧi_addr[]�е����� */

        if(lbn < INode::LARGE_FILE_BLOCK)	/* �����ļ�: ���Ƚ���7 - (128 * 2 + 6)���̿�֮�� */
        {
            index = (lbn - INode::SMALL_FILE_BLOCK) / INode::ADDRESS_PER_INDEX_BLOCK + 6;
        }
        else	/* �����ļ�: ���Ƚ���263 - (128 * 128 * 2 + 128 * 2 + 6)���̿�֮�� */
        {
            index = (lbn - INode::LARGE_FILE_BLOCK) / (INode::ADDRESS_PER_INDEX_BLOCK * INode::ADDRESS_PER_INDEX_BLOCK) + 8;
        }

        phyBlkno = this->i_addr[index];
        /* ������Ϊ�㣬���ʾ��������Ӧ�ļ��������� */
        if( 0 == phyBlkno )
        {
            this->i_flag |= INode::IUPD;
            /* ����һ�����̿��ż�������� */
            if( (pFirstBuf = fileSys.Alloc()) == NULL )
            {
                return 0;	/* ����ʧ�� */
            }
            /* i_addr[index]�м�¼���������������̿�� */
            this->i_addr[index] = pFirstBuf->b_blkno;
        }
        else
        {
            /* �����洢�����������ַ��� */
            pFirstBuf = bufMgr.Bread(phyBlkno);
        }
        /* ��ȡ��������ַ */
        iTable = (int *)pFirstBuf->b_addr;

        if(index >= 8)	/* ASSERT: 8 <= index <= 9 */
        {
            /* 
             * ���ھ����ļ��������pFirstBuf���Ƕ��μ��������
             * ��������߼���ţ����ɶ��μ���������ҵ�һ�μ��������
             */
            index = ( (lbn - INode::LARGE_FILE_BLOCK) / INode::ADDRESS_PER_INDEX_BLOCK ) % INode::ADDRESS_PER_INDEX_BLOCK;

            /* iTableָ�򻺴��еĶ��μ������������Ϊ�㣬������һ�μ�������� */
            phyBlkno = iTable[index];
            if( 0 == phyBlkno )
            {
                if( (pSecondBuf = fileSys.Alloc()) == NULL)
                {
                    /* ����һ�μ����������̿�ʧ�ܣ��ͷŻ����еĶ��μ��������Ȼ�󷵻� */
                    bufMgr.Brelse(pFirstBuf);
                    return 0;
                }
                /* ���·����һ�μ����������̿�ţ�������μ����������Ӧ�� */
                iTable[index] = pSecondBuf->b_blkno;
                /* �����ĺ�Ķ��μ���������ӳ�д��ʽ��������� */
                bufMgr.Bdwrite(pFirstBuf);
            }
            else
            {
                /* �ͷŶ��μ��������ռ�õĻ��棬������һ�μ�������� */
                bufMgr.Brelse(pFirstBuf);
                pSecondBuf = bufMgr.Bread(phyBlkno);
            }

            pFirstBuf = pSecondBuf;
            /* ��iTableָ��һ�μ�������� */
            iTable = (int *)pSecondBuf->b_addr;
        }

        /* �����߼����lbn����λ��һ�μ���������еı������index */

        if( lbn < INode::LARGE_FILE_BLOCK )
        {
            index = (lbn - INode::SMALL_FILE_BLOCK) % INode::ADDRESS_PER_INDEX_BLOCK;
        }
        else
        {
            index = (lbn - INode::LARGE_FILE_BLOCK) % INode::ADDRESS_PER_INDEX_BLOCK;
        }

        if( (phyBlkno = iTable[index]) == 0 && (pSecondBuf = fileSys.Alloc()) != NULL)
        {
            /* �����䵽���ļ������̿�ŵǼ���һ�μ���������� */
            phyBlkno = pSecondBuf->b_blkno;
            iTable[index] = phyBlkno;
            /* �������̿顢���ĺ��һ�μ�����������ӳ�д��ʽ��������� */
            bufMgr.Bdwrite(pSecondBuf);
            bufMgr.Bdwrite(pFirstBuf);
        }
        else
        {
            /* �ͷ�һ�μ��������ռ�û��� */
            bufMgr.Brelse(pFirstBuf);
        }
        /* �ҵ�Ԥ�����Ӧ�������̿�ţ������ȡԤ�������Ҫ�����һ��for����������IO�������㣬���� */
        rablock = 0;
        if( index + 1 < INode::ADDRESS_PER_INDEX_BLOCK)
        {
            rablock = iTable[index + 1];
        }
        return phyBlkno;
    }
}

void INode::ITrunc() {
/* ���ɴ��̸��ٻ����ȡ���һ�μ�ӡ����μ��������Ĵ��̿� */
    BufferManager& bm = Kernel::Instance().GetBufferManager();
    /* ��ȡg_FileSystem��������ã�ִ���ͷŴ��̿�Ĳ��� */
    FileSystem& filesys = Kernel::Instance().GetFileSystem();

    /* ������ַ��豸���߿��豸���˳� */
    if( this->i_mode & (INode::IFCHR & INode::IFBLK) )
    {
        return;
    }

    /* ����FILO��ʽ�ͷţ��Ծ���ʹ��SuperBlock�м�¼�Ŀ����̿��������
     * 
     * Unix V6++���ļ������ṹ��(С�͡����ͺ;����ļ�)
     * (1) i_addr[0] - i_addr[5]Ϊֱ���������ļ����ȷ�Χ��0 - 6���̿飻
     * 
     * (2) i_addr[6] - i_addr[7]���һ�μ�����������ڴ��̿�ţ�ÿ���̿�
     * �ϴ��128���ļ������̿�ţ������ļ����ȷ�Χ��7 - (128 * 2 + 6)���̿飻
     *
     * (3) i_addr[8] - i_addr[9]��Ŷ��μ�����������ڴ��̿�ţ�ÿ�����μ��
     * �������¼128��һ�μ�����������ڴ��̿�ţ������ļ����ȷ�Χ��
     * (128 * 2 + 6 ) < size <= (128 * 128 * 2 + 128 * 2 + 6)
     */
    for(int i = 9; i >= 0; i--)		/* ��i_addr[9]��i_addr[0] */
    {
        /* ���i_addr[]�е�i��������� */
        if( this->i_addr[i] != 0 )
        {
            /* �����i_addr[]�е�һ�μ�ӡ����μ�������� */
            if( i >= 6 && i <= 9 )
            {
                /* �������������뻺�� */
                Buf* pFirstBuf = bm.Bread(this->i_addr[i]);
                /* ��ȡ��������ַ */
                int* pFirst = (int *)pFirstBuf->b_addr;

                /* ÿ�ż���������¼ 512/sizeof(int) = 128�����̿�ţ�������ȫ��128�����̿� */
                for(int j = 128 - 1; j >= 0; j--)
                {
                    if( pFirst[j] != 0)	/* �������������� */
                    {
                        /* 
                         * ��������μ��������i_addr[8]��i_addr[9]�
                         * ��ô���ַ����¼����128��һ�μ���������ŵĴ��̿��
                         */
                        if( i >= 8 && i <= 9)
                        {
                            Buf* pSecondBuf = bm.Bread(pFirst[j]);
                            int* pSecond = (int *)pSecondBuf->b_addr;

                            for(int k = 128 - 1; k >= 0; k--)
                            {
                                if(pSecond[k] != 0)
                                {
                                    /* �ͷ�ָ���Ĵ��̿� */
                                    filesys.Free(pSecond[k]);
                                }
                            }
                            /* ����ʹ����ϣ��ͷ��Ա㱻��������ʹ�� */
                            bm.Brelse(pSecondBuf);
                        }
                        filesys.Free(pFirst[j]);
                    }
                }
                bm.Brelse(pFirstBuf);
            }
            /* �ͷ���������ռ�õĴ��̿� */
            filesys.Free(this->i_addr[i]);
            /* 0��ʾ����������� */
            this->i_addr[i] = 0;
        }
    }

    /* �̿��ͷ���ϣ��ļ���С���� */
    this->i_size0 = 0;
    /* ����IUPD��־λ����ʾ���ڴ�INode��Ҫͬ������Ӧ���INode */
    this->i_flag |= INode::IUPD;
    /* ����ļ���־ ��ԭ����RWXRWXRWX����*/
    this->i_mode &= ~(INode::ILARG & INode::IRWXU & INode::IRWXG & INode::IRWXO);
    this->i_nlink = 1;
}

INode::INode() {
    i_mode=0;
}

INode::~INode() {

}

void INode::Reset() {
    *this=INode();
}

void INode::ReadI() {
    int lbn;	/* �ļ��߼���� */
    int bn;		/* lbn��Ӧ�������̿�� */
    int offset;	/* ��ǰ�ַ�������ʼ����λ�� */
    int nbytes;	/* �������û�Ŀ�����ֽ����� */
    Buf* pBuf;
    User& u = Kernel::Instance().GetUser();
    BufferManager& bufMgr = Kernel::Instance().GetBufferManager();
    DiskDriver& devMgr = Kernel::Instance().GetDiskDriver();

    if( 0 == u.u_IOParam.m_Count )
    {
        /* ��Ҫ���ֽ���Ϊ�㣬�򷵻� */
        return;
    }

    this->i_flag |= INode::IACC;

    /* һ��һ���ַ���ض�������ȫ�����ݣ�ֱ�������ļ�β */
    while( User::U_NOERROR == u.u_error && u.u_IOParam.m_Count != 0)
    {
        lbn = bn = u.u_IOParam.m_Offset / INode::BLOCK_SIZE;
        offset = u.u_IOParam.m_Offset % INode::BLOCK_SIZE;
        /* ���͵��û������ֽ�������ȡ�������ʣ���ֽ����뵱ǰ�ַ�������Ч�ֽ�����Сֵ */
        nbytes = std::min(INode::BLOCK_SIZE - offset /* ������Ч�ֽ��� */, u.u_IOParam.m_Count);

        int remain = this->i_size0 - u.u_IOParam.m_Offset;
        /* ����Ѷ��������ļ���β */
        if( remain <= 0)
        {
            return;
        }
        /* ���͵��ֽ�������ȡ����ʣ���ļ��ĳ��� */
        nbytes = std::min(nbytes, remain);

        /* ���߼����lbnת���������̿��bn ��Bmap������Inode::rablock����UNIX��Ϊ��ȡԤ����Ŀ���̫��ʱ��
         * �����Ԥ������ʱ Inode::rablock ֵΪ 0��
         * */
        if( (bn = this->Bmap(lbn)) == 0 )
        {
            return;
        }

        pBuf = bufMgr.Bread(bn);

        /* ��¼�����ȡ�ַ�����߼���� */
        this->i_lastr = lbn;

        /* ������������ʼ��λ�� */
        unsigned char* start = pBuf->b_addr + offset;

        /* ������: �ӻ������������û�Ŀ����
         * i386оƬ��ͬһ��ҳ��ӳ���û��ռ���ں˿ռ䣬��һ��Ӳ���ϵĲ��� ʹ��i386��ʵ�� iomove����
         * ��PDP-11Ҫ�������*/
        memcpy(u.u_IOParam.m_Base, start,  nbytes);

        /* �ô����ֽ���nbytes���¶�дλ�� */
        u.u_IOParam.m_Base += nbytes;
        u.u_IOParam.m_Offset += nbytes;
        u.u_IOParam.m_Count -= nbytes;

        bufMgr.Brelse(pBuf);	/* ʹ���껺�棬�ͷŸ���Դ */
    }
}

void INode::WriteI() {
    int lbn;	/* �ļ��߼���� */
    int bn;		/* lbn��Ӧ�������̿�� */
    int offset;	/* ��ǰ�ַ�������ʼ����λ�� */
    int nbytes;	/* �����ֽ����� */
    Buf* pBuf;
    User& u = Kernel::Instance().GetUser();
    BufferManager& bufMgr = Kernel::Instance().GetBufferManager();
    DiskDriver& devMgr = Kernel::Instance().GetDiskDriver();

    /* ����Inode�����ʱ�־λ */
    this->i_flag |= (INode::IACC | INode::IUPD);

    if( 0 == u.u_IOParam.m_Count)
    {
        /* ��Ҫ���ֽ���Ϊ�㣬�򷵻� */
        return;
    }

    while( User::U_NOERROR == u.u_error && u.u_IOParam.m_Count != 0 )
    {
        lbn = u.u_IOParam.m_Offset / INode::BLOCK_SIZE;
        offset = u.u_IOParam.m_Offset % INode::BLOCK_SIZE;
        nbytes = std::min(INode::BLOCK_SIZE - offset, u.u_IOParam.m_Count);

        if( (this->i_mode & INode::IFMT) != INode::IFBLK )
        {	/* ��ͨ�ļ� */

            /* ���߼����lbnת���������̿��bn */
            if( (bn = this->Bmap(lbn)) == 0 )
            {
                return;
            }
        }

        if(INode::BLOCK_SIZE == nbytes)
        {
            /* ���д������������һ���ַ��飬��Ϊ����仺�� */
            pBuf = bufMgr.GetBlk(bn);
        }
        else
        {
            /* д�����ݲ���һ���ַ��飬�ȶ���д���������ַ����Ա�������Ҫ��д�����ݣ� */
            pBuf = bufMgr.Bread(bn);
        }

        /* ���������ݵ���ʼдλ�� */
        unsigned char* start = pBuf->b_addr + offset;

        /* д����: ���û�Ŀ�����������ݵ������� */
        memcpy(start,u.u_IOParam.m_Base,  nbytes);

        /* �ô����ֽ���nbytes���¶�дλ�� */
        u.u_IOParam.m_Base += nbytes;
        u.u_IOParam.m_Offset += nbytes;
        u.u_IOParam.m_Count -= nbytes;

        if( u.u_error != User::U_NOERROR )	/* д�����г��� */
        {
            bufMgr.Brelse(pBuf);
        }
        bufMgr.Bdwrite(pBuf);


        /* ��ͨ�ļ��������� */
        if( (this->i_size0 < u.u_IOParam.m_Offset) && (this->i_mode & (INode::IFBLK & INode::IFCHR)) == 0 )
        {
            this->i_size0 = u.u_IOParam.m_Offset;
        }

        this->i_flag |= INode::IUPD;
    }
}

void INode::Clean() {
    this->i_mode = 0;
    //this->i_count = 0;
    this->i_nlink = 0;
    //this->i_dev = -1;
    //this->i_number = -1;
    this->i_uid = -1;
    this->i_gid = -1;
    this->i_size0 = 0;
    this->i_lastr = -1;
    for(int i = 0; i < 10; i++)
    {
        this->i_addr[i] = 0;
    }
}

void INode::ICopy(Buf *bp, int inumber) {
    DiskINode dInode;
    DiskINode* pNode = &dInode;

    /* ��pָ�򻺴����б��Ϊinumber���Inode��ƫ��λ�� */
    unsigned char* p = bp->b_addr + (inumber % FileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskINode);
    /* �����������Inode���ݿ�������ʱ����dInode�У���4�ֽڿ��� */
    memcpy((int *)pNode, (int *)p, sizeof(DiskINode)/sizeof(int));

    //DiskINode& dINode = *(DiskINode*)(bp->b_addr + (inumber % FileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskINode));
    /* �����Inode����dInode����Ϣ���Ƶ��ڴ�Inode�� */
    this->i_mode = dInode.i_mode;
    this->i_nlink = dInode.i_nlink;
    this->i_uid = dInode.i_uid;
    this->i_gid = dInode.i_gid;
    this->i_size0 = dInode.i_size0;
    for(int i = 0; i < 10; i++)
    {
        this->i_addr[i] = dInode.i_addr[i];
    }
}

DiskINode::DiskINode() {
    i_mode=0;
}

DiskINode::~DiskINode() {

}
