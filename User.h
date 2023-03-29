//
// Created by roy on 26/03/2023.
//

#ifndef OSFILE_USER_H
#define OSFILE_USER_H

#include "INode.h"
#include "FileManager.h"
#include "OpenFileManager.h"
#include "File.h"

class User
{
public:
    static const int EAX = 0;	/* u.u_ar0[EAX]�������ֳ���������EAX�Ĵ�����ƫ���� */

    /* u_error's Error Code */
    /* 1~32 ����linux ���ں˴����е�/usr/include/asm/errno.h, ����for V6++ */
    enum ErrorCode
    {
        U_NOERROR	= 0,	/* No error */
        U_EPERM	= 1,	/* Operation not permitted */
        U_ENOENT	= 2,	/* No such file or directory */
        U_ESRCH	= 3,	/* No such process */
        U_EINTR	= 4,	/* Interrupted system call */
        U_EIO		= 5,	/* I/O error */
        U_ENXIO	= 6,	/* No such device or address */
        U_E2BIG	= 7,	/* Arg list too long */
        U_ENOEXEC	= 8,	/* Exec format error */
        U_EBADF	= 9,	/* Bad file number */
        U_ECHILD	= 10,	/* No child processes */
        U_EAGAIN	= 11,	/* Try again */
        U_ENOMEM	= 12,	/* Out of memory */
        U_EACCES	= 13,	/* Permission denied */
        U_EFAULT  = 14,	/* Bad address */
        U_ENOTBLK	= 15,	/* Block device required */
        U_EBUSY 	= 16,	/* Device or resource busy */
        U_EEXIST	= 17,	/* File exists */
        U_EXDEV	= 18,	/* Cross-device link */
        U_ENODEV	= 19,	/* No such device */
        U_ENOTDIR	= 20,	/* Not a directory */
        U_EISDIR	= 21,	/* Is a directory */
        U_EINVAL	= 22,	/* Invalid argument */
        U_ENFILE	= 23,	/* File table overflow */
        U_EMFILE	= 24,	/* Too many open files */
        U_ENOTTY	= 25,	/* Not a typewriter(terminal) */
        U_ETXTBSY	= 26,	/* Text file busy */
        U_EFBIG	= 27,	/* File too large */
        U_ENOSPC	= 28,	/* No space left on device */
        U_ESPIPE	= 29,	/* Illegal seek */
        U_EROFS	= 30,	/* Read-only file system */
        U_EMLINK	= 31,	/* Too many links */
        U_EPIPE	= 32,	/* Broken pipe */
        U_ENOSYS	= 100,
        //EFAULT	= 106
    };

    static const int NSIG = 32;	/* �źŸ��� */

    /* p_sig�н��ܵ����źŶ��� */
    static const int SIGNUL = 0;	/* No Signal Received */
    static const int SIGHUP = 1;	/* Hangup (kill controlling terminal) */
    static const int SIGINT = 2;    /* Interrupt from keyboard */
    static const int SIGQUIT = 3;	/* Quit from keyboard */
    static const int SIGILL = 4;	/* Illegal instrution */
    static const int SIGTRAP = 5;	/* Trace trap */
    static const int SIGABRT = 6;	/* use abort() API */
    static const int SIGBUS = 7;	/* Bus error */
    static const int SIGFPE = 8;	/* Floating point exception */
    static const int SIGKILL = 9;	/* Kill(can't be caught or ignored) */
    static const int SIGUSR1 = 10;	/* User defined signal 1 */
    static const int SIGSEGV = 11;	/* Invalid memory segment access */
    static const int SIGUSR2 = 12;	/* User defined signal 2 */
    static const int SIGPIPE = 13;	/* Write on a pipe with no reader, Broken pipe */
    static const int SIGALRM = 14;	/* Alarm clock */
    static const int SIGTERM = 15;	/* Termination */
    static const int SIGSTKFLT = 16; /* Stack fault */
    static const int SIGCHLD = 17; /* Child process has stopped or exited, changed */
    static const int SIGCONT = 18; /* Continue executing, if stopped */
    static const int SIGSTOP = 19; /* Stop executing */
    static const int SIGTSTP = 20; /* Terminal stop signal */
    static const int SIGTTIN = 21; /* Background process trying to read, from TTY */
    static const int SIGTTOU = 22; /* Background process trying to write, to TTY */
    static const int SIGURG = 23;  /* Urgent condition on socket */
    static const int SIGXCPU = 24; /* CPU limit exceeded */
    static const int SIGXFSZ = 25; /* File size limit exceeded */
    static const int SIGVTALRM = 26; /* Virtual alarm clock */
    static const int SIGPROF = 27; /* Profiling alarm clock */
    static const int SIGWINCH = 28; /* Window size change */
    static const int SIGIO = 29; /* I/O now possible */
    static const int SIGPWR = 30; /* Power failure restart */
    static const int SIGSYS = 31; /* invalid sys call */

public:
    unsigned long u_rsav[2];	/* ���ڱ���esp��ebpָ�� */
    unsigned long u_ssav[2];	/* ���ڶ�esp��ebpָ��Ķ��α��� */

