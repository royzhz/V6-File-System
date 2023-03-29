//
// Created by roy on 26/03/2023.
//

#include "BufferManager.h"
#include "Kernel.h"

BufferManager::BufferManager() {
    bufferList = new Buf;
    InitList();

    diskDriver = &Kernel::Instance().GetDiskDriver();
}

BufferManager::~BufferManager() {
    Bflush();
    delete bufferList;
}

Buf *BufferManager::GetBlk(int blkno) {
    Buf* pb;
    //�ҵ�
    if (bufferMap.find(blkno) != bufferMap.end()) {
        pb = bufferMap[blkno];
        MoveNode(pb);
        return pb;
    }
    //û�ҵ���������һ���µ�
    pb = bufferList->b_back;
    if (pb == bufferList) {
        //cout << "�޻����ɹ�ʹ��" << endl;
        return NULL;
    }
    //�ŵ����
    MoveNode(pb);
    //ɾ��ԭbuf
    bufferMap.erase(pb->b_blkno);
    //д�ش���
    if (pb->b_flags & Buf::B_DELWRI)
        diskDriver->write(pb->b_addr, BUFFER_SIZE, pb->b_blkno * BUFFER_SIZE);
    //���
    pb->b_flags &= ~(Buf::B_DELWRI | Buf::B_DONE);
    pb->b_blkno = blkno;
    //����map
    bufferMap[blkno] = pb;
    return pb;
}

Buf *BufferManager::Bread(int blkno) {
    Buf* pb = GetBlk(blkno);

    //�ҵ�
    if (pb->b_flags & (Buf::B_DONE | Buf::B_DELWRI))
        return pb;
    //�ڴ�����
    diskDriver->read(pb->b_addr, BUFFER_SIZE, pb->b_blkno * BUFFER_SIZE);
    pb->b_flags |= Buf::B_DONE;
    return pb;
}

void BufferManager::Bwrite(Buf *bf) {
    //ֱ��д��
    bf->b_flags &= ~(Buf::B_DELWRI);
    diskDriver->write(bf->b_addr, BUFFER_SIZE, bf->b_blkno * BUFFER_SIZE);
    bf->b_flags |= (Buf::B_DONE);
    this->Brelse(bf);
}
//�ŵ�LRU���
void BufferManager::Brelse(Buf *bf) {
    InsertNode(bf);
}
//�ӳ�д
void BufferManager::Bdwrite(Buf *bp) {
    bp->b_flags |= (Buf::B_DELWRI | Buf::B_DONE);
    this->Brelse(bp);
    return;
}

void BufferManager::Bclear(Buf *bp) {
    memset(bp->b_addr, 0, BufferManager::BUFFER_SIZE);
    return;
}

void BufferManager::Bflush() {
    Buf* pb = NULL;
    for (int i = 0; i < NBUF; ++i) {
        pb = nBuffer + i;
        if ((pb->b_flags & Buf::B_DELWRI)) {
            pb->b_flags &= ~(Buf::B_DELWRI);
            diskDriver->write(pb->b_addr, BUFFER_SIZE, pb->b_blkno * BUFFER_SIZE);
            pb->b_flags |= (Buf::B_DONE);
        }
    }
}

void BufferManager::FormatBuffer() {
    for (int i = 0; i < NBUF; ++i)
        nBuffer[i]={0,NULL,NULL,NULL,NULL,nullptr,0,NULL,nullptr,0,0};
    InitList();
}

void BufferManager::InitList() {
    for (int i = 0; i < NBUF; ++i) {
        if (i)
            nBuffer[i].b_forw = nBuffer + i - 1;
        else {
            nBuffer[i].b_forw = bufferList;
            bufferList->b_back = nBuffer + i;
        }

        if (i + 1 < NBUF)
            nBuffer[i].b_back = nBuffer + i + 1;
        else {
            nBuffer[i].b_back = bufferList;
            bufferList->b_forw = nBuffer + i;
        }
        nBuffer[i].b_addr = buffer[i];
        nBuffer[i].b_blkno = i;
    }
}

void BufferManager::MoveNode(Buf* bf) {
    if (bf->b_back == NULL)
        return;
    bf->b_forw->b_back = bf->b_back;
    bf->b_back->b_forw = bf->b_forw;
    bf->b_back = NULL;
    bf->b_forw = NULL;
}

void BufferManager::InsertNode(Buf *bf) {
    if (bf->b_back != NULL)
        return;
    bf->b_forw = bufferList->b_forw;
    bf->b_back = bufferList;
    bufferList->b_forw->b_back = bf;
    bufferList->b_forw = bf;
}
