

#ifndef _obj_ResourceTable_H
#define _obj_ResourceTable_H

#define RESOURCETB_FLUSH_TIME				60*10


#define RESOURCE_ID					"RID"
#define RESOURCE_FILE				"FILENAME"
#define RESOURCE_GETTIME			"GET_TIME"
#define RESOURCE_ERRCODE			"ERR_CODE"
#define RESOURCE_LASTUPDATE		"LAST_UPDATE"
#define RESOURCE_UPDATERESULT		"UPDATE_RESULT"

#define RESOURCE_ID_LEN					40
#define RESOURCE_FILE_LEN					80
#define RESOURCE_GETTIME_LEN				40				
#define RESOURCE_ERRCODE_LEN				40
#define RESOURCE_LASTUPDATE_LEN			40
#define RESOURCE_UPDATERESULT_LEN		40

typedef struct
{
	time_t			modified_time;
	int				modified_add_cnt;
	int				modified_pre_cnt;	
	pthread_mutex_t	lock;
	OS_TIMER 		autosave_timer;
}ResourceTb_Run_Stru;
extern ResourceTb_Run_Stru ResourceTb_Run;

typedef struct
{
	int 		rid;
	char 	filename[RESOURCE_FILE_LEN + 1];
	time_t	get_time;
	int		err_code;
	time_t	last_update;
	int		update_result;
}ResourceTb_OneRecord_Stru;

int Init_ResourceTb_Head(void);
int API_Load_ResourceTable(void);
int API_ResourceTb_AddOneRecord( int rid,int errcode,char *rsname,char *prersname);
int API_ResourceTb_Flush_Process(int op);
	#define API_ResourceTb_Flush_Immediate()	API_ResourceTb_Flush_Process(1)
	#define API_ResourceTb_Flush_Polling()		API_ResourceTb_Flush_Process(0)
int API_Resource_HaveUpdated(int rid,int result);
int API_ResourceTb_DelOneRecord(int rid,char *delfilename);
int API_ResourceTb_GetOneRecord(int rid,ResourceTb_OneRecord_Stru *precord);
int API_ResourceTb_Verify(void);
void ResourceTable_AutoSave_Timer_Callback(void);

#endif


