//
// Created by roy on 27/03/2023.
//

#include "FileManager.h"
#include "Kernel.h"


void FileManager::Open() {
    INode* pInode;
    User& u = Kernel::Instance().GetUser();

    pInode = this->NameI(FileManager::OPEN);	/* 0 = Open, not create */
    /* û���ҵ���Ӧ��Inode */
    if ( NULL == pInode )
    {
        return;
    }
    this->Open1(pInode, u.u_arg[1], 0);
}

void FileManager::Creat() {
    INode* pInode;
    User& u = Kernel::Instance().GetUser();
    unsigned int newACCMode = u.u_arg[1];

    /* ����Ŀ¼��ģʽΪ1����ʾ����������Ŀ¼����д�������� */
    pInode = this->NameI(FileManager::CREATE);
    /* û���ҵ���Ӧ��Inode����NameI���� */
    if ( NULL == pInode )
    {
        if(u.u_error)
            return;
        /* ����Inode */
        pInode = this->MakNode(newACCMode);
        /* ����ʧ�� */
        if ( NULL == pInode )
        {
            return;
        }

        /*
         * �����ϣ�������ֲ����ڣ�ʹ�ò���trf = 2������open1()��
         * ����Ҫ����Ȩ�޼�飬��Ϊ�ոս������ļ���Ȩ�޺ʹ������mode
         * ����ʾ��Ȩ��������һ���ġ�
         */
        this->Open1(pInode, File::FWRITE, 2);
    }
    else
    {
        /* ���NameI()�������Ѿ�����Ҫ�������ļ�������ո��ļ������㷨ITrunc()����UIDû�иı�
         * ԭ��UNIX��������������ļ�����ȥ�����½����ļ�һ����Ȼ�������ļ������ߺ����Ȩ��ʽû�䡣
         * Ҳ����˵creatָ����RWX������Ч��
         * ������Ϊ���ǲ�����ģ�Ӧ�øı䡣
         * ���ڵ�ʵ�֣�creatָ����RWX������Ч */
        this->Open1(pInode, File::FWRITE, 1);
        pInode->i_mode |= newACCMode;
    }
}

void FileManager::Open1(INode *pInode, int mode, int trf) {
    User& u = Kernel::Instance().GetUser();

    /*
     * ����ϣ�����ļ��Ѵ��ڵ�����£���trf == 0��trf == 1����Ȩ�޼��
     * �����ϣ�������ֲ����ڣ���trf == 2������Ҫ����Ȩ�޼�飬��Ϊ�ս���
     * ���ļ���Ȩ�޺ʹ���Ĳ���mode������ʾ��Ȩ��������һ���ġ�
     */
    if (trf != 2)
    {
        if ( mode & File::FREAD )
        {
            /* ����Ȩ�� */
            //this->Access(pInode, INode::IREAD);
        }
        if ( mode & File::FWRITE )
        {
            /* ���дȨ�� */
            //this->Access(pInode, INode::IWRITE);
            /* ϵͳ����ȥдĿ¼�ļ��ǲ������ */
            if ( (pInode->i_mode & INode::IFMT) == INode::IFDIR )
            {
                u.u_error = User::U_EISDIR;
            }
        }
    }

    if ( u.u_error )
    {
        this->m_InodeTable->IPut(pInode);
        return;
    }

    /* ��creat�ļ���ʱ��������ͬ�ļ������ļ����ͷŸ��ļ���ռ�ݵ������̿� */
    if ( 1 == trf )
    {
        pInode->ITrunc();
    }

    /* ����inode!
     * ����Ŀ¼�����漰�����Ĵ��̶�д�������ڼ���̻���˯��
     * ��ˣ����̱������������漰��i�ڵ㡣�����NameI��ִ�е�IGet����������
     * �����ˣ����������п��ܻ���������л��Ĳ��������Խ���i�ڵ㡣
     */
    //pInode->Prele();

    /* ������ļ����ƿ�File�ṹ */
    File* pFile = this->m_OpenFileTable->FAlloc();
    if ( NULL == pFile )
    {
        this->m_InodeTable->IPut(pInode);
        return;
    }
    /* ���ô��ļ���ʽ������File�ṹ���ڴ�Inode�Ĺ�����ϵ */
    pFile->f_flag = mode & (File::FREAD | File::FWRITE);
    pFile->f_inode = pInode;

    /* �����豸�򿪺��� */
//    pInode->OpenI(mode & File::FWRITE);

    /* Ϊ�򿪻��ߴ����ļ��ĸ�����Դ���ѳɹ����䣬�������� */
    if ( u.u_error == 0 )
    {
        return;
    }
    else	/* ����������ͷ���Դ */
    {
        /* �ͷŴ��ļ������� */
        int fd = u.u_ar0[User::EAX];
        if(fd != -1)
        {
            u.u_ofiles.SetF(fd, NULL);
            /* �ݼ�File�ṹ��Inode�����ü��� ,File�ṹû���� f_countΪ0�����ͷ�File�ṹ��*/
            pFile->f_count--;
        }
        this->m_InodeTable->IPut(pInode);
    }
}

