/*****************************************************************************
 * Projekt   : PING
 * Name      : Modul PINGW32.H
 * Funktion  : Datentypen
 *
 * Autor     : Patrick Haller [Dienstag, 26.09.1995 17.57.39]
 *****************************************************************************/

#ifndef MODULE_PING
#define MODULE_PING

#ifdef __cplusplus
      extern "C" {
#endif

#pragma pack(1)

/*****************************************************************************
 * Definitions                                                               *
 * The following definitions are excerpts from the IBM OS/2 TCP/IP stack as  *
 * the original Microsoft Toolkit lacks all of the important stuff.          *
 *****************************************************************************/


typedef u_short n_short;                /* short as received from the net */
typedef u_long  n_long;                 /* long as received from the net */
typedef u_long  n_time;                 /* ms since 00:00 GMT, byte rev */


/*
 * Options for use with [gs]etsockopt at the IP level.
 */
#define IP_OPTIONS      1               /* set/get IP per-packet options */

#define IP_MULTICAST_IF    2            /* set/get IP multicast interface*/
#define IP_MULTICAST_TTL   3            /* set/get IP multicast timetolive*/
#define IP_MULTICAST_LOOP  4            /* set/get IP multicast loopback */
#define IP_ADD_MEMBERSHIP  5            /* add  an IP group membership   */
#define IP_DROP_MEMBERSHIP 6            /* drop an IP group membership   */

#define IP_DEFAULT_MULTICAST_TTL  1     /* normally limit m'casts to 1 hop */
#define IP_DEFAULT_MULTICAST_LOOP 1     /* normally hear sends if a member */
#define IP_MAX_MEMBERSHIPS       20     /* per socket; must fit in one mbuf*/

/*
 * Argument structure for IP_ADD_MEMBERSHIP and IP_DROP_MEMBERSHIP.
 */
struct ip_mreq {
        struct in_addr  imr_multiaddr;  /* IP multicast address of group */
        struct in_addr  imr_interface;  /* local IP address of interface */
};



/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 *
 *      @(#)ip.h        7.6.1.1 (Berkeley) 3/15/88
 */
#define LITTLE_ENDIAN   1234    /* least-significant byte first (vax) */
#define BIG_ENDIAN      4321    /* most-significant byte first (IBM, net) */
#define PDP_ENDIAN      3412    /* LSB first in word, MSW first in long (pdp) */

#if defined(OS2) || defined(__OS2__) || defined(_M_IX86)
#define BYTE_ORDER      LITTLE_ENDIAN
#else
#ifndef BYTE_ORDER
/*
 * Definitions for byte order,
 * according to byte significance from low address to high.
 */

#ifdef vax
#define BYTE_ORDER      LITTLE_ENDIAN
#else
#define BYTE_ORDER      BIG_ENDIAN      /* mc68000, tahoe, most others */
#endif
#endif
#endif

/*
 * Definitions for internet protocol version 4.
 * Per RFC 791, September 1981.
 */
#define IPVERSION       4

/*
 * Structure of an internet header, naked of options.
 *
 * We declare ip_len and ip_off to be short, rather than u_short
 * pragmatically since otherwise unsigned comparisons can result
 * against negative integers quite easily, and fail in subtle ways.
 */
struct ip {
#if BYTE_ORDER == LITTLE_ENDIAN
        int     ip_hl:4,                /* header length */
                ip_v:4;                 /* version */
#endif
#if BYTE_ORDER == BIG_ENDIAN
        u_char  ip_v:4,                 /* version */
                ip_hl:4;                /* header length */
#endif
        u_char  ip_tos;                 /* type of service */
        short   ip_len;                 /* total length */
        u_short ip_id;                  /* identification */
        short   ip_off;                 /* fragment offset field */
#define IP_DF 0x4000                    /* dont fragment flag */
#define IP_MF 0x2000                    /* more fragments flag */
        u_char  ip_ttl;                 /* time to live */
        u_char  ip_p;                   /* protocol */
        u_short ip_sum;                 /* checksum */
        struct  in_addr ip_src,ip_dst;  /* source and dest address */
};
#ifndef OS2
#define IP_MAXPACKET    65535           /* maximum packet size */
#else
#define IP_MAXPACKET    32767           /* maximum packet size */
#endif

/*
 * Definitions for options.
 */
#define IPOPT_COPIED(o)         ((o)&0x80)
#define IPOPT_CLASS(o)          ((o)&0x60)
#define IPOPT_NUMBER(o)         ((o)&0x1f)

#define IPOPT_CONTROL           0x00
#define IPOPT_RESERVED1         0x20
#define IPOPT_DEBMEAS           0x40
#define IPOPT_RESERVED2         0x60

#define IPOPT_EOL               0               /* end of option list */
#define IPOPT_NOP               1               /* no operation */

#define IPOPT_RR                7               /* record packet route */
#define IPOPT_TS                68              /* timestamp */
#define IPOPT_SECURITY          130             /* provide s,c,h,tcc */
#define IPOPT_LSRR              131             /* loose source route */
#define IPOPT_SATID             136             /* satnet id */
#define IPOPT_SSRR              137             /* strict source route */

/*
 * Offsets to fields in options other than EOL and NOP.
 */
#define IPOPT_OPTVAL            0               /* option ID */
#define IPOPT_OLEN              1               /* option length */
#define IPOPT_OFFSET            2               /* offset within option */
#define IPOPT_MINOFF            4               /* min value of above */

/*
 * Time stamp option structure.
 */
struct  ip_timestamp {
        u_char  ipt_code;               /* IPOPT_TS */
        u_char  ipt_len;                /* size of structure (variable) */
        u_char  ipt_ptr;                /* index of current entry */
#if BYTE_ORDER == LITTLE_ENDIAN
        int     ipt_flg:4,              /* flags, see below */
                ipt_oflw:4;             /* overflow counter */
#endif
#if BYTE_ORDER == BIG_ENDIAN
        u_char  ipt_oflw:4,             /* overflow counter */
                ipt_flg:4;              /* flags, see below */
#endif
        union ipt_timestamp {
                n_long  ipt_time[1];
                struct  ipt_ta {
                        struct in_addr ipt_addr;
                        n_long ipt_time;
                } ipt_ta[1];
        } ipt_timestamp;
};

/* flag bits for ipt_flg */
#define IPOPT_TS_TSONLY         0               /* timestamps only */
#define IPOPT_TS_TSANDADDR      1               /* timestamps and addresses */
#define IPOPT_TS_PRESPEC        2               /* specified modules only */

/* bits for security (not byte swapped) */
#define IPOPT_SECUR_UNCLASS     0x0000
#define IPOPT_SECUR_CONFID      0xf135
#define IPOPT_SECUR_EFTO        0x789a
#define IPOPT_SECUR_MMMM        0xbc4d
#define IPOPT_SECUR_RESTR       0xaf13
#define IPOPT_SECUR_SECRET      0xd788
#define IPOPT_SECUR_TOPSECRET   0x6bc5

/*
 * Internet implementation parameters.
 */
#define MAXTTL          255             /* maximum time to live (seconds) */
#define IPFRAGTTL       60              /* time to live for frags, slowhz */
#define IPTTLDEC        1               /* subtracted when forwarding */

#define IP_MSS          576             /* default maximum segment size */

struct  ipstat {
        long    ips_total;              /* total packets received */
        long    ips_badsum;             /* checksum bad */
        long    ips_tooshort;           /* packet too short */
        long    ips_toosmall;           /* not enough data */
        long    ips_badhlen;            /* ip header length < data size */
        long    ips_badlen;             /* ip length < ip header length */
        long    ips_fragments;          /* fragments received */
        long    ips_fragdropped;        /* frags dropped (dups, out of space) */
        long    ips_fragtimeout;        /* fragments timed out */
        long    ips_forward;            /* packets forwarded */
        long    ips_cantforward;        /* packets rcvd for unreachable dest */
        long    ips_redirectsent;       /* packets forwarded on same net */
        long    ips_ipOutRequests;      /* requested for transmission */
        long    ips_ipOutNoRoutes;      /* discarded - no route */
        long    ips_ipFragFails;        /* had to fragment but could not */
        long    ips_ipFragOKs;          /* successfully fragmented datagrams */
        long    ips_ipFragCreates;      /* number of fragments created */
        long    ips_ipReasmOKs;         /* datagrams successfully assembled */
        long    ips_ipOutDiscards;      /* discarded due to lack of space */
        long    ips_ipInDiscards;       /* discarded due to lack of space 0 */
        long    ips_ipInAddrErrors;     /* discarded due to lack of space 0 */
        long    ips_ipInDelivers;       /* datagrams successfully delivered */
        long    ips_ipInUnknownProtos;  /* datagrams with unknown protocol  */
};


/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 *
 *      @(#)ip_icmp.h   7.3 (Berkeley) 12/7/87
 */

/*
 * Interface Control Message Protocol Definitions.
 * Per RFC 792, September 1981.
 */

/*
 * Structure of an icmp header.
 */
struct icmp {
        u_char  icmp_type;              /* type of message, see below */
        u_char  icmp_code;              /* type sub code */
        u_short icmp_cksum;             /* ones complement cksum of struct */
        union {
                u_char ih_pptr;                 /* ICMP_PARAMPROB */
                struct in_addr ih_gwaddr;       /* ICMP_REDIRECT */
                struct ih_idseq {
                        n_short icd_id;
                        n_short icd_seq;
                } ih_idseq;
                int ih_void;
        } icmp_hun;
#define icmp_pptr       icmp_hun.ih_pptr
#define icmp_gwaddr     icmp_hun.ih_gwaddr
#define icmp_id         icmp_hun.ih_idseq.icd_id
#define icmp_seq        icmp_hun.ih_idseq.icd_seq
#define icmp_void       icmp_hun.ih_void
        union {
                struct id_ts {
                        n_time its_otime;
                        n_time its_rtime;
                        n_time its_ttime;
                } id_ts;
                struct id_ip  {
                        struct ip idi_ip;
                        /* options and then 64 bits of data */
                } id_ip;
                u_long  id_mask;
                char    id_data[1];
        } icmp_dun;
#define icmp_otime      icmp_dun.id_ts.its_otime
#define icmp_rtime      icmp_dun.id_ts.its_rtime
#define icmp_ttime      icmp_dun.id_ts.its_ttime
#define icmp_ip         icmp_dun.id_ip.idi_ip
#define icmp_mask       icmp_dun.id_mask
#define icmp_data       icmp_dun.id_data
};

/*
 * Lower bounds on packet lengths for various types.
 * For the error advice packets must first insure that the
 * packet is large enought to contain the returned ip header.
 * Only then can we do the check to see if 64 bits of packet
 * data have been returned, since we need to check the returned
 * ip header length.
 */
#define ICMP_MINLEN     8                               /* abs minimum */
#define ICMP_TSLEN      (8 + 3 * sizeof (n_time))       /* timestamp */
#define ICMP_MASKLEN    12                              /* address mask */
#define ICMP_ADVLENMIN  (8 + sizeof (struct ip) + 8)    /* min */
#define ICMP_ADVLEN(p)  (8 + ((p)->icmp_ip.ip_hl << 2) + 8)
        /* N.B.: must separately check that ip_hl >= 5 */

/*
 * Definition of type and code field values.
 */
#define ICMP_ECHOREPLY          0               /* echo reply */
#define ICMP_UNREACH            3               /* dest unreachable, codes: */
#define         ICMP_UNREACH_NET        0               /* bad net */
#define         ICMP_UNREACH_HOST       1               /* bad host */
#define         ICMP_UNREACH_PROTOCOL   2               /* bad protocol */
#define         ICMP_UNREACH_PORT       3               /* bad port */
#define         ICMP_UNREACH_NEEDFRAG   4               /* IP_DF caused drop */
#define         ICMP_UNREACH_SRCFAIL    5               /* src route failed */
#define ICMP_SOURCEQUENCH       4               /* packet lost, slow down */
#define ICMP_REDIRECT           5               /* shorter route, codes: */
#define         ICMP_REDIRECT_NET       0               /* for network */
#define         ICMP_REDIRECT_HOST      1               /* for host */
#define         ICMP_REDIRECT_TOSNET    2               /* for tos and net */
#define         ICMP_REDIRECT_TOSHOST   3               /* for tos and host */
#define ICMP_ECHO               8               /* echo service */
#define ICMP_TIMXCEED           11              /* time exceeded, code: */
#define         ICMP_TIMXCEED_INTRANS   0               /* ttl==0 in transit */
#define         ICMP_TIMXCEED_REASS     1               /* ttl==0 in reass */
#define ICMP_PARAMPROB          12              /* ip header bad */
#define ICMP_TSTAMP             13              /* timestamp request */
#define ICMP_TSTAMPREPLY        14              /* timestamp reply */
#define ICMP_IREQ               15              /* information request */
#define ICMP_IREQREPLY          16              /* information reply */
#define ICMP_MASKREQ            17              /* address mask request */
#define ICMP_MASKREPLY          18              /* address mask reply */

#define ICMP_MAXTYPE            18

#define ICMP_INFOTYPE(type) \
        ((type) == ICMP_ECHOREPLY || (type) == ICMP_ECHO || \
        (type) == ICMP_TSTAMP || (type) == ICMP_TSTAMPREPLY || \
        (type) == ICMP_IREQ || (type) == ICMP_IREQREPLY || \
        (type) == ICMP_MASKREQ || (type) == ICMP_MASKREPLY)


        
#ifdef __cplusplus
      }
#endif


#endif /* MODULE_PHS_TYPES */

