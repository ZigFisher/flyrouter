/* This code was written and is copyrighted 1996,1997,1998 by
 *       Elmar Bartel
 *       Institut fuer Informatik
 *       Technische Universitaet Muenchen
 *       bartel@informatik.tu-muenchen.de
 *
 * Permission to use, copy, modify and distribute this software
 * and its documentation for any purpose, except making money+, is
 * herby granted, provided that the above copyright notice and
 * this permission appears in all places, where this code is
 * referenced or used literally.
 *
 * Pieces and ideas present in this code come from older
 * versions of the program ttcp (Test-TCP), floating around
 * on the internet.
 * I cannot acknowledge the original author, simply because I found
 * no reference. If someone knows more about the history, let me
 * know.
 *
 * +To make it clear:
 *  This statement does not prevent the packaging of this software
 *  and takeing money for the packaging itself. It only means you
 *  cannot make money from the program and its features.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>	/* for inet_ntoa, from where else? */
#include <netdb.h>
#include <syslog.h>

#include <sys/time.h>
#include <sys/resource.h>
#if defined(SunOS53)
#include <sys/rusage.h>
int getrusage(int who, struct rusage *rusage);
#endif /*SunOS53*/

#include "support.h"

#if !defined(TTCP_SERVICE)
#define	TTCP_SERVICE	5003
#endif /*TTCP_SERVICE*/

#if !defined(DEFAULT_BUFCNT)
#define	DEFAULT_BUFCNT	2048
#endif /*DEFAULT_BUFCNT*/

#if !defined(DEFAULT_BUFLEN)
#define	DEFAULT_BUFLEN	4096
#endif /*DEFAULT_BUFLEN*/

#if !defined(NOBODY)
#if defined(hpux) || defined(hpux9) || defined(hpux10)
#if defined(hpux9)
#define	NOBODY 59999
#else
#define	NOBODY 60001
#endif /*hpux9*/
#else
#define	NOBODY (((unsigned)1<<(sizeof(uid_t)*8-1))-1)
#endif /*hpux*/
#endif /*NOBODY*/

#if !defined(NTTCP_LOC_OPT)
#define	NTTCP_LOC_OPT	"NTTCP_LOC_OPT"
#endif /*NTTCP_LOC_OPT*/
#if !defined(NTTCP_REM_OPT)
#define	NTTCP_REM_OPT	"NTTCP_REM_OPT"
#endif /*NTTCP_REM_OPT*/

#if defined(IP_ADD_MEMBERSHIP)
#define MULTICAST
#else
#undef MULTICAST
#endif

/* this is for multicast -- if the OS has support */
#if defined(MULTICAST)
#if !defined(DEFAULT_CHANNEL)
#define	DEFAULT_CHANNEL		"mon.flyrouter.net"
#endif /*DEFAULT_CHANNEL*/
#if !defined(DEFAULT_PORT)
#define	DEFAULT_PORT		5003
#endif /*DEFAULT_PORT*/
#endif /*MULTICAST*/

/* this is solely for debugging
 * normally it destroys the formatted output
 */ 
#define VERBOSE_CONNECT	2

/* architecture specific includes/defines */

/*====================================================================*/
/* what type to used for filedescriptor set's in select */
#define	FdSet	fd_set
#define	SELECT(n, rm, wm, em, to)				\
    select(n, (FdSet *)(rm), (FdSet *)(wm), (FdSet *)(em), to)
#ifndef FD_COPY
#define	FD_COPY(src,dst)  memcpy(dst, src, sizeof(*(dst)))
#endif /*FD_COPY*/


/*====================================================================*/
#if defined(ultrix)
#define openlog(ident, a, b)	openlog(ident, 0)
#endif /*ultrix*/

/*====================================================================*/
#if defined(hpux9)
#include <sys/syscall.h>
#define getrusage(a, b)  syscall(SYS_GETRUSAGE, a, b)
#undef SELECT
#define	SELECT(n, rm, wm, em, to)				\
    select(n, (int *)(rm), (int *)(wm), (int *)(em), to)
void openlog(const char *ident, int logopt, int facility);
void syslog(int priority, const char *message, ...);
#endif /* hpux9 */ 

/*====================================================================*/
#if defined(aix)
#undef  FdSet
#define	FdSet		 void
#endif /* aix */ 

/*====================================================================*/
#if defined(FreeBSD)
#define	SIGCLD	SIGCHLD
#endif /*FreeBSD*/

/*====================================================================*/
#if defined(BSD42)
#define	SetSockopt(s, lv, oname, oval, olen) \
    setsockopt(s, lv, oname, 0, 0)
#else
#define	SetSockopt(s, lv, oname, oval, olen) \
    setsockopt(s, lv, oname, oval, olen)
#endif /*BSD42*/

/*====================================================================*/
#if defined(SunOS4)
#define NOSTRERROR
#endif /*SunOS4*/

#if defined(NOSTRERROR)
extern int sys_nerr;
extern char *sys_errlist[];
char *strerror(int n) {
    static char WrongNumber[32];
    if (0 <= n && n < sys_nerr)
	return sys_errlist[n];
    else {
	sprintf(WrongNumber, "unknown errno: %d\n", n);
	return WrongNumber;
    }
}
#endif /*NOSTRERROR*/

/*====================================================================*/
#if defined(SunOS53)
/* The following #defines address a VERY useful feature
 * the people of SunSoft introduced in SunOS5.3: nanoseconds.
 * They called their struct timespec - and as far as I
 * obseverved, they only use it in struct rusage.
 * The fucking thing: all other call's use the old timeval
 * with microseconds, so nothing is compatible anymore if you
 * do measuring with gettimeofday and getrusage.
 * I resolved this pitty, with using timespec allover
 * and adjusting the results of gettimeofday vi NANO_ADJ.
 *
 *		Happy new Year,
 *		Elmar Bartel, bartel@informatik.tu-muenchen.de
 *						29.12.1994
 *
 * It's getting better all the time: they renamed it to "tv_nsec"
 * but deliver only values from 0..10^6-1  -- or say: I haven't
 * observed other values. Ok, we are prepared, but till now
 * we define SKIPOVER to 1000000.
 * When SunSoft decides to deliver real ns, simply define
 * SKIPOVER to 1000000000.
 *		Despite this, have a happy new Year,
 *		Elmar Bartel, bartel@informatik.tu-muenchen.de
 *						30.12.1994
 */
#define TIMEVAL			timespec
#define	SKIPOVER		1000000
#define FRACT			tv_nsec
#define FRACT_TO_SEC(tv)	(((double)tv.tv_nsec)/SKIPOVER)
#define FRACT_TO_DECI(tv)	(tv.tv_nsec/(SKIPOVER/10))
#define FRACT_TO_CENT(tv)	(tv.tv_nsec/(SKIPOVER/100))
#define FRACT_TO_MILL(tv)	(tv.tv_nsec/(SKIPOVER/1000))
#define	NANO_ADJ(tv)		(tv).tv_nsec*= (SKIPOVER/1000000)
#else /*SunOS53*/
#define TIMEVAL			timeval
#define	SKIPOVER		1000000
#define FRACT			tv_usec
#define FRACT_TO_SEC(tv)	(((double)tv.tv_usec)/SKIPOVER)
#define FRACT_TO_DECI(tv)	(tv.tv_usec/(SKIPOVER/10))
#define FRACT_TO_CENT(tv)	(tv.tv_usec/(SKIPOVER/100))
#define FRACT_TO_MILL(tv)	(tv.tv_usec/(SKIPOVER/1000))
#define	NANO_ADJ(tv)
#endif /*SunOS53*/
#define	TimeVal			struct TIMEVAL

/*====================================================================*/
#if !defined(RUSAGE_SELF)
/* definitions for systems without getrusage(2) calls, but times(2):
 *    + SystemV 4.01 from AT&T
 *    +	SunOS2.[45] (SunOS2.3 has it!!)
 */
#include <sys/times.h>
#define	RUSAGE_SELF	0
#define	RUSAGE_CHILDREN	1
struct rusage {
    struct timeval ru_utime;/* user time used */
    struct timeval ru_stime;/* system time used */
    int ru_maxrss;   /* maximum resident set size */
    int ru_ixrss;    /* currently 0 */
    int ru_idrss;    /* integral resident set size */
    int ru_isrss;    /* currently 0 */
    int ru_minflt;   /* page faults not requiring physical I/O */
    int ru_majflt;   /* page faults requiring physical I/O */
    int ru_nswap;    /* swaps */
    int ru_inblock;  /* block input operations */
    int ru_oublock;  /* block output operations */
    int ru_msgsnd;   /* messages sent */
    int ru_msgrcv;   /* messages received */
    int ru_nsignals; /* signals received */
    int ru_nvcsw;    /* voluntary context switches */
    int ru_nivcsw;   /* involuntary context switches */
};