void FileManager::Close() {
    User& u = Kernel::Instance().GetUser();
    int fd = u.u_arg[0];

    /* ��ȡ���ļ����ƿ�File�ṹ */
    File* pFile = u.u_ofiles.GetF(fd);
    if ( NULL == pFile )
    {
        return;
    }

    /* �ͷŴ��ļ�������fd���ݼ�File�ṹ���ü��� */
    u.u_ofiles.SetF(fd, NULL);
    this->m_OpenFileTable->CloseF(pFile);
}

void FileManager::Seek() {
    File* pFile;
    User& u = Kernel::Instance().GetUser();
    int fd = u.u_arg[0];

    pFile = u.u_ofiles.GetF(fd);
    if ( NULL == pFile )
    {
        return;  /* ��FILE�����ڣ�GetF��������� */
    }

    /* �ܵ��ļ�������seek */
    if ( pFile->f_flag & File::FPIPE )
    {
        u.u_error = User::U_ESPIPE;
        return;
    }

    int offset = u.u_arg[1];

    /* ���u.u_arg[2]��3 ~ 5֮�䣬��ô���ȵ�λ���ֽڱ�Ϊ512�ֽ� */
    if ( u.u_arg[2] > 2 )
    {
        offset = offset << 9;
        u.u_arg[2] -= 3;
    }

    switch ( u.u_arg[2] )
    {
        /* ��дλ������Ϊoffset */
        case 0:
            pFile->f_offset = offset;
            break;
            /* ��дλ�ü�offset(�����ɸ�) */
        case 1:
            pFile->f_offset += offset;
            break;
            /* ��дλ�õ���Ϊ�ļ����ȼ�offset */
        case 2:
            pFile->f_offset = pFile->f_inode->i_size0 + offset;
            break;
    }
}

void FileManager::Read() {
    this->Rdwr(File::FREAD);
}

void FileManager::Write() {
    this->Rdwr(File::FWRITE);
}

void FileManager::Rdwr(File::FileFlags mode) {
    File* pFile;
    User& u = Kernel::Instance().GetUser();

    /* ����Read()/Write()��ϵͳ���ò���fd��ȡ���ļ����ƿ�ṹ */
    pFile = u.u_ofiles.GetF(u.u_arg[0]);	/* fd */
    if ( NULL == pFile )
    {
        /* �����ڸô��ļ���GetF�Ѿ����ù������룬�������ﲻ��Ҫ�������� */
        /*	u.u_error = User::EBADF;	*/
        return;
    }


    /* ��д��ģʽ����ȷ */
    if ( (pFile->f_flag & mode) == 0 )
    {
        u.u_error = User::U_EACCES;
        return;
    }

    u.u_IOParam.m_Base = (unsigned char *)u.u_arg[1];	/* Ŀ�껺������ַ */
    u.u_IOParam.m_Count = u.u_arg[2];		/* Ҫ���/д���ֽ��� */
    u.u_segflg = 0;		/* User Space I/O�����������Ҫ�����ݶλ��û�ջ�� */

    /* �����ļ���ʼ��λ�� */
    u.u_IOParam.m_Offset = pFile->f_offset;
    if ( File::FREAD == mode )
    {
        pFile->f_inode->ReadI();
    }
    else
    {
        pFile->f_inode->WriteI();
    }

    /* ���ݶ�д�������ƶ��ļ���дƫ��ָ�� */
    pFile->f_offset += (u.u_arg[2] - u.u_IOParam.m_Count);

    /* ����ʵ�ʶ�д���ֽ������޸Ĵ��ϵͳ���÷���ֵ�ĺ���ջ��Ԫ */
    u.u_ar0[User::EAX] = u.u_arg[2] - u.u_IOParam.m_Count;
}

