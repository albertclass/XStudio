#ifndef _IPHDR_H_
#define _IPHDR_H_

#include <pshpack1.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#include <inaddr.h>
#define LITTLE_ENDIAN
//
// IPv4 Header (without any IP options)
//
typedef struct ip_hdr
{
	#if defined(LITTLE_ENDIAN)
		unsigned char ihl : 4;     //�ײ�����  
		unsigned char version : 4; //�汾   
	#else
		unsigned char version : 4; //�汾  
		unsigned char ihl : 4;     //�ײ�����  
	#endif

    unsigned char  tos;				// IP type of service
    unsigned short totlen;			// Total length
    unsigned short id;				// Unique identifier 
	unsigned short frag_off;        // Fragment offset field
    unsigned char  ttl;				// Time to live
    unsigned char  proto;			// Protocol(TCP,UDP etc)
    unsigned short checksum;		// IP checksum
    unsigned int   src_addr;			// Source address
    unsigned int   dst_addr;			// Source address
} IPV4_HDR, *PIPV4_HDR, FAR * LPIPV4_HDR;

//
// Define the UDP header 
//
typedef struct udp_hdr
{
    unsigned short src_portno;       // Source port no.
    unsigned short dest_portno;      // Dest. port no.
    unsigned short udp_length;       // Udp packet length
    unsigned short udp_checksum;     // Udp checksum
} UDP_HDR, *PUDP_HDR;

//
// Define the TCP header
//
typedef struct _tcp_hdr
{
	unsigned short src_port;    //Դ�˿ں�  
	unsigned short dst_port;    //Ŀ�Ķ˿ں�  
	unsigned int seq_num;        //���к�  
	unsigned int ack_num;        //ȷ�Ϻ�  
	#if defined(LITTLE_ENDIAN)
		unsigned char reserved_1 : 4; //����6λ�е�4λ�ײ�����  
		unsigned char thl : 4;        //tcpͷ������  
		unsigned char flag : 6;       //6λ��־  
		unsigned char reseverd_2 : 2; //����6λ�е�2λ  
	#else
		unsigned char thl : 4;        //tcpͷ������  
		unsigned char reserved_1 : 4; //����6λ�е�4λ�ײ�����  
		unsigned char reseverd_2 : 2; //����6λ�е�2λ  
		unsigned char flag : 6;       //6λ��־   
	#endif
	unsigned short wnd_size;    //16λ���ڴ�С  
	unsigned short chk_sum;     //16λTCP�����  
	unsigned short urgt_p;      //16Ϊ����ָ��  
} TCP_HDR, *PTCP_HDR;

typedef struct _peer
{
	unsigned int	addr;
	unsigned short	port;
}TCP_PEER, *PTCP_PEER;

__inline bool operator < (const struct _peer& lhs, const struct _peer& rhs)
{
	return (lhs.addr < rhs.addr) ? true :
		(lhs.addr == rhs.addr) ? (lhs.port < rhs.port) : 
		false;
}

__inline bool operator > (const struct _peer& lhs, const struct _peer& rhs)
{
	return rhs < lhs;
}

__inline bool operator == (const struct _peer& lhs, const struct _peer& rhs)
{
	return lhs.addr == rhs.addr && lhs.port == rhs.port;
}

__inline bool operator != (const struct _peer& lhs, const struct _peer& rhs)
{
	return !(lhs == rhs);
}

typedef struct _tcp_connection
{
	TCP_PEER src;
	TCP_PEER dst;
}TCP_CONNECTION, PTCP_CONNECTION;

__inline bool operator == (const struct _tcp_connection& lhs, const struct _tcp_connection& rhs)
{
	return lhs.src == rhs.src && lhs.dst == rhs.dst;
}

__inline bool operator != (const struct _tcp_connection& lhs, const struct _tcp_connection& rhs)
{
	return !(lhs == rhs);
}

typedef struct _tcp_unorder
{
	struct _tcp_unorder *next;
	TCP_HDR			tcp;
	unsigned int	len;
	char			data[1];
}TCP_UNORDER, *PTCP_UNORDER;

typedef struct _tcp_session
{
	char state;
	PTCP_UNORDER 	unorder_list;	// δ�����
	unsigned int	seq_num;		// ������seq

	char	msgbuf[0xffff]; // msg buffer
	size_t	msglen;
}TCP_SESSION, *PTCP_SESSION;

//
// Stucture to extract port numbers that overlays the UDP and TCP header
//
typedef struct port_hdr
{
    unsigned short src_portno;
    unsigned short dest_portno;
} PORT_HDR, *PPORT_HDR;

//
// IGMP header
//
typedef struct igmp_hdr
{
    unsigned char   version_type;
    unsigned char   max_resp_time;
    unsigned short  checksum;
    unsigned long   group_addr;
} IGMP_HDR, *PIGMP_HDR;

typedef struct igmp_hdr_query_v3
{
    unsigned char   type;
    unsigned char   max_resp_time;
    unsigned short  checksum;
    unsigned long   group_addr;
    unsigned char   resv_suppr_robust;
    unsigned char   qqi;
    unsigned short  num_sources;
    unsigned long   sources[1];
} IGMP_HDR_QUERY_V3, *PIGMP_HDR_QUERY_V3;

typedef struct igmp_group_record_v3
{
    unsigned char   type;
    unsigned char   aux_data_len;
    unsigned short  num_sources;
    unsigned long   group_addr;
    unsigned long   source_addr[1];
} IGMP_GROUP_RECORD_V3,  *PIGMP_GROUP_RECORD_V3;

typedef struct igmp_hdr_report_v3
{
    unsigned char   type;
    unsigned char   reserved1;
    unsigned short  checksum;
    unsigned short  reserved2;
    unsigned short  num_records;
} IGMP_HDR_REPORT_V3, *PIGMP_HDR_REPORT_V3;

struct pkghead16
{
	unsigned short mark;
	unsigned short chk;
	unsigned short len;
};

struct pkghead32
{
	unsigned short	mark;
	unsigned short	chk;
	unsigned int	len;
};

struct data_header
{
	unsigned short	mark;
	unsigned short	business;
	unsigned int	length;
};

struct pkghead
{
	unsigned short	_length;
};

struct msghead
{
	unsigned short	_class;
	unsigned short	_message;
	unsigned int	_reserved;
};

#define	TH_FIN	0x01
#define	TH_SYN	0x02
#define	TH_RST	0x04
#define	TH_PUSH	0x08
#define	TH_ACK	0x10
#define	TH_URG	0x20

#define TH_NETDEV        0x00000001
#define TH_TAPI          0x00000002

#define CLOSED		0
#define SYN_SENT	1
#define SYN_RECV	2
#define ESTABLISHED 3
#define FIN_WAIT_1	4
#define FIN_WAIT_2	5
#define CLOSE_WAIT	6
#define TIME_WAIT	7
#define LAST_ACK	8

//
// Size defines
//
#define MAX_IP_SIZE        65535
#define MIN_IP_HDR_SIZE       20

//
// Macros to extract the high and low order 4-bits from a byte
//
#define HI_BYTE(byte)    (((byte) >> 4) & 0x0F)
#define LO_BYTE(byte)    ((byte) & 0x0F)

//
// Used to indicate to parser what fields to filter on
//
#define FILTER_MASK_SOURCE_ADDRESS      0x01
#define FILTER_MASK_SOURCE_PORT         0x02
#define FILTER_MASK_DESTINATION_ADDRESS 0x04
#define FILTER_MASK_DESTINATION_PORT    0x08

#include <poppack.h>
#endif