int getrusage(int who, struct rusage *ru) {
    struct tms tm;
    if (times(&tm) < 0)
    	return -1;
    ru->ru_utime.tv_sec= tm.tms_utime/CLK_TCK;
    ru->ru_utime.tv_usec= (tm.tms_utime%CLK_TCK)*(SKIPOVER/CLK_TCK);
    ru->ru_stime.tv_sec= tm.tms_stime/CLK_TCK;
    ru->ru_stime.tv_usec= (tm.tms_stime%CLK_TCK)*(SKIPOVER/CLK_TCK);
    return 0;
}
#endif /*RUSAGE_SELF*/

#define DEFAULT_FORMAT \
		"%9b%8.2rt%8.2ct%12.4rbr%12.4cbr%8c%10.2rcr%10.1ccr"
		/* see at end of main what this means */

char UsageMessage[] = "\
Usage: nttcp [local options] host [remote options]\n\
       local/remote options are:\n\
\t-t	transmit data (default for local side)\n\
\t-r	receive data\n\
\t-l#	length of bufs written to network (default 4k)\n\
\t-m	use IP/multicasting for transmit (enforces -t -u)\n\
\t-n#	number of source bufs written to network (default 2048)\n\
\t-u	use UDP instead of TCP\n\
\t-g#us	gap in micro seconds between UDP packets (default 0s)\n\
\t-d	set SO_DEBUG in sockopt\n\
\t-D	don't buffer TCP writes (sets TCP_NODELAY socket option)\n\
\t-w#	set the send buffer space to #kilobytes, which is\n\
\t	dependent on the system - default is 16k\n\
\t-T	print title line (default no)\n\
\t-f	give own format of what and how to print\n\
\t-c	compares each received buffer with expected value\n\
\t-s	force stream pattern for UDP transmission\n\
\t-S	give another initialisation for pattern generator\n\
\t-p#   specify another service port\n\
\t-i	behave as if started via inetd\n\
\t-R#	calculate the getpid()/s rate from # getpid() calls\n\
\t-v	more verbose output\n\
\t-V	print version number and exit\n\
\t-?    print this help\n\
\t-N	remote number (internal use only)\n\
\tdefault format is: " DEFAULT_FORMAT "\n";

static char VersionString[] = 
	"nttcp version 1.47  http://www.leo.org/~elmar/nttcp/\n";

/* global variables */
char	MsgBuf[1024];	/* to generate formated messages there
			 * be careful NOT to overwrite 1024 bytes!!
			 */
char	*myname;	/* our name used in prints */
struct sockaddr_in dta_to;	/* the destination where to send
				 * the packets
				 */
int	SysCalls= 0;	 /* counts the number of read/write calls */
int	ReportLimit=100; /* maximum number of failed comparisions to report */
int	Remote= 0;		/* ==1: we are the remote side */
char    *version= "1.47";
struct itimerval	itval;
#define	SetItVal(msec) \
	itval.it_interval.tv_sec= \
	itval.it_interval.tv_usec= 0;\
	itval.it_value.tv_sec=(msec)/1000; \
	itval.it_value.tv_usec=((msec)%1000)*1000; \
	setitimer(ITIMER_REAL, &itval, Nil(struct itimerval))
#define	Suspend(msec)	SetItVal(msec); pause()

struct linger Ling = {
	1,		/* option on */
	1000000		/* linger time, for our control connection */
};

/* global variables needed by StartTimer, StopTimer */
TimeVal time0;		/* Time at which timing started */
struct rusage  ru0;		/* rusage at start */

/* this describes the connection(s) to our remote site(s) */
typedef struct {
    char *HostName;
    char *IPName;
    int	 Socket;
    FILE *fin;
    FILE *fout;
} RemoteConnection;
RemoteConnection Peer[100];
int PeerCount=0;

/* this structure is filled from the commandline options */
typedef struct {
    int PidCalls;		/* do PidCalls to getpid() */
    int	udp;			/* ==1: use UDP instead of TCP */
    int	Compare;		/* compare the received data */
    int	StreamPattern;		/* transmit a pattern stream */
    int	Transmit;  		/* ==1: transmitting, ==0 receiveing */
    int Title;     		/* ==1: print title line */
    int	b_flag;    		/* ==1: use mread(), not used right now */
    int	SockOpt;   		/* Debug-SocketOptions */
    int	NoDelay;   		/* ==1: set TCP_NODELAY socket option */
    int	BufCnt;			/* number of buffers to send */
    int	BufLen;			/* buffer length */
    int FixedDataSize;		/* if>0: calc BufCnt/BufLen from this */
    int Window,	SndWin, RcvWin; /* TCP window sizes */
    int	Verbose;		/* ==1: more diagnostics */
    char *MulticastChannel;	/* we send multicast traffic */
    short MulticastPort;	
    int GapLength;		/* Gap length between packets */
    int inetd;			/* run in inetd mode */
    short Service;		/* the service-port inetd listens*/
    char *Format;		/* the output format */
    char *RemHost;		/* the remote host to connect to */
    char *InitString;		/* the stream init string */
    int RemoteNumber;		/* */
} Options;

Options	opt;			/* the commandline options */

void fMessage(FILE *f, char *s) {
    if (Remote) {
	fprintf(f, "%s-%d: %s", myname, opt.RemoteNumber, s);
    }
    else {
	fprintf(f, "%s-l: %s", myname, s);
    }
    fflush(f);
    return;
}
#define Message(x)	fMessage(stdout, x)

void Exit(char *s, int ret) {
    syslog(LOG_DEBUG, s);
    fMessage(stderr,s);
    exit(ret);
}

void SigCld(int dummy) {
    wait(Nil(int));
    /*fMessage(stderr, "Got SIGCLD\n");*/
    signal(SIGCLD, SigCld);
}

void AlarmNothing(int dummy) {
    /*fMessage(stderr, "Got SIGALRM\n");*/
}
char *AlarmMsg;
void AlarmExit(int dummy) {
    fMessage(stderr, AlarmMsg);
    exit(9);
}

void SigPipe(int dummy) {
    Exit("got SIGPIPE, it seems our remote data socket dissapered.\n", 103);
}

void FetchRemoteMsg(char finCh) {
    int p, pcnt, fdcnt, fdmax;
    FdSet ReadMask, TestMask;
    char prevChar= '\0';
    struct timeval tmo;
    FD_ZERO(&ReadMask);
    tmo.tv_sec= 10;
    tmo.tv_usec= 0;
    pcnt= fdmax=0;
    for (p=0; p<PeerCount; p++) {
    	if (Peer[p].Socket <= 0)
	    continue;
	FD_SET(Peer[p].Socket, &ReadMask);
	pcnt++;
	if (Peer[p].Socket > fdmax)
	    fdmax= Peer[p].Socket;
    }
    while (pcnt > 0) {
	if (opt.Verbose) {
	    char MsgBuf[64];
	    sprintf(MsgBuf, "try to get outstanding messages from %d remote clients\n", pcnt);
	    Message(MsgBuf);
	}
	FD_COPY(&ReadMask, &TestMask);
	fdcnt= SELECT(fdmax+1, &TestMask, 0, 0, &tmo);
	if (fdcnt <= 0) {
	    break;
	}
	for (p=0; p<PeerCount; p++) {
	    if (Peer[p].Socket < 0)
	    	continue;
	    if (FD_ISSET(Peer[p].Socket, &TestMask)) {
	    	int rc;
		rc= read (Peer[p].Socket, MsgBuf, sizeof(MsgBuf)-1);
		if (rc > 0) {
		    MsgBuf[rc]= '\0';
		    fputs(MsgBuf, stdout);
		    if ( /* test for end of this transmission */
		    	   (rc==1 && *MsgBuf == prevChar)
		 	|| (rc>1
			  && MsgBuf[rc-1]==finCh&&MsgBuf[rc-2]==finCh))
		    {
		    	prevChar= MsgBuf[rc-1];
		    	FD_CLR(Peer[p].Socket, &ReadMask);
			pcnt--;
		    }
		}
		else if (rc <= 0) {
		    FD_CLR(Peer[p].Socket, &ReadMask);
		    close(Peer[p].Socket);
		    Peer[p].Socket= -1;
		    pcnt--;
		}
	    }
	}
    }
}

void sysError(FILE *f, char *s) {
    syslog(LOG_DEBUG, "%s: %s, errno=%d\n", s, strerror(errno), errno);
    fMessage(f, s);
    fprintf(f, ": %s, errno=%d\n", strerror(errno), errno);
    fflush(f);
}

void exitError(char *s, int ret) {
    sysError(stderr, s);
    if (!Remote)		/* local */
    	FetchRemoteMsg('\0');
    fflush(stdout);
    exit(ret);
}


void tvAdd(TimeVal *tsum, TimeVal *t1, TimeVal *t0) {
    tsum->tv_sec = t0->tv_sec + t1->tv_sec;
    tsum->FRACT = t0->FRACT + t1->FRACT;
    if (tsum->FRACT > SKIPOVER)
	tsum->tv_sec++, tsum->FRACT -= SKIPOVER;
}

void tvSub(TimeVal *tdiff, TimeVal *t1, TimeVal *t0) {
    tdiff->tv_sec = t1->tv_sec - t0->tv_sec;
    tdiff->FRACT = t1->FRACT - t0->FRACT;
    if (tdiff->FRACT < 0)
	tdiff->tv_sec--, tdiff->FRACT += SKIPOVER;
}