INode *FileManager::NameI(FileManager::DirectorySearchMode mode)
{
    INode* pInode;
    Buf* pBuf;
    char curchar;
    char* pChar;
    int freeEntryOffset;	/* �Դ����ļ�ģʽ����Ŀ¼ʱ����¼����Ŀ¼���ƫ���� */
    User& u = Kernel::Instance().GetUser();
    BufferManager& bufMgr = Kernel::Instance().GetBufferManager();

    /*
     * �����·����'/'��ͷ�ģ��Ӹ�Ŀ¼��ʼ������
     * ����ӽ��̵�ǰ����Ŀ¼��ʼ������
     */
    pInode = u.u_cdir;
    if ( '/' == (curchar = NextChar()) )
    {
        pInode = this->rootDirInode;
    }

    /* ����Inode�Ƿ����ڱ�ʹ�ã��Լ���֤������Ŀ¼���������и�Inode�����ͷ� */
    this->m_InodeTable->IGet(pInode->i_number);

    /* �������////a//b ����·�� ����·���ȼ���/a/b */
    while ( '/' == curchar )
    {
        curchar =NextChar();
    }
    /* �����ͼ���ĺ�ɾ����ǰĿ¼�ļ������ */
    if ( '\0' == curchar && mode != FileManager::OPEN )
    {
        u.u_error = User::U_ENOENT;
        goto out;
    }

    /* ���ѭ��ÿ�δ���pathname��һ��·������ */
    while (true)
    {
        /* ����������ͷŵ�ǰ��������Ŀ¼�ļ�Inode�����˳� */
        if ( u.u_error != User::U_NOERROR )
        {
            break;	/* goto out; */
        }

        /* ����·��������ϣ�������ӦInodeָ�롣Ŀ¼�����ɹ����ء� */
        if ( '\0' == curchar )
        {
            return pInode;
        }

        /* ���Ҫ���������Ĳ���Ŀ¼�ļ����ͷ����Inode��Դ���˳� */
        if ( (pInode->i_mode & INode::IFMT) != INode::IFDIR )
        {
            u.u_error = User::U_ENOTDIR;
            break;	/* goto out; */
        }

        /*
         * ��Pathname�е�ǰ׼������ƥ���·������������u.u_dbuf[]�У�
         * ���ں�Ŀ¼����бȽϡ�
         */
        pChar = &(u.u_dbuf[0]);
        while ( '/' != curchar && '\0' != curchar && u.u_error == User::U_NOERROR )
        {
            if ( pChar < &(u.u_dbuf[DirectoryEntry::DIRSIZ]) )
            {
                *pChar = curchar;
                pChar++;
            }
            curchar = NextChar();
        }
        /* ��u_dbufʣ��Ĳ������Ϊ'\0' */
        while ( pChar < &(u.u_dbuf[DirectoryEntry::DIRSIZ]) )
        {
            *pChar = '\0';
            pChar++;
        }

        /* �������////a//b ����·�� ����·���ȼ���/a/b */
        while ( '/' == curchar )
        {
            curchar = NextChar();
        }

        if ( u.u_error != User::U_NOERROR )
        {
            break; /* goto out; */
        }

        /* �ڲ�ѭ�����ֶ���u.u_dbuf[]�е�·���������������Ѱƥ���Ŀ¼�� */
        u.u_IOParam.m_Offset = 0;
        /* ����ΪĿ¼����� �����հ׵�Ŀ¼��*/
        u.u_IOParam.m_Count = pInode->i_size0 / (DirectoryEntry::DIRSIZ + 4);
        freeEntryOffset = 0;
        pBuf = NULL;

        while (true)
        {
            /* ��Ŀ¼���Ѿ�������� */
            if ( 0 == u.u_IOParam.m_Count )
            {
                if ( NULL != pBuf )
                {
                    bufMgr.Brelse(pBuf);
                }
                /* ����Ǵ������ļ� */
                if ( FileManager::CREATE == mode && curchar == '\0' )
                {
//                    /* �жϸ�Ŀ¼�Ƿ��д */
//                    if ( this->Access(pInode, INode::IWRITE) )
//                    {
//                        u.u_error = User::U_EACCES;
//                        goto out;	/* Failed */
//                    }

                    /* ����Ŀ¼Inodeָ�뱣���������Ժ�дĿ¼��WriteDir()�������õ� */
                    u.u_pdir = pInode;

                    if ( freeEntryOffset )	/* �˱�������˿���Ŀ¼��λ��Ŀ¼�ļ��е�ƫ���� */
                    {
                        /* ������Ŀ¼��ƫ��������u���У�дĿ¼��WriteDir()���õ� */
                        u.u_IOParam.m_Offset = freeEntryOffset - (DirectoryEntry::DIRSIZ + 4);
                    }
                    else  /*���⣺Ϊ��if��֧û����IUPD��־��  ������Ϊ�ļ��ĳ���û�б�ѽ*/
                    {
                        pInode->i_flag |= INode::IUPD;
                    }
                    /* �ҵ�����д��Ŀ���Ŀ¼��λ�ã�NameI()�������� */
                    return NULL;
                }

                /* Ŀ¼��������϶�û���ҵ�ƥ����ͷ����Inode��Դ�����Ƴ� */
                u.u_error = User::U_ENOENT;
                goto out;
            }

            /* �Ѷ���Ŀ¼�ļ��ĵ�ǰ�̿飬��Ҫ������һĿ¼�������̿� */
            if ( 0 == u.u_IOParam.m_Offset % INode::BLOCK_SIZE )
            {
                if ( NULL != pBuf )
                {
                    bufMgr.Brelse(pBuf);
                }
                /* ����Ҫ���������̿�� */
                int phyBlkno = pInode->Bmap(u.u_IOParam.m_Offset / INode::BLOCK_SIZE );
                pBuf = bufMgr.Bread(phyBlkno );
            }

            /* û�ж��굱ǰĿ¼���̿飬���ȡ��һĿ¼����u.u_dent */
            int* src = (int *)(pBuf->b_addr + (u.u_IOParam.m_Offset % INode::BLOCK_SIZE));


            u.u_dent = *(DirectoryEntry*)src;

            u.u_IOParam.m_Offset += (DirectoryEntry::DIRSIZ + 4);
            u.u_IOParam.m_Count--;

            /* ����ǿ���Ŀ¼���¼����λ��Ŀ¼�ļ���ƫ���� */
            if ( 0 == u.u_dent.m_ino )
            {
                continue;
            }

            int i;
            for ( i = 0; i < DirectoryEntry::DIRSIZ; i++ )
            {
                if ( u.u_dbuf[i] != u.u_dent.m_name[i] )
                {
                    break;	/* ƥ����ĳһ�ַ�����������forѭ�� */
                }
            }

            if( i < DirectoryEntry::DIRSIZ )
            {
                /* ����Ҫ������Ŀ¼�����ƥ����һĿ¼�� */
                continue;
            }
            else
            {
                /* Ŀ¼��ƥ��ɹ����ص����While(true)ѭ�� */
                break;
            }
        }

        /*
         * ���ڲ�Ŀ¼��ƥ��ѭ�������˴���˵��pathname��
         * ��ǰ·������ƥ��ɹ��ˣ�����ƥ��pathname����һ·��
         * ������ֱ������'\0'������
         */
        if ( NULL != pBuf )
        {
            bufMgr.Brelse(pBuf);
        }

        /* �����ɾ���������򷵻ظ�Ŀ¼Inode����Ҫɾ���ļ���Inode����u.u_dent.m_ino�� */
        if ( FileManager::DELETE == mode && '\0' == curchar )
        {
            return pInode;
        }

        /*
         * ƥ��Ŀ¼��ɹ������ͷŵ�ǰĿ¼Inode������ƥ��ɹ���
         * Ŀ¼��m_ino�ֶλ�ȡ��Ӧ��һ��Ŀ¼���ļ���Inode��
         */
        this->m_InodeTable->IPut(pInode);
        pInode = this->m_InodeTable->IGet(u.u_dent.m_ino);
        /* �ص����While(true)ѭ��������ƥ��Pathname����һ·������ */

        if ( NULL == pInode )	/* ��ȡʧ�� */
        {
            return NULL;
        }
    }
    out:
    this->m_InodeTable->IPut(pInode);
    return NULL;
}

