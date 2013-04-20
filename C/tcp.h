/*	BSDI tcp.h,v 2.11 1996/10/11 16:01:49 pjd Exp	*/

/*
 * Copyright (c) 1982, 1986, 1993, 1998
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)tcp.h	8.1 (Berkeley) 6/10/93
 */

#ifndef _NETINET_TCP_H_
#define _NETINET_TCP_H_

typedef	u_long	tcp_seq;
/*
 * TCP header.
 * Per RFC 793, September, 1981.
 */
struct tcphdr {
	u_short	th_sport;		/* source port */
	u_short	th_dport;		/* destination port */
	tcp_seq	th_seq;			/* sequence number */
	tcp_seq	th_ack;			/* acknowledgement number */
#if BYTE_ORDER == LITTLE_ENDIAN 
	u_char	th_x2:4,		/* (unused) */
		th_off:4;		/* data offset */
#endif
#if BYTE_ORDER == BIG_ENDIAN 
	u_char	th_off:4,		/* data offset */
		th_x2:4;		/* (unused) */
#endif

#define TH_ELN  0x1 /* explicit loss notification */
#define TH_ECN  0x2 /* explicit congestion notification */
#define TH_FS   0x4 /* fast start */

	u_char	th_flags;
#define	TH_FIN	0x01
#define	TH_SYN	0x02
#define	TH_RST	0x04
#define	TH_PUSH	0x08
#define	TH_ACK	0x10
#define	TH_URG	0x20
	u_short	th_win;			/* window */
	u_short	th_sum;			/* checksum */
	u_short	th_urp;			/* urgent pointer */
};

#define	TCPOPT_EOL		0
#define	TCPOPT_NOP		1
#define	TCPOPT_MAXSEG		2

#define REMAP_SET_FROM		100
#define REMAP_SET_TO		101

#define TCPOLEN_MAXSEG		4
#define TCPOPT_WINDOW		3
#define TCPOLEN_WINDOW		3
#define TCPOPT_SACK_PERMITTED	4		/* Experimental */
#define TCPOLEN_SACK_PERMITTED	2
#define TCPOPT_SACK		5 /* Experimental */
/*#define TCPOPT_SACK		6	*/	/* XXX FOR EXPTS ONLY */
#ifdef ACC
#define TCPOPT_PEERWIN          7
#define TCPOLEN_PEERWIN         4
#define TCPOPT_PEERWIN_HDR      (TCPOPT_PEERWIN<<8|TCPOLEN_PEERWIN)
#endif
#define TCPOPT_TIMESTAMP	8
#define    TCPOLEN_TIMESTAMP		10
#define    TCPOLEN_TSTAMP_APPA		(TCPOLEN_TIMESTAMP+2) /* appendix A */

#define TCPOPT_TSTAMP_HDR	\
    (TCPOPT_NOP<<24|TCPOPT_NOP<<16|TCPOPT_TIMESTAMP<<8|TCPOLEN_TIMESTAMP)

#define TCPOPT_SACK_PERMIT_HDR  \
(TCPOPT_NOP<<24|TCPOPT_NOP<<16|TCPOPT_SACK_PERMITTED<<8|TCPOLEN_SACK_PERMITTED)
#define TCPOPT_SACK_HDR         (TCPOPT_NOP<<24|TCPOPT_NOP<<16|TCPOPT_SACK<<8)

#ifdef SACK
#define TCPOLEN_SACK            8 /* 2*sizeof(tcp_seq): len of a sack block */
#endif

/* NOTE: SMART and SACK are mutually exclusive */
#define TCPOPT_SMART  5
#define TCPOLEN_SMART 10 

#define TCPOLEN_SMART_TOTAL (TCPOLEN_SMART + 2) /* including 2 leading NOPs */
#define TCPOPT_SMART_HDR       \
    (TCPOPT_NOP<<24|TCPOPT_NOP<<16|TCPOPT_SMART<<8|TCPOLEN_SMART)


/*
 * Default maximum segment size for TCP.
 * With an IP MSS of 576, this is 536,
 * but 512 bytes of data is probably more convenient.
 * However, we must leave space for timestamp options (12 bytes).
 */
#ifdef LARGE_IP_MSS
#ifdef IPINIP
#define	TCP_MSS	 (IP_MSS - sizeof (struct tcpiphdr) - sizeof(struct ip))
#else
#define	TCP_MSS	 (IP_MSS - sizeof (struct tcpiphdr))
#endif
#else
#define	TCP_MSS	536
#endif

#define	TCP_MAXWIN	65535	/* largest value for (unscaled) window */

#define TCP_MAX_WINSHIFT	14	/* maximum window shift */

/*
 * User-settable options (used with setsockopt).
 */
