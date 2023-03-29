//
// Created by roy on 26/03/2023.
//

#ifndef OSFILE_BUFFERMANAGER_H
#define OSFILE_BUFFERMANAGER_H

#include<cstring>
#include<unordered_map>
#include "DiskManager.h"

class Buf
{
public:
    enum BufFlag	/* b_flags�б�־λ */
    {
        B_WRITE = 0x1,		/* д�������������е���Ϣд��Ӳ����ȥ */
        B_READ	= 0x2,		/* �����������̶�ȡ��Ϣ�������� */
        B_DONE	= 0x4,		/* I/O�������� */
        B_ERROR	= 0x8,		/* I/O��������ֹ */
        B_BUSY	= 0x10,		/* ��Ӧ��������ʹ���� */
        B_WANTED = 0x20,	/* �н������ڵȴ�ʹ�ø�buf�������Դ����B_BUSY��־ʱ��Ҫ�������ֽ��� */
        B_ASYNC	= 0x40,		/* �첽I/O������Ҫ�ȴ������ */
        B_DELWRI = 0x80		/* �ӳ�д������Ӧ����Ҫ��������ʱ���ٽ�������д����Ӧ���豸�� */
    };

public:
    unsigned int b_flags;	/* ������ƿ��־λ */

    int		padding;		/* 4�ֽ���䣬ʹ��b_forw��b_back��Buf������Devtab��
							 * �е��ֶ�˳���ܹ�һ�£�����ǿ��ת������� */
    /* ������ƿ���й���ָ�� */
    Buf*	b_forw;
    Buf*	b_back;
    Buf*	av_forw;
    Buf*	av_back;

    short	b_dev;			/* �������豸�ţ����и�8λ�����豸�ţ���8λ�Ǵ��豸�� */
    int		b_wcount;		/* �贫�͵��ֽ��� */
    unsigned char* b_addr;	/* ָ��û�����ƿ�������Ļ��������׵�ַ */
    int		b_blkno;		/* �����߼���� */
    int		b_error;		/* I/O����ʱ��Ϣ */
    int		b_resid;		/* I/O����ʱ��δ���͵�ʣ���ֽ��� */
};

class BufferManager {
public:

    static const int NBUF = 100;            //������ƿ顢������������
    static const int BUFFER_SIZE = 512;     //��������С�� ���ֽ�Ϊ��λ

    BufferManager();
    ~BufferManager();

    Buf* GetBlk(int blkno);         //����һ�黺�棬���ڶ�д�豸�ϵĿ�blkno
    Buf* Bread(int blkno);
    void Bwrite(Buf* bf);
    void Brelse(Buf* bf);

    void Bdwrite(Buf* bf);          //�ӳ�д���̿�
    void Bclear(Buf* bf);           //��ջ���������
    void Bflush();                         //���������ӳ�д�Ļ���ȫ�����������
    void FormatBuffer();                   //��ʽ������Buffer
private:
    Buf* bufferList;//������ƿ����ͷָ��
    Buf nBuffer[NBUF];//������ƿ�����
    unsigned char buffer[NBUF][BUFFER_SIZE];//����������

    DiskDriver* diskDriver;
    std::unordered_map<int, Buf*> bufferMap;
    void InitList();
    //��bpȡ��Ų�����
    void MoveNode(Buf* bf);
    //����һ��bp�����
    void InsertNode(Buf* bf);
};


#endif //OSFILE_BUFFERMANAGER_H