INode *FileManager::MakNode(unsigned int mode) {
    INode* pInode;
    User& u = Kernel::Instance().GetUser();

    /* ����һ������DiskInode������������ȫ����� */
    pInode = this->m_FileSystem->IAlloc();
    if( NULL ==	pInode )
    {
        return NULL;
    }

    pInode->i_flag |= (INode::IACC | INode::IUPD);
    pInode->i_mode = mode | INode::IALLOC;
    pInode->i_nlink = 1;
    pInode->i_uid = u.u_uid;
    pInode->i_gid = u.u_gid;
    /* ��Ŀ¼��д��u.u_dent�����д��Ŀ¼�ļ� */
    this->WriteDir(pInode);
    return pInode;
}

void FileManager::WriteDir(INode *pInode) {
    User& u = Kernel::Instance().GetUser();

    /* ����Ŀ¼����Inode��Ų��� */
    u.u_dent.m_ino = pInode->i_number;

    /* ����Ŀ¼����pathname�������� */
    for ( int i = 0; i < DirectoryEntry::DIRSIZ; i++ )
    {
        u.u_dent.m_name[i] = u.u_dbuf[i];
    }

    u.u_IOParam.m_Count = DirectoryEntry::DIRSIZ + 4;
    u.u_IOParam.m_Base = (unsigned char *)&u.u_dent;
    u.u_segflg = 1;

    /* ��Ŀ¼��д�븸Ŀ¼�ļ� */
    u.u_pdir->WriteI();
    this->m_InodeTable->IPut(u.u_pdir);
}