#define	TCP_NODELAY	0x01	/* don't delay send to coalesce packets */
#define	TCP_MAXSEG	0x02	/* set maximum segment size */
#define	TCP_STDURG	0x03	/* URGENT pointer is last byte of urgent data */

#ifdef KERNEL
/* Parameters that can be set with sysctl. */
int	pmtu_expire;
int	pmtu_probe;
int	tcp_43maxseg;		/* Fill in MAXSEG per 4.2BSD rules */
int	tcp_conntimeo;		/* initial connection timeout */
int	tcp_do_rfc1323;
int	tcp_keepcnt;		/* max idle probes */
int	tcp_keepidle;		/* time before probing idle */
int	tcp_keepintvl;		/* interval betwn idle probes */
int	tcp_maxpersistidle;	/* max idle time in persist */
int	tcp_pmtu;		/* turn on path MTU discovery code */
int	tcp_recvspace;
int	tcp_sendspace;
int 	tcp_mssdflt;
int 	tcp_rttdflt;
int	tcp_syn_cache_limit;	/* Maximum # entries allowed in SYN cache */
int	tcp_syn_cache_interval;	/* Interval for SYN cache timer, in 1/2 secs */
int	tcp_syn_bucket_limit;
#endif

/* snoop and related expts (esp. ber generation) */
#define	TCP_SNOOP_ERRPROB0	0x101
#define	TCP_SNOOP_ERRPROB1	0x102
#define	TCP_SNOOP_BER_MODEL	0x103  
#define	TCP_SNOOP_TRANS0	0x104
#define	TCP_SNOOP_TRANS1	0x105
#define TCP_SNOOP_TIMERGRAN	0x106
#define TCP_SNOOP_BER_DISABLE   0x107   /* disable generation of errors */
#define TCP_SNOOP_DISABLE       0x108   /* disable snoop operation */
#define TCP_SNOOP_BASE          0x109   /* disable snoop operation */
#define TCP_SNOOP_MASK          0x10a   /* disable snoop operation */
  
/* tcp_stats, itcp, linkemu, eln, newreno, etc. */
#define TCP_ENABLE_STATS        0x200 /* collect TCP-related statistics */
#define ITCP_WIRED_SOCKET_SET   0x201 /* set socket for wired ITCP conn */
#define ITCP_WLESS_SOCKET_SET   0x202 /* set socket for the wless TICP conn */
#define ITCP_WIRED_SOCKET_CLR   0x203 /* clear socket for wired I-TCP */ 
#define ITCP_WLESS_SOCKET_CLR   0x204 /* clear socket for wireless I-TCP */
#define TCP_SNOOP_LINKEMU_ENABLE 0x205/* emulation of link-level retx */
#define TCP_ELN_ENABLE          0x206 /* enable ELN to the sender */
#define TCP_ELN_THRESH          0x207
#define TCP_ELN_ORDER           0x208
#define TCP_SNOOP_REXMT_DISABLE 0x209 /* disable snoop/local rexmission */
#define TCP_SMART_ENABLE        0x20a /* enable SMART rexmission scheme */
#define TCP_SMART_SNOOP_ENABLE  0x20b /* snoop + SMART */
#define TCP_PERFECT_ELN_ENABLE  0x20c /* perfect ELN information */
#define TCP_SNOOP_BURSTRATE     0x20d /* burst rate of packet errors */
#define TCP_NEWRENO_ENABLE      0x20e /*extend fast recovery on partial acks*/
#define TCP_GET_SRTT            0x20f /* get srtt from kernel */
#define TCP_GET_RTTVAR          0x210 /* get rtt variance from kernel */
#define TCP_NEWRENO_ENABLE_FOR_CONN 0x211  /* per-connection NEWRENO */

/* TCP SACK and Enhanced Recovery */
#define TCP_SACK_DISABLE        0x300 /* per-connection SACKs */
#define TCP_DO_SACK             0x301 /* SACKs */
#define TCP_ENH_RECOVERY        0x302 /* enhanced loss recovery mode */
#define TCP_ER                  0x303 /* per-connection ER */
#define TCP_ER_ALLCONNS         0x304 /* All conns ER (superceded by above) */

/* Asymmetric network stuff: DirecPC, delacks, etc. */
#define TCP_IGNORE_PEER         0x400 /* Ignore peer's mss (asymmetry) */
#define TCP_NOTSTAMP            0x401 /* Eliminate timestamps */
#define TCP_ADVT_LARGE_MSS      0x402 /* Advertise large mss */
#define TCP_DELACKS_ENABLE      0x403 /* Delayed acks */
#define TCP_SS_DELACK_OK        0x404 /* okay to delay acks during peer's
					 slow start phase */