    /* ����ӱ������������ԭ�еı���
     * int u_uisa[16]
     * int u_uisd[16]
     * u_tsize
     * u_dsize
     * u_ssize
     */

    /* ϵͳ������س�Ա */
    unsigned int	u_ar0[1];		/* ָ�����ջ�ֳ���������EAX�Ĵ���
								��ŵ�ջ��Ԫ�����ֶδ�Ÿ�ջ��Ԫ�ĵ�ַ��
								��V6��r0���ϵͳ���õķ���ֵ���û�����
								x86ƽ̨��ʹ��EAX��ŷ���ֵ�����u.u_ar0[R0] */

    int u_arg[5];				/* ��ŵ�ǰϵͳ���ò��� */
    char* u_dirp;				/* ϵͳ���ò���(һ������Pathname)��ָ�� */

    /* ʱ����س�Ա */
    int u_utime;		/* �����û�̬ʱ�� */
    int u_stime;		/* ���̺���̬ʱ�� */
    int u_cutime;		/* �ӽ����û�̬ʱ���ܺ� */
    int u_cstime;		/* �ӽ��̺���̬ʱ���ܺ� */

    /* �źŴ�����س�Ա */
    unsigned long u_signal[NSIG];	/* �źŴ���� */
    unsigned long u_qsav[2];		/* ���ڽ��յ��ź�ʱֱ�Ӵ�Sleep()����������Trap() */
    bool u_intflg;		/* ϵͳ�����ڼ��Ƿ��ܵ��źŴ�� ��1��ʾ����ϡ�0��ʾδ�����*/

    /* �ļ�ϵͳ��س�Ա */
    INode* u_cdir;		/* ָ��ǰĿ¼��Inodeָ�� */
    INode* u_pdir;		/* ָ��Ŀ¼��Inodeָ�� */

    DirectoryEntry u_dent;					/* ��ǰĿ¼��Ŀ¼�� */
    char u_dbuf[DirectoryEntry::DIRSIZ];	/* ��ǰ·������ */
    char u_curdir[128];						/* ��ǰ����Ŀ¼����·�� */
    char name[128];

    ErrorCode u_error;			/* ��Ŵ����� */
    int u_segflg;				/* ����I/O����û���ϵͳ�ռ� */

    /* ���̵��û���ʶ */
    short u_uid;		/* ��Ч�û�ID */
    short u_gid;		/* ��Ч��ID */
    short u_ruid;		/* ��ʵ�û�ID */
    short u_rgid;		/* ��ʵ��ID */

    /* �ļ�ϵͳ��س�Ա */
    OpenFiles u_ofiles;		/* ���̴��ļ������������ */

    /* �ļ�I/O���� */
    IOParameter u_IOParam;	/* ��¼��ǰ����д�ļ���ƫ�������û�Ŀ�������ʣ���ֽ������� */

    /* Member Functions */
public:
    /* ��ȡ��ǰ�û�����Ŀ¼ */
    User();
    char* Pwd();
    char* getName();
    void setname(const char*);
};

#endif //OSFILE_USER_H