void FileManager::ChDir() {
    INode* pInode;
    User& u = Kernel::Instance().GetUser();

    //����cd ..����Ҫ����
    char* path = (char*)u.u_arg[0];

    if (path[0] == '.' && path[1] == '.' && path[2] == '\0')
    {
        int left=0;
        delete []path;
        path=new char[strlen(u.u_curdir)+1];
        strcpy(path,u.u_curdir);
        int length=strlen(path);
        for(int i=length-2;i>=0;i--){
            if(path[i]=='/'){
                left=i;
                break;
            }
        }
        for(int i=left+1;i<length;i++){
            path[i]='\0';
        }

        u.u_arg[0]=(int)path;
    }

    pInode = this->NameI(FileManager::OPEN);
    if ( NULL == pInode )
    {
        return;
    }
    /* ���������ļ�����Ŀ¼�ļ� */
    if ( (pInode->i_mode & INode::IFMT) != INode::IFDIR )
    {
        u.u_error = User::U_ENOTDIR;
        this->m_InodeTable->IPut(pInode);
        return;
    }

    this->m_InodeTable->IPut(u.u_cdir);
    u.u_cdir = pInode;
    //pInode->Prele();

    this->SetCurDir((char *)u.u_arg[0] /* pathname */);
}

void FileManager::UnLink() {
    INode* pInode;
    INode* pDeleteInode;
    User& u = Kernel::Instance().GetUser();

    pDeleteInode = this->NameI(FileManager::DELETE);
    if ( NULL == pDeleteInode )
    {
        return;
    }

    pInode = this->m_InodeTable->IGet(u.u_dent.m_ino);
    if ( NULL == pInode )
    {
        return ;
    }
    /* ���Ŀ¼����û���ļ������򱨴� */
//    if ( (pInode->i_mode & INode::IFMT) == INode::IFDIR)
//    {
//        this->m_InodeTable->IPut(pDeleteInode);
//        this->m_InodeTable->IPut(pInode);
//        return;
//    }
    /* д��������Ŀ¼�� */
    u.u_IOParam.m_Offset -= (DirectoryEntry::DIRSIZ + 4);
    u.u_IOParam.m_Base = (unsigned char *)&u.u_dent;
    u.u_IOParam.m_Count = DirectoryEntry::DIRSIZ + 4;

    u.u_dent.m_ino = 0;
    pDeleteInode->WriteI();

    /* �޸�inode�� */
    pInode->i_nlink--;
    pInode->i_flag |= INode::IUPD;

    this->m_InodeTable->IPut(pDeleteInode);
    this->m_InodeTable->IPut(pInode);
}

