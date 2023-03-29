//
// Created by roy on 26/03/2023.
//

#ifndef OSFILE_OPENFILEMANAGER_H
#define OSFILE_OPENFILEMANAGER_H

#include "INode.h"
#include "File.h"
#include "FileSystem.h"

class INodeTable;
class OpenFileTable;

extern INodeTable g_InodeTable;
extern OpenFileTable g_OpenFileTable;

class INodeTable {
public:
    static const int NINODE = 100;           //�ڴ�INode������
private:
    INode m_INodeTable[NINODE];              //�ڴ�INode���飬ÿ�����ļ�����ռ��һ���ڴ�INode
    FileSystem* fileSystem;                  //��ȫ�ֶ���g_FileSystem������

public:
    INodeTable();
    ~INodeTable();
    INode* IGet(int inumber);                //�������INode��Ż�ȡ��ӦINode�������INode�Ѿ����ڴ��У����ظ��ڴ�INode��
    //��������ڴ��У���������ڴ�����������ظ��ڴ�INode
    void IPut(INode* pNode);                 //���ٸ��ڴ�INode�����ü����������INode�Ѿ�û��Ŀ¼��ָ������
    //���޽������ø�INode�����ͷŴ��ļ�ռ�õĴ��̿�
    void UpdateINodeTable();                 //�����б��޸Ĺ����ڴ�INode���µ���Ӧ���INode��
    int IsLoaded(int inumber);               //�����Ϊinumber�����INode�Ƿ����ڴ濽����
    //������򷵻ظ��ڴ�INode���ڴ�INode���е�����
    INode* GetFreeINode();                   //���ڴ�INode����Ѱ��һ�����е��ڴ�INode
    void Reset();
};

class OpenFileTable
{
public:
    /* static consts */
    //static const int NINODE	= 100;	/* �ڴ�Inode������ */
    static const int NFILE	= 100;	/* ���ļ����ƿ�File�ṹ������ */

    /* Functions */
public:
    /* Constructors */
    OpenFileTable();
    /* Destructors */
    ~OpenFileTable();

    void Reset();

    // /*
    // * @comment �����û�ϵͳ�����ṩ���ļ�����������fd��
    // * �ҵ���Ӧ�Ĵ��ļ����ƿ�File�ṹ
    // */
    // File* GetF(int fd);
    /*
     * @comment ��ϵͳ���ļ����з���һ�����е�File�ṹ
     */
    File* FAlloc();
    /*
     * @comment �Դ��ļ����ƿ�File�ṹ�����ü���f_count��1��
     * �����ü���f_countΪ0�����ͷ�File�ṹ��
     */
    void CloseF(File* pFile);

    /* Members */
public:
    File m_File[NFILE];			/* ϵͳ���ļ���Ϊ���н��̹������̴��ļ���������
								�а���ָ����ļ����ж�ӦFile�ṹ��ָ�롣*/
};




#endif //OSFILE_OPENFILEMANAGER_H
