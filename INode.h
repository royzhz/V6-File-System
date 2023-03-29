//
// Created by roy on 26/03/2023.
//

#ifndef OSFILE_INODE_H
#define OSFILE_INODE_H

#include "BufferManager.h"



class INode
{
public:

    //INodeFlag�еı�־λ
    enum INodeFlag
    {
        IUPD = 0x1,//�ڴ�INode���޸Ĺ�����Ҫ���¶�Ӧ���INode
        IACC = 0x2 //�ڴ�INode�����ʹ�����Ҫ�޸����һ�η���ʱ��
    };

    static const unsigned int IALLOC = 0x8000;		/* �ļ���ʹ�� */
    static const unsigned int IFMT = 0x6000;		/* �ļ��������� */
    static const unsigned int IFDIR = 0x4000;		/* �ļ����ͣ�Ŀ¼�ļ� */
    static const unsigned int IFCHR = 0x2000;		/* �ַ��豸���������ļ� */
    static const unsigned int IFBLK = 0x6000;		/* ���豸���������ļ���Ϊ0��ʾ���������ļ� */
    static const unsigned int ILARG = 0x1000;		/* �ļ��������ͣ����ͻ�����ļ� */
    static const unsigned int ISUID = 0x800;		/* ִ��ʱ�ļ�ʱ���û�����Ч�û�ID�޸�Ϊ�ļ������ߵ�User ID */
    static const unsigned int ISGID = 0x400;		/* ִ��ʱ�ļ�ʱ���û�����Ч��ID�޸�Ϊ�ļ������ߵ�Group ID */
    static const unsigned int ISVTX = 0x200;		/* ʹ�ú���Ȼλ�ڽ������ϵ����Ķ� */
    static const unsigned int IREAD = 0x100;		/* ���ļ��Ķ�Ȩ�� */
    static const unsigned int IWRITE = 0x80;		/* ���ļ���дȨ�� */
    static const unsigned int IEXEC = 0x40;			/* ���ļ���ִ��Ȩ�� */
    static const unsigned int IRWXU = (IREAD|IWRITE|IEXEC);		/* �ļ������ļ��Ķ���д��ִ��Ȩ�� */
    static const unsigned int IRWXG = ((IRWXU) >> 3);			/* �ļ���ͬ���û����ļ��Ķ���д��ִ��Ȩ�� */
    static const unsigned int IRWXO = ((IRWXU) >> 6);			/* �����û����ļ��Ķ���д��ִ��Ȩ�� */

    static const int BLOCK_SIZE = 512;                                        //�ļ��߼����С��512�ֽ�
    static const int ADDRESS_PER_INDEX_BLOCK = BLOCK_SIZE / sizeof(int);      //ÿ������������������飩�����������̿��

    static const int SMALL_FILE_BLOCK = 6;                                    //С���ļ���ֱ������������Ѱַ���߼����
    static const int LARGE_FILE_BLOCK = 128 * 2 + 6;                          //�����ļ�����һ�μ������������Ѱַ���߼����
    static const int HUGE_FILE_BLOCK = 128 * 128 * 2 + 128 * 2 + 6;
    //�����ļ��������μ����������Ѱַ�ļ��߼����

    /* static member */
    static int rablock;		/* ˳���ʱ��ʹ��Ԥ�����������ļ�����һ�ַ��飬rablock��¼����һ�߼����
							����bmapת���õ��������̿�š���rablock��Ϊ��̬������ԭ�򣺵���һ��bmap�Ŀ���
							�Ե�ǰ���Ԥ������߼���Ž���ת����bmap���ص�ǰ��������̿�ţ����ҽ�Ԥ����
							�������̿�ű�����rablock�С� */

    char i_flag;//��־
    char i_count ;//���ü���
    //int i_dev;//�豸��
    int i_number;//Inode��
    int i_mode ;//�ļ����͡�Ȩ��
    char i_nlink;//�ļ���������
    char i_uid;//�û�ID
    char i_gid;//��ID
    int i_size0 ;//�ļ����ȵĸ�λ8
    //char *i_size1;//�ļ����ȵĵ�λ16����
    int i_addr [10];//�ļ����ݵ������̿��
    int i_lastr;//���һ�ζ�ȡ�ļ����߼����


public:
    INode();
    ~INode();
    void Reset();
    void ReadI();                           //����Inode�����е�������̿���������ȡ��Ӧ���ļ�����
    void WriteI();                          //����Inode�����е�������̿�������������д���ļ�
    int Bmap(int lbn);                      //���ļ����߼����ת���ɶ�Ӧ�������̿��
    void IUpdate(int time);                 //�������Inode�����ķ���ʱ�䡢�޸�ʱ��
    void ITrunc();                          //�ͷ�Inode��Ӧ�ļ�ռ�õĴ��̿�
    void Clean();                           //���Inode�����е�����
    void ICopy(Buf* bp, int inumber);//���������Inode�ַ�������Ϣ�������ڴ�Inode��
};


class DiskINode//64�ֽ�
{
public:
    int     i_mode;//״̬��������Ϣ����λ9���ر�ʾȨ�ޡ�
    char    i_nlink ;//����Ŀ¼�Ĳ�������
    char    i_uid;//�û�ID
    char    i_gid;//��ID
    int    i_size0 ;//�ļ����ȵĸ�λ8
    //char    *i_sizel ;//�ļ����ȵĵ�λ16����
    int     i_addr [10] ;//�ļ����ݵ������̿��
    int     i_atime ;//���һ�η���ʱ��
    int     i_mtime ;//���һ���޸�ʱ��
    int     i_ctime ;//���һ�θı�ʱ��
public:
    DiskINode();
    ~DiskINode();
};

#endif //OSFILE_INODE_H