void StartTimer() {
    /* do anything to initialize time measurement. If you have no
     * getrusage or something like this, you may use anything that
     * seems appropriate to you. The only thing you have adhere to,
     * is the interface of StopTimer.
     */
    gettimeofday(&time0, (struct timezone *)0);
    getrusage(RUSAGE_SELF, &ru0);
    NANO_ADJ(&time0);
    SysCalls=0;
}

void StopTimer(double *cput, double *realt) {
    /* delivers in cput:  the amount of cpu time in seconds
     *          in realt: the real time in seconds
     * both values are measured since the last call to StartTimer.
     * If only realtime can be measured, deliver the same value
     * to cput (you know what you are doing - right?)
     */ 
    TimeVal timedol;
    struct rusage ru1;
    TimeVal td, tend, tstart;
    double secs;

    gettimeofday(&timedol, (struct timezone *)0);
    getrusage(RUSAGE_SELF, &ru1);
    NANO_ADJ(timedol);
    /*
    prusage(&ru0, &ru1, &timedol, &time0, line);
    (void)strncpy( str, line, len);
    */

    /* Get real time */
    tvSub(&td, &timedol, &time0);
    secs = td.tv_sec + FRACT_TO_SEC(td);
    if (secs < 0.00001)
	*realt= 0.00001;
    else
	*realt= secs;

    /* Get CPU time (user+sys) */
    tvAdd(&tend, &ru1.ru_utime, &ru1.ru_stime);
    tvAdd(&tstart, &ru0.ru_utime, &ru0.ru_stime);
    tvSub(&td, &tend, &tstart);
    secs = td.tv_sec + FRACT_TO_SEC(td);
    if (secs < 0.00001)
	*cput= 0.00001;
    else
	*cput= secs;
}

double GetPidRate(int cnt) {
    int n= cnt;
    double	cput;	/* cpu time for cnt getpid() calls in s */
    double	realt;	/* real time for cnt getpid() calls in s */
    StartTimer();
    while (--n > 0) {
    	pid_t pid= getpid();
	pid+= 1;	/* to keep compilers quiet*/
    }
    StopTimer(&cput, &realt);
    return ((double)cnt)/cput;
}

/*====================================================================*/
/* random definitions */

static unsigned char rb[250];
static int actp,actq;

#define Advance250		\
	if (++actp >= 250)	\
	    actp=0;		\
	if (++actq >= 250)	\
	    actq=0
#define Value250		\
	rb[actp]^= rb[actq]

unsigned char r250() {
/* based on an article in Dr.Dobb's Journal May 1991 */
/* delivers a cyclic pseudo-random sequence of bytes */
/* the algorithm is based on the primitive polynom
 *         x^250+x^130 mod 2
 * The length of the sequence is 2^250-1 which is about
 * 10^73. This should be long enough.
 * We use here eight initial 250 bit sequences stored in
 * in the array rb, from where we generate the next bits.
 * The initialisatzion of these eight sequences must be
 * done with some care: they must be linear independent.
 * The key supplied to the program is used to initialize
 * the array rb. We set every element of rb four times.
 *					Elmar Bartel
 */
    Advance250;
    return Value250;
}

void streamr250(char *cp, int len) {
    while (len-- > 0) {
    	Advance250;
    	*(unsigned char *)cp++= Value250;
    }
}

void initr250(char *seed) {
    int q,i,w;
    char *p,b;

    memset(rb,0,sizeof(rb));
    p=seed;
    w= 1;
    q= 0;
    /* Generate the seed in the array */
    for (i=0; i<1000; i++) {
	w= (w* *p) % 1009;
	rb[q]= w & 0xff;
	if (++q>=250)
	    q= 0;
	if (!*++p)
	    p=seed;
    }
    /* make it linear independent */
    b= -1;
    while (b) {
	rb[q]|= b;
	b<<=1;
	rb[q]^= b;
	if ((q+=37)>=250)
	    q-= 250;
    }
    actp=0;
    actq=103;	/* This is essential, it defines the primitive
	     	 * polynom
		 */
}
/* end of random */
/*====================================================================*/


int SetTCP_NoDelay(int fd) {
    int one, ret;
    struct protoent *p;
    p = getprotobyname("tcp");
    if (p == Nil(struct protoent))
    	return -1;
    ret= SetSockopt(fd, p->p_proto, TCP_NODELAY, (char *)&one, sizeof(one));
    if (ret < 0)
    	return ret;
    else
    	return 0;
}

/* this is the original version
 * since isprint may be different depending on LOCALE
 * I replaced it.
 *				ElB 1996-02-07 
 
void Pattern(char *cp, int cnt) {
    int c = 0;
    while (cnt-- > 0) {
	while (!isprint((c&0x7F)))
	    c++;
	*cp++ = (c++&0x7F);
    }
}
*/

void Pattern(char *cp, int cnt) {
    streamr250(cp, cnt);
}

void StartPattern(char *InitString) {
    if (InitString)
	initr250(InitString);
    else
	initr250("This is a simple init string");
}

/*
 *			M R E A D
 *
 * This function performs the function of a read(II) but will
 * call read(II) multiple times in order to get the requested
 * number of characters.  This can be necessary because
 * network connections don't deliver data with the same
 * grouping as it is written with.  Written by Robert S. Miles, BRL.
 */
int mread(int fd, char *bufp, unsigned n) {
    unsigned	count = 0;
    int		nread;

    do {
	nread= read(fd, bufp, n-count);
	SysCalls++;
	if (nread < 0)  {
	    perror("ttcp_mread");
	    return(-1);
	}
	if (nread == 0)
	    return (int)count;
	count+= (unsigned)nread;
	bufp+= nread;
    } while (count < n);
    return (int)count;
}

int Nread(int fd, char *buf, int count) {
    register int cnt;
    if (opt.udp) {
	struct sockaddr_in from;
	int len= sizeof(from);
	 cnt= recvfrom(fd, buf, count, 0, (struct sockaddr *)&from, &len);
	 SysCalls++;
    }
    else {
	/*
	if (opt.b_flag)
	    cnt= mread(fd, buf, count);
	else {
	*/
	    cnt= read(fd, buf, count);
	    SysCalls++;
	/*
	}
	*/
    }
    return(cnt);
}

#define TV_DOUBLE(tv) ((tv)->tv_sec + (tv)->tv_usec*1e-6)
void delay(int us) {
    /* This function tries to delay execution for us microseconds.
     * It is called to implement a defined frame-gap. But this is
     * a weak try, since unix is NOT a realtime operating system.
     * First, there is no portable way of doing real microsecond
     * sleeps - the resolution is about 10ms. Everything below exists
     * only if the manufacturer wants to supoort it.
     * Second nobody can assure that the packet we send off after
     * this delay is really sent.
     * For the first point, SunSolaris running on new Sparc hardware
     * (Sparc Station20, Ultra Sparc), can do a real usleep.
     * We try to do it in a most portable way, and try to make
     * it work, when the machine supports timings less than 10ms:
     *   + everything greater than 10ms is done with select.
     *   + the remaining time is wasted with looping through
     *     gettimeofday. If the machine supports resolution lower
     *     than 10ms this will do the delay on expense of CPU-time.
     *     If not, it does no harm, but simply costs one call of
     *	   gettimeofday.
     */
    struct timeval tv;
    long remain;
    double start, sec_delay;

    tv.tv_sec= us/(1000*1000);
    tv.tv_usec= us%(1000*1000);
    /* do it with select only to a resolution of clocktick */
    remain= tv.tv_usec%(10*1000);
    tv.tv_usec= tv.tv_usec - remain;
    if (tv.tv_usec > 0 || tv.tv_sec > 0)
	(void)SELECT(1, 0, 0, 0, &tv);
    /* now try to make a better resolution */
    sec_delay= remain*1e-6;
    gettimeofday(&tv, (struct timezone *)0);
    start= TV_DOUBLE(&tv);
    while (1) {
	struct timeval tv1;
	gettimeofday(&tv1, (struct timezone *)0);
	if (TV_DOUBLE(&tv1) - start >= sec_delay)
	    break;
    }
}
    
int Nwrite(int fd, char *buf, int count) {
    int cnt;
    if (opt.StreamPattern)
	streamr250(buf, count);
    if (opt.udp) {
  again:
	cnt= sendto(fd, buf, count, 0, (struct sockaddr *)&dta_to, sizeof(dta_to));
	SysCalls++;
	if (cnt < 0 && errno == ENOBUFS) {
	    errno = 0;
	    goto again;
	}
    }
    else {
	cnt= write(fd, buf, count);
	SysCalls++;
    }
    if (opt.GapLength) {
	delay(opt.GapLength);
    }
    return(cnt);
}

void Usage() {
    fputs(UsageMessage, stderr);
    fflush(stderr);
    exit(1);
}