#define TCP_PEER_MSS            0x405 /* MSS used by peer */
#define TCP_MAXBURST            0x406 /* maximum burst size for TCP sender */
#define TCP_CANCEL_BURST        0x407 /* cancel pending burst in tcp_output? */
#define TCP_MIN_ACKS_PER_WIN    0x408 /* minimum # of acks to send per win */
#define TCP_COUNT_BYTES_ACKED   0x409 /* count bytes acked for new cwnd */
 
/* misc. */
#define TCP_SND_CWND_SCALE      0x500 /* scaling factor initial snd_cwnd */
#define TCP_FAST_START_ENABLE   0x501 /* enable TCP fast start */
#define TCP_FAST_START_FLR_ENABLE 0x502 /* enable TCP-FS fast loss recovery */
#define TCP_FAST_START_FRT_ENABLE 0x503 /* enable TCP-FS fast reset timer  */
#define TCP_NO_QUENCH           0x504 /* tcp_quench not called when ip_output fails */ 
#define TCP_SESSION_ENABLE      0x505 /* enable TCP session */
#define TCP_ENABLE_SESSION_STATS 0x506 /* enable stats for TCP session */
#define TCP_SESSION_INCR_CWND   0x507 /* increment session cwnd for each new conn */
#define TCP_LOW_DELAY           0x508 /* set IPTOS_LOWDELAY for pkts of this conn */
/*
 * Names for TCP sysctl objects
 */
#define	TCPCTL_MSSDFLT		1	/* default seg size */
#define	TCPCTL_DO_RFC1323	2	/* use RFC1323 options */
#define	TCPCTL_KEEPIDLE		3	/* time before probing idle */
#define	TCPCTL_KEEPINTVL	4	/* interval betwn idle probes */
#define	TCPCTL_KEEPCNT		5	/* max idle probes */
#define	TCPCTL_MAXPERSISTIDLE	6	/* max idle time in persist */
#define	TCPCTL_SENDSPACE	7	/* default send buffer */
#define	TCPCTL_RECVSPACE	8	/* default recv buffer */
#define	TCPCTL_CONNTIMEO	9	/* default recv buffer */
#define	TCPCTL_PMTU		10	/* Enable path MTU discovery */
#define	TCPCTL_PMTU_EXPIRE	11	/* When to expire discovered MTU info */
#define	TCPCTL_PMTU_PROBE	12	/* When probing for higher MTU */
#define	TCPCTL_43MAXSEG		13	/* Fill in MAXSEG per 4.3BSD rules */
#define	TCPCTL_STATS		14	/* statistics */
#define	TCPCTL_SYN_CACHE_LIMIT	15	/* Max size of SYN cache */
#define	TCPCTL_SYN_BUCKET_LIMIT	16	/* Max size of buckets in SYN cache */
#define	TCPCTL_SYN_CACHE_INTER	17	/* Interval for SYN cache timer */
#define	TCPCTL_MAXID		18

#define	TCPCTL_NAMES { \
	{ 0, 0 }, \
	{ "mssdflt", CTLTYPE_INT }, \
	{ "do_rfc1323", CTLTYPE_INT }, \
	{ "keepidle", CTLTYPE_INT }, \
	{ "keepinterval", CTLTYPE_INT }, \
	{ "keepcount", CTLTYPE_INT }, \
	{ "maxpersistidle", CTLTYPE_INT }, \
	{ "sendspace", CTLTYPE_INT }, \
	{ "recvspace", CTLTYPE_INT }, \
	{ "conntimeo", CTLTYPE_INT }, \
	{ "pmtu", CTLTYPE_INT }, \
	{ "pmtu_expire", CTLTYPE_INT }, \
	{ "pmtu_probe", CTLTYPE_INT }, \
	{ "43maxseg", CTLTYPE_INT }, \
	{ "stats", CTLTYPE_STRUCT }, \
	{ "syn_cache_limit", CTLTYPE_INT }, \
	{ "syn_bucket_limit", CTLTYPE_INT }, \
	{ "syn_cache_interval", CTLTYPE_INT }, \
}

#define	TCPCTL_VARS { \
	0, \
	&tcp_mssdflt, \
	&tcp_do_rfc1323, \
	&tcp_keepidle, \
	&tcp_keepintvl, \
	&tcp_keepcnt, \
	&tcp_maxpersistidle, \
	&tcp_sendspace, \
	&tcp_recvspace, \
	&tcp_conntimeo, \
	&tcp_pmtu, \
	&pmtu_expire, \
	&pmtu_probe, \
	&tcp_43maxseg, \
	0, \
	&tcp_syn_cache_limit, \
	&tcp_syn_bucket_limit, \
	&tcp_syn_cache_interval, \
}

#endif /* !_NETINET_TCP_H_ */