
#ifndef _VDP_NET_MANANGE_IP_LIST_H
#define _VDP_NET_MANANGE_IP_LIST_H

#include "vdp_net_manange.h"

#define	IP_LIST_NUMBER_MAX			100
	
#define 	IP_LIST_UPDATE_STATE_IDLE	0
#define 	IP_LIST_UPDATE_STATE_BUSY	1

#define 	IP_LIST_ATTR_BOTH_OK			0
#define 	IP_LIST_ATTR_IP_REPEAT		1
#define 	IP_LIST_ATTR_MAC_REPEAT		2
#define 	IP_LIST_ATTR_BOTH_REPEAT		3

typedef struct _IPG_LIST_ARRAY_
{
	NetManange_ipg_list_update		asker;
	int							state;		//0/idle, 1/updating
	ipg_list_node 					node[IP_LIST_NUMBER_MAX];
	int							number;
} IPG_LIST_ARRAY;

int API_IPG_List_Update_Get_Node( NetManange_ipg_list_report* pOneNode );

int API_IPG_List_Update_Request( unsigned char msg_target_id, unsigned char  msg_source_id, unsigned char msg_type, unsigned char sub_type );

int API_IPG_List_Update_Reply( int ask_ip, unsigned char msg_target_id, unsigned char  msg_source_id, unsigned char msg_type, unsigned char sub_type );

void API_IPG_List_Update_Over( void );

// 得到ipglist的总共条数
int API_IPG_List_Report_Max_Number( void );
//得到ipg list 的节点数据
int API_IPG_List_Report_Data( int start, int number, ipg_list_node* pnodes );


#endif