void GetSizeValue(int *ac, char **av[], int *val, int Limit, char *what) {
    char *arg;
    int argc= *ac;
    char **argv= *av;
    if (argv[0][2] == '\0') {
	argc--, argv++;
	arg= argv[0];
    }
    else
	arg= &argv[0][2];
    *ac= argc;
    *av= argv;
    if (arg == NULL) {
	sprintf(MsgBuf, "missing argument value for %s\n", what);
	fMessage(stderr, MsgBuf);
	Usage();
    }
    if ((*val=atoi(arg)) <= 0 || *val >= Limit) {
	sprintf(MsgBuf, "invalid value for %s (%.30s)\n", what, arg);
	fMessage(stderr, MsgBuf);
	Usage();
    }
}

char* EightBits(char *bits, char value) {
    char bit=1;
    int i=sizeof(value)*8;
    bits[i]= '\0';
    do {
    	bits[--i]= (value&bit ? '1' : '0');
    } while (bit<<=1);
    return bits;
}

int BufCompare(char *cp, char *expect, int leng, unsigned long nBytes, int BufLen, int *Reported) {
    int Failed=0;
    int n,ne;
    /*ne= nBytes%BufLen;*/
    ne=0;
    for (n=0; n<leng; n++) {
	if (cp[n] != expect[ne]) {
	    Failed++;
	    if (*Reported == 0) {
		sprintf(MsgBuf, "Here the list of at most %d failed comparisions:\n", ReportLimit);
		Message(MsgBuf);
		sprintf(MsgBuf, "%-9s%-10s%-10s\n", "byte#", "expected", "received");
		Message(MsgBuf);
	    }
	    if (*Reported < ReportLimit) {
	    	char rBits[9], eBits[9];
		(*Reported)++;
		sprintf(MsgBuf,
		  "%9ld%10s%10s\n",
		  nBytes+n, EightBits(eBits,expect[n]), EightBits(rBits, cp[n]));
		Message(MsgBuf);
	    }
	}
	/*
	if (++ne >= BufLen)
	    ne=0;
	*/
	ne++;
    }
    return Failed;
}

short GetService() {
    struct servent *ttcpService;
    ttcpService= getservbyname("ttcp", "tcp");
    if (ttcpService == Nil(struct servent))
	return TTCP_SERVICE;
    else
	return ntohs(ttcpService->s_port);
}

void InitOptions(Options *opt) {
    /*memset(opt, 0, sizeof(Options);*/
    opt->PidCalls= 0;		/* do PidCalls to getpid() */
    opt->udp= 0;  		/* ==1: use UDP instead of TCP */
    opt->Compare= 0;		/* ==1: compare the received data */
    opt->StreamPattern=0;	/* transmit a pattern stream */
    opt->Title=0;     		/* ==1: print title line */
    opt->b_flag=0;    		/* ==1: use mread(), not used right now */
    opt->SockOpt=0;   		/* Debug-SocketOptions */
    opt->NoDelay=0;   		/* ==1: set TCP_NODELAY socket option */
    opt->BufCnt=DEFAULT_BUFCNT;	/* number of buffers to send */
    opt->BufLen=DEFAULT_BUFLEN;	/* buffer length */
    opt->FixedDataSize=0;	/* if>0: calc BufCnt/BufLen from this */
    opt->Window=0;		/* ==1: set the TCP window size */
    opt->Verbose=0;		/* ==1: more diagnostics */
    opt->GapLength=0;		/* no delay, send as fast as possible */
    opt->inetd= 0;		/* ==1: run in inetd mode */
    opt->Service= GetService();	/* the service-port to listen to */
    opt->MulticastChannel= NULL;
    opt->MulticastPort= 0;
    opt->Transmit=1;  		/* ==1: transmitting, ==0 receiveing */
    opt->Format= DEFAULT_FORMAT;
    opt->InitString= Nil(char);
    return;
}

void ParseOptions(int *ac, char **av[], Options *opt) {
    int argc= *ac;
    char **argv= *av;
    argv++; argc--;
    while (argc>0 && argv[0][0] == '-')  {
	switch (argv[0][1]) {
	  case 't':
	    opt->Transmit= 1;
	    break;
	  case 'r':
	    opt->Transmit= 0;
	    break;
	  case 'T':
	    opt->Title = 1;
	    break;
	  /*
	  case 'B':
	    opt->b_flag = 1;
	    break;
	  */
	  case 'd':
	    opt->SockOpt|= SO_DEBUG;
	    break;
	  case 'D':
	    opt->NoDelay = 1;
	    break;
	  case 'f':
	    if (argv[0][2])
	    	opt->Format= &argv[0][2];
	    else {
	    	argv++; argc--;
	    	opt->Format= argv[0];
	    }
	    break;
	  case 'g':
	    GetSizeValue(&argc, &argv, &opt->GapLength, 10000000,
	      "UDP gap-length");
	    break;
	  case 'l':
	    GetSizeValue(&argc, &argv, &opt->BufLen, 100000000,
	      "bufferlength");
	    if (opt->FixedDataSize > 0 && opt->BufLen > 0)
	    	opt->BufCnt= opt->FixedDataSize / opt->BufLen;
	    break;
	  case 'n': 
	    GetSizeValue(&argc, &argv, &opt->BufCnt, 1000000000,
	      "buffercount");
	    if (opt->FixedDataSize > 0 && opt->BufCnt > 0)
	    	opt->BufLen= opt->FixedDataSize / opt->BufCnt;
	    break;
	  case 'x': 
	    GetSizeValue(&argc, &argv, &opt->FixedDataSize, 1000000000,
	      "fixed data size");
	    break;
	  case 'N': 
	    GetSizeValue(&argc, &argv, &opt->RemoteNumber, 10,
	      "RemoteNumber");
	    break;
	  case 'w':
	    GetSizeValue(&argc, &argv, &opt->SndWin, 10000,
	      "windowsize");
	    opt->RcvWin = opt->SndWin= opt->SndWin*1024;
	    opt->Window=1;			
	    break;
	  case 'u':
	    opt->udp = 1;
	    break;
	  case 'v':
	    opt->Verbose = 1;
	    break;
	  case 'V':
	    fputs(VersionString, stdout);
	    exit(0);
	  case '?':
	    fputs(VersionString, stdout);
	    fputs(UsageMessage, stdout);
	    exit(0);
	  case 's':
	    opt->StreamPattern= 1;
	    break;
	  case 'c':
	    opt->Compare= 1;
	    break;
	  case 'p': {
		int val;
		GetSizeValue(&argc, &argv, &val, 256*256, "service port");
		opt->Service= (short)val;
	    }
	    break;
	  case 'R':
	    GetSizeValue(&argc, &argv, &opt->PidCalls, 2000000000,
	      "pid calls");
	    break;
	  case 'S':
	    if (argv[0][2])
	    	opt->InitString= &argv[0][2];
	    else {
	    	argv++; argc--;
	    	opt->InitString= argv[0];
	    }
	    break;
	  case 'i':
	    opt->inetd= 1;
	    break;
	  case 'm': {
#if defined(MULTICAST)
	    char *p;
	    if (argv[0][2])
	    	opt->MulticastChannel= &argv[0][2];
	    else {
	    	argv++; argc--;
	    	opt->MulticastChannel= argv[0];
	    }
	    if ( (p=strchr(opt->MulticastChannel, ':')) ) {
	    	*p= '\0';
		opt->MulticastPort= atoi(p+1);
	    }
	    else
		opt->MulticastPort= DEFAULT_PORT;
#else /*MULTICAST*/
	    strcpy(MsgBuf, "need -DMULTICAST when compile, to use MULTICAST");
	    fMessage(stderr, MsgBuf);
	    Usage();
#endif /*MULTICAST*/
	    break;
	  }
	  default: {
	    strcpy(MsgBuf, "unknown option: ");
	    strncat(MsgBuf, argv[0], sizeof(MsgBuf)-strlen(MsgBuf)-1);
	    fMessage(stderr, MsgBuf);
	    Usage();
	  }
	}
	argv++; argc--;
    }
    /*
    opt->RemHost= argv[0];
    */
    *ac= argc;
    *av= argv;
    return;
}

int MakeArgvFromEnv(char *EnvName, char *argv[], int start, int Max) {
    /* fill argv at index start with the parts of argv
     * return the number of elements filled, or -1 on failure.
     */
    char *env= getenv(EnvName);
    if (env != Nil(char)) {
	int  ac= start;
	char *tok, *space= " \t\n";
    	char *nenv= strdup(env);
	if (nenv == Nil(char)) {
	    return -1;
	}
	tok= strtok(nenv, space);
	while (tok != Nil(char)) {
	    argv[ac++]= tok;
	    if (ac ==  Max-1)
	    	break; /* silently stop parsing */
	    tok= strtok(Nil(char), space);
	}
	argv[ac]= NULL;
	return ac-start;
    }
    else {
	return 0;
    }
}

void ParseOptionsFromEnv(char *EnvName, Options *opt) {
    int ac, argc;

    /* parse options from the environment
     * only options will be looked at. Further
     * things will be simply ignored.
     * At most 50 options are parsed
     */
    char **av_scratch, **argv= (char **)malloc(50*sizeof(char *));
    if (argv == Nil(char *))
    	return;

    argc= 1;
    ac= MakeArgvFromEnv(EnvName, argv, argc, 50);
    if (ac < 0) {
    	free(argv);
    	return;
    }
    argc= ac+1;
    av_scratch= argv;
    ParseOptions(&argc, &av_scratch, opt);
    free(argv);
    return;
}

