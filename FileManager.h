//
// Created by roy on 27/03/2023.
//

#ifndef OSFILE_FILEMANAGER_H
#define OSFILE_FILEMANAGER_H

#include "File.h"
#include "OpenFileManager.h"

class DirectoryEntry
{
    /* static members */
public:
    static const int DIRSIZ = 28;	/* Ŀ¼����·�����ֵ�����ַ������� */

    /* Functions */
public:
    /* Constructors */
    DirectoryEntry();
    /* Destructors */
    ~DirectoryEntry();

    /* Members */
public:
    int m_ino;		/* Ŀ¼����Inode��Ų��� */
    char m_name[DIRSIZ];	/* Ŀ¼����·�������� */
};

class FileManager
{
public:
    /* Ŀ¼����ģʽ������NameI()���� */
    enum DirectorySearchMode
    {
        OPEN = 0,		/* �Դ��ļ���ʽ����Ŀ¼ */
        CREATE = 1,		/* ���½��ļ���ʽ����Ŀ¼ */
        DELETE = 2		/* ��ɾ���ļ���ʽ����Ŀ¼ */
    };

    /* Functions */
public:
    /* Constructors */
    FileManager();
    /* Destructors */
    ~FileManager();


    /*
     * @comment Open()ϵͳ���ô������
     */
    void Open();

    /*
     * @comment Creat()ϵͳ���ô������
     */
    void Creat();

    /*
     * @comment Open()��Creat()ϵͳ���õĹ�������
     */
    void Open1(INode* pInode, int mode, int trf);

    /*
     * @comment Close()ϵͳ���ô������
     */
    void Close();

    /*
     * @comment Seek()ϵͳ���ô������
     */
    void Seek();

    /*
     * @comment Read()ϵͳ���ô������
     */
    void Read();

    /*
     * @comment Write()ϵͳ���ô������
     */
    void Write();

    /*
     * @comment ��дϵͳ���ù������ִ���
     */
    void Rdwr(enum File::FileFlags mode);

    /*
     * @comment Ŀ¼��������·��ת��Ϊ��Ӧ��Inode��
     * �����������Inode
     */
    INode* NameI(enum DirectorySearchMode mode);

    /*
     * @comment ��ȡ·���е���һ���ַ�
     */
    static char NextChar();

    /*
     * @comment ��Creat()ϵͳ����ʹ�ã�����Ϊ�������ļ������ں���Դ
     */
    INode* MakNode(unsigned int mode);

    /*
     * @comment ��Ŀ¼��Ŀ¼�ļ�д��һ��Ŀ¼��
     */
    void WriteDir(INode* pInode);

    /*
     * @comment ���õ�ǰ����·��
     */
    void SetCurDir(char* pathname);

    /* �ı䵱ǰ����Ŀ¼ */
    void ChDir();

    /* ȡ���ļ� */
    void UnLink();

    /* �鿴Ŀ¼�������ļ�*/
    void Ls();

public:
    /* ��Ŀ¼�ڴ�Inode */
    INode* rootDirInode;

    /* ��ȫ�ֶ���g_FileSystem�����ã��ö���������ļ�ϵͳ�洢��Դ */
    FileSystem* m_FileSystem;

    /* ��ȫ�ֶ���g_InodeTable�����ã��ö������ڴ�Inode��Ĺ��� */
    INodeTable* m_InodeTable;

    /* ��ȫ�ֶ���g_OpenFileTable�����ã��ö�������ļ�����Ĺ��� */
    OpenFileTable* m_OpenFileTable;
};


#endif //OSFILE_FILEMANAGER_H
