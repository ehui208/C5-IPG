
#ifndef _LINPHONEC_INTERFACE_H
#define _LINPHONEC_INTERFACE_H

#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

#include "video_multicast_common.h"

typedef enum eXosip_event_type
{
	/* REGISTER related events */
	EXOSIP_REGISTRATION_NEW,           /**< announce new registration.       */
	EXOSIP_REGISTRATION_SUCCESS,       /**< user is successfully registred.  */
	EXOSIP_REGISTRATION_FAILURE,       /**< user is not registred.           */
	EXOSIP_REGISTRATION_REFRESHED,     /**< registration has been refreshed. */
	EXOSIP_REGISTRATION_TERMINATED,    /**< UA is not registred any more.    */

	/* INVITE related events within calls */
	EXOSIP_CALL_INVITE,            /**< announce a new call                    0x05 ist:step2*/
	EXOSIP_CALL_REINVITE,          /**< announce a new INVITE within call     */

	EXOSIP_CALL_NOANSWER,          /**< announce no answer within the timeout */
	EXOSIP_CALL_PROCEEDING,        /**< announce processing by a remote app   */
	EXOSIP_CALL_RINGING,           /**< announce ringback                     0x09 */
	EXOSIP_CALL_ANSWERED,          /**< announce start of call                0x0A */
	EXOSIP_CALL_REDIRECTED,        /**< announce a redirection                */
	EXOSIP_CALL_REQUESTFAILURE,    /**< announce a request failure            */
	EXOSIP_CALL_SERVERFAILURE,     /**< announce a server failure             */
	EXOSIP_CALL_GLOBALFAILURE,     /**< announce a global failure             */
	EXOSIP_CALL_ACK,               /**< ACK received for 200ok to INVITE      0x0F ist:step4*/

	EXOSIP_CALL_CANCELLED,         /**< announce that call has been cancelled 0x10 */
	EXOSIP_CALL_TIMEOUT,           /**< announce that call has failed         0x11 */

	/* request related events within calls (except INVITE) */
	EXOSIP_CALL_MESSAGE_NEW,              /**< announce new incoming request. */
	EXOSIP_CALL_MESSAGE_PROCEEDING,       /**< announce a 1xx for request. */
	EXOSIP_CALL_MESSAGE_ANSWERED,         /**< announce a 200ok  			  0x14 ist:step5*/
	EXOSIP_CALL_MESSAGE_REDIRECTED,       /**< announce a failure. */
	EXOSIP_CALL_MESSAGE_REQUESTFAILURE,   /**< announce a failure. */
	EXOSIP_CALL_MESSAGE_SERVERFAILURE,    /**< announce a failure. */
	EXOSIP_CALL_MESSAGE_GLOBALFAILURE,    /**< announce a failure. */

	EXOSIP_CALL_CLOSED,            /**< a BYE was received for this call      */

	/* for both UAS & UAC events */
	EXOSIP_CALL_RELEASED,             /**< call context is cleared.           0x1A ist:step6 */

	/* response received for request outside calls */
	EXOSIP_MESSAGE_NEW,              /**< announce new incoming request. 	0x1b - ist:step1*/
	EXOSIP_MESSAGE_PROCEEDING,       /**< announce a 1xx for request. */
	EXOSIP_MESSAGE_ANSWERED,         /**< announce a 200ok  				0x1d - ist:step3*/
	EXOSIP_MESSAGE_REDIRECTED,       /**< announce a failure. */
	EXOSIP_MESSAGE_REQUESTFAILURE,   /**< announce a failure. */
	EXOSIP_MESSAGE_SERVERFAILURE,    /**< announce a failure. */
	EXOSIP_MESSAGE_GLOBALFAILURE,    /**< announce a failure. */

	/* Presence and Instant Messaging */
	EXOSIP_SUBSCRIPTION_UPDATE,         /**< announce incoming SUBSCRIBE.      */
	EXOSIP_SUBSCRIPTION_CLOSED,         /**< announce end of subscription.     */

	EXOSIP_SUBSCRIPTION_NOANSWER,          /**< announce no answer              */
	EXOSIP_SUBSCRIPTION_PROCEEDING,        /**< announce a 1xx                  */
	EXOSIP_SUBSCRIPTION_ANSWERED,          /**< announce a 200ok                */
	EXOSIP_SUBSCRIPTION_REDIRECTED,        /**< announce a redirection          */
	EXOSIP_SUBSCRIPTION_REQUESTFAILURE,    /**< announce a request failure      */
	EXOSIP_SUBSCRIPTION_SERVERFAILURE,     /**< announce a server failure       */
	EXOSIP_SUBSCRIPTION_GLOBALFAILURE,     /**< announce a global failure       */
	EXOSIP_SUBSCRIPTION_NOTIFY,            /**< announce new NOTIFY request     */

	EXOSIP_SUBSCRIPTION_RELEASED,          /**< call context is cleared.        */

	EXOSIP_IN_SUBSCRIPTION_NEW,            /**< announce new incoming SUBSCRIBE.*/
	EXOSIP_IN_SUBSCRIPTION_RELEASED,       /**< announce end of subscription.   */

	EXOSIP_NOTIFICATION_NOANSWER,          /**< announce no answer              */
	EXOSIP_NOTIFICATION_PROCEEDING,        /**< announce a 1xx                  */
	EXOSIP_NOTIFICATION_ANSWERED,          /**< announce a 200ok                */
	EXOSIP_NOTIFICATION_REDIRECTED,        /**< announce a redirection          */
	EXOSIP_NOTIFICATION_REQUESTFAILURE,    /**< announce a request failure      */
	EXOSIP_NOTIFICATION_SERVERFAILURE,     /**< announce a server failure       */
	EXOSIP_NOTIFICATION_GLOBALFAILURE,     /**< announce a global failure       */

	EXOSIP_EVENT_COUNT                  	/**< MAX number of events              */
} eXosip_event_type_t;

#define	LINPHONE_DAT_BUF_LEN				240

typedef struct vdp_linphonec_if_buffer_head_tag
{
	unsigned char 	len;				// 数据包长度, 除开本身的所有字节
	int				targetIP;		// 发送方标示为目标ip地址，接收方标示为本机地址
	//int				sourceIP;		// 发送标示为源ip地址，接收方标示为目标地址
	unsigned char 	cmd;			// 事件类型
	unsigned char		buf[LINPHONE_DAT_BUF_LEN];
} vdp_linphonec_if_buffer_head;

int init_linphone_if_service( void );
int deinit_linphone_if_service(void);

int API_linphonec_Invite( int ip );
int API_linphonec_Answer( void );
int API_linphonec_Close( void );
int API_linphonec_quit( void );

#endif