int createControlSocket(char *RemHost) {
    int rh;
    struct sockaddr_in sinrh;	/* for control socket to remote host */
    struct sockaddr_in sinlh;	/* for control socket on local host */
    memset(&sinrh, 0, sizeof(sinrh));
    if (atoi(RemHost) > 0) {
	sinrh.sin_family = AF_INET;
	sinrh.sin_addr.s_addr = inet_addr(RemHost);
	Peer[PeerCount].IPName= RemHost;
	Peer[PeerCount].HostName= RemHost;
    }
    else {
	struct hostent *addr;
	if ((addr=gethostbyname(RemHost)) == NULL) {
	    strcpy(MsgBuf, "bad hostname: ");
	    strncat(MsgBuf, RemHost, sizeof(MsgBuf)-strlen(MsgBuf)-1);
	    sysError(stderr, MsgBuf);
	    return -6;
	}
	sinrh.sin_family = addr->h_addrtype;
	memcpy(&sinrh.sin_addr.s_addr, addr->h_addr, addr->h_length);
	Peer[PeerCount].IPName= strdup(inet_ntoa(sinrh.sin_addr));
	Peer[PeerCount].HostName= RemHost;
    }
    sinrh.sin_port = htons(opt.Service);
    sinrh.sin_family = AF_INET; 

    if ((rh = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	sysError(stderr, "control-socket");
	return -7;
    }
    Peer[PeerCount].Socket= rh;

    memset(&sinlh, 0, sizeof(sinlh));
    sinlh.sin_family = AF_INET;
    sinlh.sin_port = 0;	      /* our side has the free choice */
    if (bind(rh, (struct sockaddr *)&sinlh, sizeof(sinlh)) < 0) {
	sysError(stderr, "bind");
	close(rh);
	return -8;
    }

    if (connect(rh, (struct sockaddr *)&sinrh, sizeof(sinrh)) < 0) {
	sysError(stderr, "connect");
	close(rh);
	return -9;
    }
	
    if (SetTCP_NoDelay(rh) < 0) {
	sysError(stderr, "cannot set TCP_NODELAY on protocol socket");
	close(rh);
	return -10;
    }
    Peer[PeerCount].Socket= rh;
    PeerCount++;
    return 0;
}

void stripNewLine(char *s) {
    int l= strlen(s)-1;
    if (0 <= l && s[l] == '\n')
	s[l]= '\0';
    return;
}

int main(int argc, char *argv[]) {

    struct sockaddr_in PeerAddr;
    int  PeerAddrLeng;
    char *DataPortFormat= "dataport: %d\n";
    int DataPort;
    struct sockaddr_in sinlh;	/* for control socket on local host */
    int  fd;	     		/* data socket to transport the data */
    char *buf;	     		/* the buffer to read from, to write to */
    char *ExpectBuf= 0;		 /* when comparing the data */
    unsigned long nBytes;	/* the amount of transfered/received bytes */
    unsigned long nBuffer;	/* the number of buffers transferd/received */
    double	cput;		/* cpu time for transmission in s */
    double	realt;		/* real time for transmission in s */

    if ((myname=strrchr(argv[0], '/')) != Nil(char))
    	myname++;
    else
    	myname= argv[0];

    if (getuid() == 0 || geteuid() == 0) {
    	if (setuid(NOBODY) != 0) {
	    sprintf(MsgBuf, "cannot setuid(%d)\n", NOBODY);
	    Message(MsgBuf);
	    exit(1);
	}
    }

    InitOptions(&opt);
    ParseOptionsFromEnv(NTTCP_LOC_OPT, &opt);
    ParseOptions(&argc, &argv, &opt);
    if (opt.Verbose) {
	sprintf(MsgBuf, "%s, version %s\n", myname, version);
	Message(MsgBuf);
    } 
    if (opt.inetd) {
    	/* we simulate inetd behaviour */
	int nsrv, srv, fromleng;
	struct sockaddr_in sinsrv;
	struct sockaddr_in frominet;
	if (opt.Verbose) {
	    sprintf(MsgBuf, "running in inetd mode on port %d - "
	      "ignoring options beside -v and -p\n", opt.Service);
	    Message(MsgBuf);
	}
	if ((srv = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	    exitError("service-socket: socket", 99);
	memset(&sinsrv, 0, sizeof(sinsrv));
	sinsrv.sin_family = AF_INET;
	sinsrv.sin_port = htons(opt.Service);
	if (bind(srv, (struct sockaddr *)&sinsrv, sizeof(sinsrv)) < 0)
	    exitError("service-socket: bind:", 101);
	if (listen(srv, 2) < 0)
	    exitError("service-socket", 103);
	signal(SIGCLD, SigCld);
	while (1) {
	    int FailCnt;
	    pid_t pid;
	    fromleng = sizeof(frominet);
	    memset(&frominet, 0, fromleng);
	    while ((nsrv=accept(srv, (struct sockaddr *)&frominet, &fromleng)) < 0)
	    	if (errno == EINTR)
		    continue;
		else
		    break;
	    if (nsrv < 0)
		exitError("service-accept", 115);
	    FailCnt=0;
	    while ((pid=fork()) < 0) {
	    	if (errno == EAGAIN && ++FailCnt < 100) {
		    sprintf(MsgBuf, "fork failed for try #%d\n", FailCnt);
		    Message(MsgBuf);
		    signal(SIGALRM, AlarmNothing);
		    Suspend(200);
		    continue;
		}
		else
		    break;
	    }
	    if (pid < 0)
		exitError("service-fork", 116);
	    if (pid == 0) { /* child */
	    	close(0);
		if (dup2(nsrv, 0) < 0)
		    exitError("service-dup2(0)", 117);
	    	close(1);
		if (dup2(nsrv, 1) < 0)
		    exitError("service-dup2(1)", 118);
	    	close(2);
		if (dup2(nsrv, 2) < 0)
		    exitError("service-dup2(2)", 119);
	    	break;
	    }
	    if (opt.Verbose) {
	    	sprintf(MsgBuf,
		  "forked child (pid=%d) on socket-fd="
		  "fd=%d after %d retries\n",
		   (int)pid, nsrv, FailCnt);
		Message(MsgBuf);
	    }
	    close(nsrv);
	}
    }

    PeerAddrLeng= sizeof(PeerAddr);
    if (getpeername(0, (struct sockaddr *)&PeerAddr, &PeerAddrLeng) == 0) {
    	/* assume we are the remote side, ie not started via commandline */
    	StrVec *OptionArg;
	struct hostent *PeerHost;
    	char OptionLine[1024];
	Remote= 1;
	Peer[PeerCount].IPName= strdup(inet_ntoa(PeerAddr.sin_addr));

	PeerHost=gethostbyaddr((char *)&PeerAddr.sin_addr.s_addr,
	  sizeof(PeerAddr.sin_addr.s_addr), PeerAddr.sin_family);
	openlog(myname, LOG_CONS, LOG_USER);
	if (PeerHost)
	    Peer[PeerCount].HostName= strdup(PeerHost->h_name);
	else
	    Peer[PeerCount].HostName= "?";
	syslog(LOG_INFO, "call from %.50s (=%.30s)\n",
	  Peer[PeerCount].HostName, Peer[PeerCount].IPName);

	if (SetSockopt(0, SOL_SOCKET, SO_LINGER, (char *)&Ling, sizeof(Ling))<0)
	    exitError("setsockopt-linger", 2);
	if (SetSockopt(1, SOL_SOCKET, SO_LINGER, (char *)&Ling, sizeof(Ling))<0)
	    exitError("setsockopt-linger", 2);
	
	/* don't buffer writes to our client */
	if (SetTCP_NoDelay(1) < 0)
	    exitError("cannot set TCP_NODELAY on protocol socket", 3);
	
	OptionLine[sizeof(OptionLine)-1]= '\0';
	if (fgets(OptionLine, sizeof(OptionLine), stdin) == Nil(char)) {
	    sprintf(MsgBuf, "%s: fgets: cannot read stdin\n", myname);
	    Exit(MsgBuf, 2);
	}
	if (OptionLine[sizeof(OptionLine)-1] != '\0') {
	    sprintf(MsgBuf, "%s: optionline longer than %d\n",
	      myname, sizeof(OptionLine)-1);
	    Exit(MsgBuf, 3);
	}
	OptionLine[strlen(OptionLine)-1]= '\0'; /* remove trainling newline */
	    
	OptionArg= StrVecFromString(OptionLine, '@');
	argv= OptionArg->String;
	argc= OptionArg->Leng;
	myname= argv[0];
	InitOptions(&opt);
	ParseOptions(&argc, &argv, &opt);
	if (opt.Verbose) {
	    sprintf(MsgBuf, "Pid=%d, InetPeer= %.30s\n",
	      (int)getpid(), Peer[PeerCount].IPName);
	    Message(MsgBuf);
	    sprintf(MsgBuf,"Optionline=\"%.201s\"\n", OptionLine);
	    Message(MsgBuf);
	}
    	Peer[PeerCount].Socket= 0;
    	Peer[PeerCount].fin= stdin;
    	Peer[PeerCount].fout= stdout;
	syslog(LOG_DEBUG,
	  "call from %.50 (=%.30s): done remote initial processing\n",
	  Peer[PeerCount].HostName, Peer[PeerCount].IPName);
	PeerCount++;
    }
    else { /*local*/
    	if (opt.Verbose) { 
	    sprintf(MsgBuf, "Pid=%d\n", (int)getpid());
	    Message(MsgBuf);
	}
#if !defined(SunOS53) && !defined(SunOS54)
    	if (errno != ENOTSOCK) {
	    fprintf(stderr, "%s: getpeername %s\n",
	      myname, strerror(errno));
	    exit(4);
	}
#endif /*SunOS53,SunOS54*/
    }

    if (opt.InitString || opt.StreamPattern)
    	opt.Compare= 1;
    if (!opt.udp && opt.Compare)
    	opt.StreamPattern= 1;

    StartPattern(opt.InitString);
    
    /* if there is are remaining options, we expect the first to be
     * the name of the host to connect to, and all further options
     * are passed to this host
     */

    if (!Remote) {	/* all the local processing needed to
    			 * set up data for the remote part
			 */
	int i, p;
	StrVec *RemOpt;
	char *RemOptStr, *RemNum;
	int RemOptLeng;
	char OptBuf[64];

	if (argc <= 0) {
	    fMessage(stderr, "don't know where to connect to\n");
	    Usage();
	}
	i= 0;
	while (i<argc && argv[i][0] != '-') {
	    /* take this as a remote hostname */
	    createControlSocket(argv[i]);
	    i++;
	}
	if (PeerCount == 0)
	    exitError("Could not connect to remote host\n", 10);
	if (PeerCount > 1) {
#if defined(MULTICAST)
	    /* assmume we want multicasting, even if not specified */
	    if (opt.MulticastChannel == NULL) {
		opt.MulticastChannel= DEFAULT_CHANNEL;
		opt.MulticastPort   = DEFAULT_PORT;
	    }
	    opt.Transmit= 1;
	    opt.udp= 1;
#else /*MULTICAST*/
	    sprintf(MsgBuf, "cannot send to multiple hosts, "
	      "use only the first one: %.50s (=%.30s)\n",
	      Peer[0].HostName, Peer[0].IPName);
	    Message(MsgBuf);
	    PeerCount= 1;
#endif /*MULTICAST*/
	}

	RemOpt= StrVecCreate(10);
	StrVecAppend(RemOpt, myname);
	if (opt.Transmit)
	    StrVecAppend(RemOpt, "-r");
	else {
	    StrVecAppend(RemOpt, "-t");
	    if (opt.NoDelay)
		    StrVecAppend(RemOpt, "-D");
	}
	{
	    /* append option for remote side from environment */
	    char **av= (char **)malloc(50*sizeof(char *));
	    if (argv != Nil(char *)) {
		int ac= MakeArgvFromEnv(NTTCP_REM_OPT, av, 0, 50);
		StrVec *sv= StrVecFromArgv(ac, av);
		if (sv != Nil(StrVec)) {
		    StrVecJoin(RemOpt, sv);
		    StrVecDestroy(sv);
		}
		free(av);
	    }
	}
	sprintf(OptBuf, "-l%d", opt.BufLen);
	StrVecAppend(RemOpt, OptBuf);
	sprintf(OptBuf, "-n%d", opt.BufCnt);
	StrVecAppend(RemOpt, OptBuf);
	if (opt.udp)
	    StrVecAppend(RemOpt, "-u");
	if (opt.Verbose)
	    StrVecAppend(RemOpt, "-v");
	if (opt.Compare)
	    StrVecAppend(RemOpt, "-c");
	if (opt.StreamPattern)
	    StrVecAppend(RemOpt, "-s");
	if (opt.Format) {
	    StrVecAppend(RemOpt, "-f");
	    StrVecAppend(RemOpt, opt.Format);
	}
	if (opt.InitString) {
	    StrVecAppend(RemOpt, "-S");
	    StrVecAppend(RemOpt, opt.InitString);
	}
	if (opt.PidCalls) {
	    sprintf(OptBuf, "-R%d", opt.PidCalls);
	    StrVecAppend(RemOpt, OptBuf);
	}
	if (opt.GapLength > 0) {
	    sprintf(OptBuf, "-g%d", opt.GapLength);
	    StrVecAppend(RemOpt, OptBuf);
	}
	if (opt.MulticastChannel) {
	    sprintf(OptBuf, "-m%s:%d",
	      opt.MulticastChannel, opt.MulticastPort);
	    StrVecAppend(RemOpt, OptBuf);
	}
	StrVecAppend(RemOpt, "-Nx");
	argv++; argc--;
	if (argc > 0) {
	    StrVec *MoreOpt= StrVecFromArgv(argc, argv);
	    StrVecJoin(RemOpt, MoreOpt);
	    StrVecDestroy(MoreOpt);
	}
	RemOptStr= StrVecToString(RemOpt, '@');
	RemOptLeng= strlen(RemOptStr);
	RemOptStr[RemOptLeng++]= '\n';
	RemOptStr[RemOptLeng]= '\0';

	RemNum= strstr(RemOptStr, "-Nx") + 2;

	for (p=0; p<PeerCount; p++) {
	    int rs= Peer[p].Socket;
	    *RemNum= '1'+p;
	    if (write(rs, RemOptStr, RemOptLeng) != RemOptLeng) {
	    	sprintf(MsgBuf, "cannot write options to peer \"%.50s\"=\"%.30s\"\n",
		  Peer[p].HostName, Peer[p].IPName);
		exitError(MsgBuf, 11);
	    }
	}
    }

    /* Here we are - either local or remote - all our options parsed.
     * Now create buffers and the data sockets.
     * The RECEIVE side will choose a free port and listen on it.
     */
    if( (buf = (char *)malloc(opt.BufLen)) == (char *)NULL) {
	sprintf(MsgBuf, "malloc failed for %d bytes (snd/rcv buffer)\n", opt.BufLen);
	Exit(MsgBuf, 12);
    }
    if (opt.Compare) {
	if ((ExpectBuf = (char *)malloc(opt.BufLen)) == (char *)NULL) {
	    sprintf(MsgBuf, "malloc failed for %d bytes (ExpectBuf)\n", opt.BufLen);
	    Exit(MsgBuf, 13);
	}
    }
    if ((fd = socket(AF_INET, opt.udp?SOCK_DGRAM:SOCK_STREAM, 0)) < 0)
	exitError("data-socket", 13);

    if (Remote) {
    	Peer[0].fin= stdin;
    	Peer[0].fout= stdout;
	/*
    	rhin= stdin;
    	rhout= stdout;
	*/
    }
    else { /*Local*/
    	int p;
	for (p=0; p<PeerCount; p++) {
	    Peer[p].fin= fdopen(Peer[p].Socket, "r");
	    if (Peer[p].fin == NULL) 
	    	exitError("fdopen", 21);
	    Peer[p].fout= fdopen(Peer[p].Socket, "w");
	    if (Peer[p].fout == NULL) 
	    	exitError("fdopen", 21);
	}
	/*
	rhin= Peer[0].fin;
	rhout= Peer[0].fout;
	*/
    }

    /* protocol for exchange of connection parameter:
     * Version 1.3: the receiving side sends via the control
     * 		    connection the port it is listening on.
     *              to the sending side. The sending side
     *		    will bind its data socket to this port
     *		    number and starts sending.
     *
     * Version 1.4:
     *	we want to support multicast. The multicast group
     *  and the port are specified to the -m option in
     *	usual form:    "multicast-IP:port".
     *	Each (or the only) receiving side will transmit
     *	its portnumber to confirm it could open the required
     *	socket. This is in conformance to the previous
     *  protocol. If a receiving side could not aquire
     *	such a port, it will send -1 as port number.
     */

    if (opt.Transmit) {
    	char LineBuf[256];
	int p;
	for (p=0; p<PeerCount; p++) {
	    DataPort= 0;
	    while (fgets(LineBuf, sizeof(LineBuf), Peer[p].fin) != NULL) {
		if (opt.Verbose) {
		    stripNewLine(LineBuf);
		    sprintf(MsgBuf,
		      "from %s: \"%.50s\" (=%.30s)\n",
		      Peer[p].HostName, Peer[p].IPName, LineBuf);
		    Message(MsgBuf);
		}
		sscanf(LineBuf, DataPortFormat, &DataPort);
		if (DataPort > 0)
		    break;
	    }
#if defined(MULTICAST)
	    if (opt.MulticastChannel && DataPort != DEFAULT_PORT) {
		sprintf(MsgBuf, "receiving side %.50s (=%.30s) "
		  "couldn't get multicast socket",
		  Peer[p].HostName, Peer[p].IPName);
		Exit(MsgBuf, 14);
	    }
#endif /*MULTICAST*/
	    if (DataPort == 0) {
		sprintf(MsgBuf,
		  "couldn't get dataport from receiving side "
		  "\"%.50s\" (=%.30s)",
		  Peer[p].HostName, Peer[p].IPName);
		Exit(MsgBuf, 15);
	    }
	}
/*syslog(LOG_DEBUG, "call from %.50s (=%.30s): setup dataport\n", Peer[0].HostName, Peer[0].IPName);*/
	memset(&dta_to, 0, sizeof(dta_to));
	dta_to.sin_family= AF_INET;
	if (opt.MulticastChannel) {
	    dta_to.sin_addr.s_addr= inet_addr(opt.MulticastChannel); 
	    dta_to.sin_port= htons(opt.MulticastPort);
	}
	else {
	    dta_to.sin_addr.s_addr= inet_addr(Peer[0].IPName); 
	    dta_to.sin_port= htons(DataPort);
	}
	if (opt.udp) {
#if defined(MULTICAST)
	    if (opt.MulticastChannel) {
		char loop;
		/*
		u_char ttl;
		 * set onother ttl for multicast (default==1)
		 *  TTL   0 are restricted to the same host
		 *  TTL   1 are restricted to the same subnet
		 *  TTL  32 are restricted to the same site
		 *  TTL  64 are restricted to the same region
		 *  TTL 128 are restricted to the same continent
		 *  TTL 255 are unrestricted in scope.
		if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
		    exitError("cannot set multicast TTL, 15);
		}
		 */
		/* we do not want to get our packets looped back */
		loop= 0;
		if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)) < 0) {
		    exitError("cannot diable multicast loopback", 15);
		}
	    }
#endif /*MULTICAST*/
	}
	else { /* == TCP */
	    memset(&sinlh, 0, sizeof(sinlh));
	    sinlh.sin_family = AF_INET;
	    sinlh.sin_port = 0;	      /* our side has the free choice */

	    if (bind(fd, (struct sockaddr *)&sinlh, sizeof(sinlh)) < 0)
		exitError("bind-dta", 14);

	    if (opt.Verbose & VERBOSE_CONNECT) {
		sprintf(MsgBuf, "waiting for connect\n");
		Message(MsgBuf);
	    }
	    if (connect(fd, (struct sockaddr *)&dta_to, sizeof(dta_to)) < 0) {
	    	sprintf(MsgBuf, "connect-dta: fd=%d, sin_port=%d, s_addr=%s",
		  fd, ntohs(dta_to.sin_port), inet_ntoa(dta_to.sin_addr)); 
		exitError(MsgBuf, 15);
	    }
	    if (opt.Verbose & VERBOSE_CONNECT) {
		sprintf(MsgBuf, "connected !\n");
		Message(MsgBuf);
	    }
	    if (opt.NoDelay) {
		if (SetTCP_NoDelay(fd) < 0)
		    exitError("setsockopt: nodelay", 16);
	    }
	}
    }
    else { /*Receive*/
	memset(&sinlh, 0, sizeof(sinlh));
	sinlh.sin_family = AF_INET;
#if defined(MULTICAST)
	if (opt.MulticastChannel) {
	    int ml, p, join_group;
	    struct ip_mreq mreq;
	    sinlh.sin_port = htons(opt.MulticastPort);
	    if (bind(fd, (struct sockaddr *)&sinlh, sizeof(sinlh)) < 0) {
		char Message[256];
		/*fprintf(Peer[0].fout, DataPortFormat, -1);*/
		fprintf(Peer[0].fout, DataPortFormat, -1);
		sprintf(Message,
		  "cannot bind to fixed multicast port %d\n",
		  ntohs(sinlh.sin_port));
		exitError(Message, 23);
	    }
	    /* join the multicast group */
	    mreq.imr_interface.s_addr= INADDR_ANY;
	    mreq.imr_multiaddr.s_addr= inet_addr(opt.MulticastChannel); 
	    join_group= setsockopt(fd, IPPROTO_IP,
	      IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq));
	    if (join_group < 0)
		sprintf(MsgBuf, DataPortFormat, -1);
	    else
		sprintf(MsgBuf, DataPortFormat, DEFAULT_PORT);

	    /* tell it our clients */
	    ml= strlen(MsgBuf);
	    for (p=0; p<PeerCount; p++) {
	    	fputs(MsgBuf, Peer[p].fout);
		fflush(Peer[p].fout);
	    }
	    if (join_group < 0) {
		sprintf(MsgBuf,
		  "cannot join multicast channel %.50s\n",
		  opt.MulticastChannel);
		exitError(MsgBuf, 22);
	    }
	}
	else
#endif /*MULTICAST*/
	{
	    DataPort= TTCP_SERVICE+1;
	    while (DataPort < 20000) {
		sinlh.sin_port = htons(DataPort);
		if (bind(fd, (struct sockaddr *)&sinlh, sizeof(sinlh)) < 0)
		    DataPort+= 1;
		else
		    break;
	    }
	    if (DataPort >= 20000) {
		fprintf(Peer[0].fout, DataPortFormat, 0);
		exitError("too many bind calls on datasocket", 17);
	    }
	    else {
	        listen(fd, 5);	/* its essential to listen to this socket
		                 * BEFORE we tell the port our partner!
				 * Otherwise the connect of our partner (if he
				 * is fast enough) may be refused since we haven't
				 * called listen yet.
				 */
		fprintf(Peer[0].fout, DataPortFormat, DataPort);
	    }
	    fflush(Peer[0].fout);
	}
	if (opt.udp) { 
	    /* nothing to do */
	}
	else { /* == TCP */
	    struct sockaddr_in frominet;
	    int fromleng;
	    fromleng = sizeof(frominet);
	    memset(&frominet, 0, fromleng);
	    AlarmMsg= "accept timed out\n";
	    signal(SIGALRM, AlarmExit);
	    SetItVal(80*1000);
	    if ((fd=accept(fd, (struct sockaddr *)&frominet, &fromleng)) < 0)
		exitError("accept", 18);
	    SetItVal(0);
	    if (opt.Verbose) {
		struct sockaddr_in peer;
		int peerlen = sizeof(peer);
		if (getpeername(fd, (struct sockaddr *)&peer, &peerlen) < 0)
		    exitError("getpeername", 19);
		sprintf(MsgBuf,
		  "accept from %s\n", inet_ntoa(peer.sin_addr));
		Message(MsgBuf);
	    }
	}
    }
    
    /*
    if (SetSockopt(fd, SOL_SOCKET, SO_LINGER, (char *)&Ling, sizeof(Ling))<0)
	exitError("fd setsockopt-linger", 22);
    */

    /* now both sides have an open connection */

    /* things to do on both sides */
    if (opt.udp)  {
    }
    else { /* !udp / TCP */
    	if (opt.SockOpt) {
	    int one;
	    if (SetSockopt(fd, SOL_SOCKET, opt.SockOpt, (char *)&one, sizeof(one)) < 0)
		exitError("setsockopt", 23);
	}
	if (opt.Window) {
	    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF,
		    (char *)&opt.SndWin, sizeof(opt.SndWin)) < 0)
		exitError("setsockopt", 24);
	    if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF,
		    (char *)&opt.RcvWin, sizeof(opt.RcvWin)) < 0)
		exitError("setsockopt", 25);
	}
    }

    /* print window sizes */
    if (opt.Verbose) {
	int optlen;
	int WinSize;

	optlen= sizeof(WinSize);
	if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *)&WinSize, &optlen) < 0)
	    strcpy(MsgBuf, "get send window size didn't work\n");
	else
	    sprintf(MsgBuf, "send window size = %d\n", WinSize);
	Message(MsgBuf);
	optlen= sizeof(WinSize);
	if (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *)&WinSize, &optlen) < 0)
	    strcpy(MsgBuf, "get recv window size didn't work\n");
	else
	    sprintf(MsgBuf, "receive window size = %d\n", WinSize);
	Message(MsgBuf);
    }

    if (opt.Verbose) {
	sprintf(MsgBuf, "buflen=%d, bufcnt=%d, dataport=%d/%s%s\n",
	  opt.BufLen, opt.BufCnt, DataPort,
	  opt.udp?"udp":"tcp", Remote?"\n":"");
	Message(MsgBuf);
    	
	if (!Remote) { /*Local*/
	    /* get messages from the other side(s) */
	    FetchRemoteMsg('\n');
	}
    }
    if (opt.PidCalls) {
    	/* do only the measure of the pidrate */
	sprintf(MsgBuf, "pid-rate (from %d calls): %.0f/s\n",
	  opt.PidCalls, GetPidRate(opt.PidCalls));
	Message(MsgBuf);
	goto Fin;
    }


    if (opt.Transmit) {
	int n= opt.BufCnt;
    	signal(SIGPIPE, SigPipe);
	if (opt.udp) {
	    (void)Nwrite(fd, buf, 4);			/* rcvr start */
	    nBytes= 0;
	    nBuffer= 0;
	    if (opt.StreamPattern)
	    	StartPattern(opt.InitString);
	    else
		Pattern(buf, opt.BufLen);
	    StartTimer();
	    while (n-- && Nwrite(fd, buf, opt.BufLen) == opt.BufLen) {
		nBytes+= opt.BufLen;
		nBuffer++;
	    }
	    StopTimer(&cput, &realt);
	    opt.GapLength= 0;
	    (void)Nwrite(fd, buf, 4);			/* rcvr end */
	    sleep(2);
	    (void)Nwrite(fd, buf, 4);			/* rcvr end */
	    sleep(2);
	    (void)Nwrite(fd, buf, 4);			/* rcvr end */
	}
	else {
	    nBytes= 0;
	    nBuffer= 0;
	    StartTimer();
	    while (n-- && Nwrite(fd, buf, opt.BufLen) == opt.BufLen) {
		nBytes+= opt.BufLen;
		nBuffer++;
	    }
	    StopTimer(&cput, &realt);
	}
	if (opt.Verbose) {
	    sprintf(MsgBuf, "transmitted %ld bytes\n", nBytes);
	    Message(MsgBuf);
	}
	close(fd);
syslog(LOG_DEBUG, "call from %.50s (=%.30s): everything transmitted\n",
Peer[0].HostName, Peer[0].IPName);
    }
    else { /* receive */
	int cnt;
	int ReportCnt;
	unsigned long FailedCnt=0;  /* number of failed comparisions */
	if (opt.Compare) {
	    ReportCnt= FailedCnt= 0;
	    if (!opt.StreamPattern)
		streamr250(ExpectBuf, opt.BufLen);
	}
	nBytes= 0;
	nBuffer= 0;
	if (opt.udp) {
	    while ((cnt=Nread(fd, buf, opt.BufLen)) > 0)  {
		static int going = 0;
		if (cnt <= 4) {
		    if (going) {
			StopTimer(&cput, &realt);
			if (opt.Verbose)
			    Message("got EOF\n");
			break;	/* "EOF" */
		    }
		    else {
			going = 1;
			StartTimer();
		    }
		}
		else {
		    nBuffer++;
		    if (opt.StreamPattern)
			streamr250(ExpectBuf, cnt);
		    if (opt.Compare)
		    	FailedCnt+= BufCompare(buf, ExpectBuf, cnt, nBytes, opt.BufLen, &ReportCnt);
		    nBytes += cnt;
		}
	    }
	}
	else {
syslog(LOG_DEBUG, "call from %.50s (%.30s): start receiving\n",
Peer[0].HostName, Peer[0].IPName);
	    StartTimer();
	    while ((cnt=Nread(fd, buf, opt.BufLen)) > 0) {
	    	if (opt.Compare) {
		    streamr250(ExpectBuf, cnt);
		    FailedCnt+= BufCompare(buf, ExpectBuf, cnt, nBytes, opt.BufLen, &ReportCnt);
		}
		nBytes += cnt;
		nBuffer++;
	    }
	    StopTimer(&cput, &realt);
syslog(LOG_DEBUG, "call from %.50s (%.30s: everything received\n",
Peer[0].HostName, Peer[0].IPName);
	}

	if (opt.Verbose) {
	    sprintf(MsgBuf, "received %ld bytes\n", nBytes);
	    Message(MsgBuf);
	}
	if (opt.Compare && ReportCnt < FailedCnt) {
	    sprintf(MsgBuf, "further %ld differences not reported\n",
	      FailedCnt-ReportCnt);
	    Message(MsgBuf);
	}
#if defined(MULTICAST)
	if (opt.MulticastChannel) {
	    /* leave the multicast group */
	    struct ip_mreq mreq;
	    mreq.imr_interface.s_addr= INADDR_ANY;
	    mreq.imr_multiaddr.s_addr= inet_addr(opt.MulticastChannel); 
	    setsockopt(fd,
	      IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&mreq, sizeof(mreq));
	}
#endif /*MULTICAST*/
    }
    
    { 	/* now lets do our output. What is printed is determined
   	 * by a string with the following printf like format string.
	 * The specifiers which are allowed:
	 * l		buffer-length in bytes		(int)
	 * n		buffer-count			(int)
	 * b		transfered bytes		(int)
	 * c		calls     	 		(int)
	 * rt		real-time in s      		(float)
	 * ct		cpu-time in s			(float)
	 * rbr		real bit rate in MBit/s		(float)
	 * cbr		cpu bit rate in MBit/s		(float)
	 * rcr		real call reate in Calls/s	(float)
	 * ccr		cpu call rate in Calls/s	(float)
	 */
	char *iFormat= "%*.*ld";
	char *fFormat= "%*.*f";
	char *fs;
	LenStr *TitleLine, *StatLine;

	fs= opt.Format;
	TitleLine= LenStrMake(" ");
	if (Remote) {
	    sprintf(MsgBuf, "%d", opt.RemoteNumber);
	    StatLine= LenStrMake(MsgBuf);
	}
	else
	    StatLine= LenStrMake("l");

	while (*fs) {
	    char *cp, *nc; 
	    char *TitleStr;
	    int fp, fw;
	    
	    cp= strchr(fs, '%');
	    if (cp) {
		int l= cp-fs;
		if (l >= sizeof(MsgBuf))
		    l= sizeof(MsgBuf)-1;
		LenStrncat(StatLine, fs, l);
		LenStrPadRight(TitleLine, ' ', LenStrLeng(TitleLine)+l);
		fs= cp;
	    }
	    else {
		LenStrcpy(StatLine, fs);
		LenStrPadRight(TitleLine, ' ', strlen(fs));
		break;
	    }
	    fs++;
	    nc= fs;
	    fw= strtol(fs, &nc, 0);
	    if (nc == fs)
		fw= 0;
	    else
	    	fs= nc;
	    if (*fs == '.') {
		fs++;
		nc= fs;
		fp=strtol(fs, &nc, 0);
		if (nc == fs)
		    fp= 0;
		else
		    fs=nc;
	    }
	    else
		fp= 0;

	    if (fp>30)
		fp= 30;
	    if (fw>30)
		fw= 30;
	    /*
	     * l		buffer-length in bytes		(int)
	     * n		buffer-count			(int)
	     * b		transfered bytes		(int)
	     * c		calls     	 		(int)
	     * rt		real-time in s      		(float)
	     * rbr		real bit rate in MBit/s		(float)
	     * rcr		real call reate in Calls/s	(float)
	     * ct		cpu-time in s			(float)
	     * cbr		cpu bit rate in MBit/s		(float)
	     * ccr		cpu call rate in Calls/s	(float)
	     */
	    if (strncmp(fs, "ccr", 3)==0) {
		sprintf(MsgBuf, fFormat, fw, fp,
		  ((double)SysCalls)/cput);
		TitleStr= "CPU-C/s";
		fs+= 3;
	    }
	    else if (strncmp(fs, "cbr", 3)==0) {
		sprintf(MsgBuf, fFormat, fw, fp,
		  ((double)nBytes)/cput/125000);
		TitleStr= "CPU-MBit/s";
		fs+= 3;
	    }
	    else if (strncmp(fs, "ct", 2)==0) {
		sprintf(MsgBuf, fFormat, fw, fp, cput);
		TitleStr= "CPU s";
		fs+= 2;
	    }
	    else if (strncmp(fs, "rcr", 3)==0) {
		sprintf(MsgBuf, fFormat, fw, fp,
		  ((double)SysCalls)/realt);
		TitleStr= "Real-C/s";
		fs+= 3;
	    }
	    else if (strncmp(fs, "rbr", 3)==0) {
		sprintf(MsgBuf, fFormat, fw, fp,
		  ((double)nBytes)/realt/125000);
		TitleStr= "Real-MBit/s";
		fs+= 3;
	    }
	    else if (strncmp(fs, "rt", 2)==0) {
		sprintf(MsgBuf, fFormat, fw, fp, realt);
		TitleStr= "Real s";
		fs+= 2;
	    }
	    else if (*fs == 'l') {
		sprintf(MsgBuf, iFormat, fw, fp, opt.BufLen);
		TitleStr= "BufLen";
		fs++;
	    }
	    else if (*fs == 'n') {
		sprintf(MsgBuf, iFormat, fw, fp, nBuffer);
		TitleStr= "BufCnt";
		fs++;
	    }
	    else if (*fs == 'b') {
		sprintf(MsgBuf, iFormat, fw, fp, nBytes);
		TitleStr= "Bytes";
		fs++;
	    }
	    else if (*fs == 'c') {
		sprintf(MsgBuf, iFormat, fw, fp, SysCalls);
		TitleStr= "Calls";
		fs++;
	    }
	    else
		continue;

	    LenStrcat(StatLine, MsgBuf);
	    sprintf(MsgBuf, "%*s", fw, TitleStr);
	    LenStrcat(TitleLine, MsgBuf);
	}

	if (opt.Title) {
	    LenStrApp(TitleLine, '\n');
	    fputs(LenStrString(TitleLine), stdout); 
	}
	LenStrApp(StatLine, '\n');
	fputs(LenStrString(StatLine), stdout); 
    }

  Fin:
    if (Remote) {
    	if (opt.Verbose) {
	    Message("exiting\n\n");
	}
	fflush(stdout);
	fflush(stderr);
	shutdown(0, 2);
	shutdown(1, 2);
	shutdown(2, 2);
     }
    else {
	FetchRemoteMsg('\n');
    }
    exit(0);
}
