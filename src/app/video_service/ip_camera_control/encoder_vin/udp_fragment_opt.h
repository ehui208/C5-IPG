
#ifndef _UDP_FRAGMENT_OPT_H
#define _UDP_FRAGMENT_OPT_H

#define PACK_MARK  			"SWITCH"
#define PACK_MARK_LEN 		6

#define VIDEO_BUF_COUNT 	10
#define VIDEO_BUF_LEN 		(200*1200)		//1244160  //720*576*3

#define VIDEO_MAX_PACK_NUM  200
#define VIDEO_BUF_FRAME 	1

#define VIDEO_DATA_TYPE		0x8001

typedef int			bool;
#define true		1
#define false		0

typedef struct
{
	unsigned int 	Timestamp;           				//时间戳
	unsigned short 	FrameNo;           					//帧序号
	short 			TotalPackage;               		//总包数
	bool 			PackageReceived[VIDEO_MAX_PACK_NUM];//已收到包数
	unsigned int 	Len;                 				//数据长度
	bool 			AllReceived;                 		//数据包全收到标记
	unsigned char 	*Buffer;            				//数据
	unsigned short 	FrameType;         					//帧类型
} AVPack;

typedef struct _AVPackNode_t
{
	AVPack 					Content;
	struct _AVPackNode_t 	*llink, *rlink;
} AVPackNode;


#pragma pack(2)
typedef struct 
{
	unsigned short DataType;          //数据类型
	unsigned short FrameNo;           //帧序号
	unsigned int   Timestamp;         //时间戳
	unsigned int   Framelen;          //帧数据长度
	unsigned short TotalPackage;      //总包数
	unsigned short CurrPackage;       //当前包数
	unsigned short Datalen;           //数据长度
	unsigned short PackLen;           //数据包大小
}AVPackHead;

#define FRAGMENT_SIZE		1200

typedef struct
 { 
	//每帧所有分片公用信息 
	char 			flag[PACK_MARK_LEN]; 	//可以任意字符或数字或其他方法，用来标志我们发送的数据 
	AVPackHead		m_head;					// 数据包头
	unsigned char	m_data[FRAGMENT_SIZE];	// 数据区
}udp_fragment_t;

#pragma pack()


int init_send_fragment_head( udp_fragment_t* p_udp_fragment);

// 功能: 准备一个数据包前需要初始化一个分包包头
// 参数: p_udp_fragment - 分发数据包结构，udp_total_size - udp待分包的数据长度
// 返回: 0/ok， 1/err
int start_send_fragment_head( udp_fragment_t* p_udp_fragment, int udp_total_size);

// 功能: 从一个待分包的数据缓冲区提取数据组装为分发包
// 参数: p_udp_fragment - 分发数据包结构，p_udp_buf - udp待分包的数据区
// 返回: 0/ok， 1/err
int pull_send_fragment_data( udp_fragment_t* p_udp_fragment, unsigned char* p_udp_buf);

// 功能: 准备下一个分包数据
// 参数: p_udp_fragment - 分发数据包结构
// 返回: 0/ok， 1/err
int prepare_for_next_fragment( udp_fragment_t* p_udp_fragment);


void CVideoPackProcessor_InitNode();
void CVideoPackProcessor_ClearNode();
void CVideoPackProcessor_LockNode();
void CVideoPackProcessor_UnLockNode();
void ReleaseSemaphore(void);
void TriggerSemaphore(void);
int CVideoPackProcessor_ProcPack(unsigned char *buf,int len);
AVPackNode * CVideoPackProcessor_PickPack();
void CVideoPackProcessor_ReturnNode(AVPackNode * pNode);


#endif

