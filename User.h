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
    static const int EAX = 0;	/* u.u_ar0[EAX]；访问现场保护区中EAX寄存器的偏移量 */

    /* u_error's Error Code */
    /* 1~32 来自linux 的内核代码中的/usr/include/asm/errno.h, 其余for V6++ */
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

    static const int NSIG = 32;	/* 信号个数 */

    /* p_sig中接受到的信号定义 */
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
    unsigned long u_rsav[2];	/* 用于保存esp与ebp指针 */
    unsigned long u_ssav[2];	/* 用于对esp和ebp指针的二次保护 */

    /* 新添加变量，用于替代原有的变量
     * int u_uisa[16]
     * int u_uisd[16]
     * u_tsize
     * u_dsize
     * u_ssize
     */

    /* 系统调用相关成员 */
    unsigned int	u_ar0[1];		/* 指向核心栈现场保护区中EAX寄存器
								存放的栈单元，本字段存放该栈单元的地址。
								在V6中r0存放系统调用的返回值给用户程序，
								x86平台上使用EAX存放返回值，替代u.u_ar0[R0] */

    int u_arg[5];				/* 存放当前系统调用参数 */
    char* u_dirp;				/* 系统调用参数(一般用于Pathname)的指针 */

    /* 时间相关成员 */
    int u_utime;		/* 进程用户态时间 */
    int u_stime;		/* 进程核心态时间 */
    int u_cutime;		/* 子进程用户态时间总和 */
    int u_cstime;		/* 子进程核心态时间总和 */

    /* 信号处理相关成员 */
    unsigned long u_signal[NSIG];	/* 信号处理表 */
    unsigned long u_qsav[2];		/* 用于接收到信号时直接从Sleep()函数跳回至Trap() */
    bool u_intflg;		/* 系统调用期间是否受到信号打断 ，1表示被打断、0表示未被打断*/

    /* 文件系统相关成员 */
    INode* u_cdir;		/* 指向当前目录的Inode指针 */
    INode* u_pdir;		/* 指向父目录的Inode指针 */

    DirectoryEntry u_dent;					/* 当前目录的目录项 */
    char u_dbuf[DirectoryEntry::DIRSIZ];	/* 当前路径分量 */
    char u_curdir[128];						/* 当前工作目录完整路径 */
    char name[128];

    ErrorCode u_error;			/* 存放错误码 */
    int u_segflg;				/* 表明I/O针对用户或系统空间 */

    /* 进程的用户标识 */
    short u_uid;		/* 有效用户ID */
    short u_gid;		/* 有效组ID */
    short u_ruid;		/* 真实用户ID */
    short u_rgid;		/* 真实组ID */

    /* 文件系统相关成员 */
    OpenFiles u_ofiles;		/* 进程打开文件描述符表对象 */

    /* 文件I/O操作 */
    IOParameter u_IOParam;	/* 记录当前读、写文件的偏移量，用户目标区域和剩余字节数参数 */

    /* Member Functions */
public:
    /* 获取当前用户工作目录 */
    User();
    char* Pwd();
    char* getName();
    void setname(const char*);
};

#endif //OSFILE_USER_H