FileManager::FileManager() {
    m_FileSystem=&Kernel::Instance().GetFileSystem();
    m_OpenFileTable=&g_OpenFileTable;
    m_InodeTable=&g_InodeTable;
    rootDirInode = m_InodeTable->IGet(FileSystem::ROOT_INODE_NO);

    rootDirInode->i_count = 0x7f;//���ü���
}

char FileManager::NextChar() {
    User& u = Kernel::Instance().GetUser();

    /* u.u_dirpָ��pathname�е��ַ� */
    return *u.u_dirp++;
}

void FileManager::SetCurDir(char *pathname) {
    User& u = Kernel::Instance().GetUser();

    /* ·�����ǴӸ�Ŀ¼'/'��ʼ����������u.u_curdir������ϵ�ǰ·������ */
    if ( pathname[0] != '/' )
    {
        strcat(u.u_curdir,pathname);
        strcat(u.u_curdir, "/");
    }
    else	/* ����ǴӸ�Ŀ¼'/'��ʼ����ȡ��ԭ�й���Ŀ¼ */
    {
        strcpy(u.u_curdir,pathname);
    }

}

void FileManager::Ls() {
    INode* pInode;
    Buf* pBuf=NULL;
    int freeEntryOffset;	/* �Դ����ļ�ģʽ����Ŀ¼ʱ����¼����Ŀ¼���ƫ���� */
    char* pFileName=new char[1024];
    memset(pFileName, 0, 1024);
    User& u = Kernel::Instance().GetUser();
    BufferManager& bufMgr = Kernel::Instance().GetBufferManager();

    pInode=u.u_cdir;
    u.u_IOParam.m_Offset = 0;
    u.u_IOParam.m_Count = pInode->i_size0 / (DirectoryEntry::DIRSIZ + 4);
    /* ����Inode�Ƿ����ڱ�ʹ�ã��Լ���֤������Ŀ¼���������и�Inode�����ͷ� */
    this->m_InodeTable->IGet(pInode->i_number);

    while (true)
    {
        /* ����������ͷŵ�ǰ��������Ŀ¼�ļ�Inode�����˳� */
        if ( u.u_error != User::U_NOERROR )
        {
            break;	/* goto out; */
        }

        while (true)
        {
            /* ��Ŀ¼���Ѿ�������� */
            if ( 0 == u.u_IOParam.m_Count )
            {
                if ( NULL != pBuf )
                {
                    bufMgr.Brelse(pBuf);
                }

                goto out;
            }

            /* �Ѷ���Ŀ¼�ļ��ĵ�ǰ�̿飬��Ҫ������һĿ¼�������̿� */
            if ( 0 == u.u_IOParam.m_Offset % INode::BLOCK_SIZE )
            {
                if ( NULL != pBuf )
                {
                    bufMgr.Brelse(pBuf);
                }
                /* ����Ҫ���������̿�� */
                int phyBlkno = pInode->Bmap(u.u_IOParam.m_Offset / INode::BLOCK_SIZE );
                pBuf = bufMgr.Bread(phyBlkno );
            }

            /* û�ж��굱ǰĿ¼���̿飬���ȡ��һĿ¼����u.u_dent */
            int* src = (int *)(pBuf->b_addr + (u.u_IOParam.m_Offset % INode::BLOCK_SIZE));

            u.u_dent = *(DirectoryEntry*)src;

            u.u_IOParam.m_Offset += (DirectoryEntry::DIRSIZ + 4);
            u.u_IOParam.m_Count--;

            /* ����ǿ���Ŀ¼���¼����λ��Ŀ¼�ļ���ƫ���� */
            if ( 0 == u.u_dent.m_ino )
            {
                if ( 0 == freeEntryOffset )
                {
                    freeEntryOffset = u.u_IOParam.m_Offset;
                }
                /* ��������Ŀ¼������Ƚ���һĿ¼�� */
                continue;
            }

            strcat(pFileName, u.u_dent.m_name);
            strcat(pFileName, " ");
        }

    }
    out:
    u.u_ar0[User::EAX]=(int)pFileName;
}

FileManager::~FileManager() = default;
DirectoryEntry::DirectoryEntry()= default;
DirectoryEntry::~DirectoryEntry()= default;
