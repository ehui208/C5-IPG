#ifndef _FAVCAVCODEC_H_
#define _FAVCAVCODEC_H_


//-------------------------------------
#define P_SLICE 				0x00
#define B_SLICE 				0x01
#define I_SLICE 				0x02

// Constant value, don't change
/*
#define OUTPUT_FMT_CbYCrY	0
#define OUTPUT_FMT_RGB555	1
#define OUTPUT_FMT_RGB888	2
#define OUTPUT_FMT_RGB565	3
#define OUTPUT_FMT_YUV420	4
#define OUTPUT_FMT_YUV422	5
*/
#define OUTPUT_FMT_YUV420	0
#define OUTPUT_FMT_YUV422	1

//------------------------------

	// ioctl flag, should be consistent with driver definition
#define FAVC_IOCTL_DECODE_INIT			0x4170
#define FAVC_IOCTL_DECODE_FRAME   	        0x4172
#define FAVC_IOCTL_DECODE_420_INIT          0x4174
	
#define FAVC_IOCTL_ENCODE_INIT    	        0x4173
#define FAVC_IOCTL_ENCODE_FRAME   	        0x4175
#define FAVC_IOCTL_GET_SYSINFO  	        0x4177
#define FAVC_IOCTL_GET_SPSPPS     	        0x4179
#define FAVC_IOCTL_ENCODE_NVOP	        0x417B
#define FAVC_IOCTL_ENCODE_INIT_MP4		0x417D
#define FAVC_IOCTL_ENCODE_VUI               	0x417F
#define FAVC_IOCTL_ENCODE_INIT_WRP		0x4180
#define FAVC_IOCTL_ENCODE_CROP               0x4181

#define FAVC_IOCTL_ENCODE_GET_BSINFO	0x4185

#define FAVC_IOCTL_DECODE_GET_BSSIZE	0x4186
#define FAVC_IOCTL_DECODE_GET_BSINFO	0x4187      // This is only for debug
#define FAVC_IOCTL_DECODE_GET_OUTPUT_INFO	0x4188
#define FAVC_IOCTL_ENCODE_GET_YUVSRCINFO	0x4189

#define FAVC_IOCTL_PERFORMANCE_RESET		0x41FE
#define FAVC_IOCTL_PERFORMANCE_REPORT		0x41FF
	
typedef struct avc_workbuf
{
	u_int32_t	addr;
	u_int32_t	size;
	u_int32_t	offset;
} avc_workbuf_t;	
	
/*****************************************************************************
 * Decoder structures
 ****************************************************************************/
typedef struct
{
	u_int32_t bEndOfDec;					// Used by driver
	u_int32_t u32Width;						// Decoded bitstream width
	u_int32_t u32Height;					// Decoded bitstream height
	u_int32_t u32UsedBytes;					// Reported used bitstream byte in buffer. It is not accurate. The inaccuracy is less than 256 bytes
	u_int32_t u32FrameNum;					// Decoded slice number
	//u_int8_t * pu8FrameBaseAddr_phy;		// output frame buffer
	//u_int8_t * pu8FrameBaseAddr_U_phy;		// frame buffer (U) if output format is yuv420
	//u_int8_t * pu8FrameBaseAddr_V_phy;		// frame buffer (V) if output format is yuv420
	u_int32_t isDisplayOut;					// 0 -> Buffer in reorder buffer, 1 -> available buffer, -1 -> last flush frame
	u_int32_t isISlice;						// 1-> I Slice, 0 -> P slice
	u_int32_t Reserved0;
} FAVC_DEC_RESULT; 
 
typedef struct
{
	u_int32_t u32API_version;
	u_int32_t u32MaxWidth;					// Not used now
	u_int32_t u32MaxHeight;					// Not used now
	u_int32_t u32FrameBufferWidth;			// LCM buffer width (in pixel)
	u_int32_t u32FrameBufferHeight;			// LCM buffer height (in pixel)
	//u_int32_t u32CacheAlign;				// Not used now
	u_int32_t u32Pkt_size;					// Current decoding bitstream length
	u_int8_t *pu8Pkt_buf;					// Current decoding bitstream buffer address
	u_int32_t pu8Display_addr[3];				// Buffer address for decoded data
	u_int32_t got_picture;					// 0 -> Decoding has someting error. 1 -> decoding is OK in current bitstream
	//uint32_t u32BS_buf_sz;					// Reserved bitstream buffer size for decoding (Updated by driver in initialization)
	u_int8_t * pu8BitStream_phy;				// physical address. buffer for bitstream
	//u_int8_t * pu8BitStream;					// virtual address.
	//u_int8_t * mb_info_phy;					// physical address for MB info (Updated by driver in initialization)
	//u_int8_t * intra_pred_phy;				// Physical address for intra predication (Updated by driver in initialization)
	u_int32_t u32OutputFmt;					// Decoded output format, 0-> Planar YUV420 format, 1-> Packet YUV422 foramt
	u_int32_t crop_x;						// pixel unit: crop x start point at decoded-frame (not supported now)
	u_int32_t crop_y;						// pixel unit: crop y start point at decoded-frame (not supported now)

	FAVC_DEC_RESULT tResult;				// Return decoding result by driver		
}FAVC_DEC_PARAM; 
 
/* API return values */
typedef enum 
{
	RETCODE_OK, 							// Operation succeed.
	RETCODE_ERR_MEMORY,					// Operation failed (out of memory).
	RETCODE_ERR_API ,						// Operation failed (API version error).
	RETCODE_ERR_HEADER,
	RETCODE_ERR_FILL_BUFFER,
	RETCODE_ERR_FILE_OPEN,
	RETCODE_HEADER_READY,
	RETCODE_BS_EMPTY,
	RETCODE_WAITING,
	RETCODE_DEC_OVERFLOW,
	RETCODE_HEADER_FINISH,
	RETCODE_DEC_TIMEOUT,
	RETCODE_PARSING_TIMEOUT,
	RETCODE_ERR_GENERAL,
	RETCODE_NOT_SUPPORT,
										// Below is added but not used
	RETCODE_FAILURE,
	RETCODE_FRAME_NOT_COMPLETE	
} AVC_RET;
 

/*****************************************************************************
 * H.264 Encoder structures
 ****************************************************************************/
typedef struct 
{
	u_int32_t u32API_version;
	u_int32_t u32BitRate;					// The encoded bitrate in bps.
	u_int32_t u32FrameWidth; 				// The width of encoded frame in pels.
	u_int32_t u32FrameHeight; 				// The height of encoded frame in pels.
	u_int32_t fFrameRate; 					// The base frame rate per second
	u_int32_t u32IPInterval; 					// The frame interval between I-frames.
	u_int32_t u32MaxQuant;					// The maximum quantization value. (max = 51)
	u_int32_t u32MinQuant;					// The minimum quantization value. (min=0)
	u_int32_t u32Quant; 					// The frame quantization value for initialization 

	int32_t ssp_output;   					// This variable tells the H.264 must be encoded out sps + pps before slice data.
										// ->  1 : force the encoder to output sps+pps  
										// ->  0 : force the encoder to output sps+pps on any Slice I frame
										// -> -1: (default) only output SPS+PPS on first IDR frame.
										
	int32_t intra; 							// This variable tells the H.264 must be encoded out an I-Slice type frame.
										// ->  1 : forces the encoder  to create a keyframe.
										// ->  0 :  forces the  encoder not to  create a keyframe.
										// -> -1: (default) let   the  encoder   decide  (based   on   contents  and u32IPInterval)  		

	/**********************************************************************************************/
	// The additional parameters for Region Of Interest feature
	/**********************************************************************************************/
	int32_t bROIEnable;  					// To enable the function of encoding rectangular region of interest(ROI) within captured frame. (0 : Disable, 1: Enable)
	u_int32_t u32ROIX;  						// The upper-left corner x coordinate of rectangular region of interest within captured frame if bROIEnable is enabled.
	u_int32_t u32ROIY;  						// The upper-left corner coordinate y of region of interest within captured frame if bROIEnable is enabled
	u_int32_t u32ROIWidth;  					// The width of user-defined rectangular region of interest within the captured frame in pixel units if bROIEnable is enabled
	u_int32_t u32ROIHeight; 					// The height of user-defined rectangular region of interest within the captured frame in pixel units if bROIEnable is enabled
	
	/**********************************************************************************************/
	// Buffer allocated by Application. 
	/**********************************************************************************************/  
	u_int8_t *pu8YFrameBaseAddr;  			// The base address for input Y frame buffer. (8-byte aligned)
	u_int8_t *pu8UVFrameBaseAddr; 			// The base address for input UV frame buffer in H.264 2D mode.(8-byte aligned)
	u_int8_t *pu8UFrameBaseAddr;  			// The base address for input U frame buffer.(8-byte aligned) (pu8UVFrameBaseAddr must be equal to pu8UFrameBaseAddr)
	u_int8_t *pu8VFrameBaseAddr;  			// The base address for input V frame buffer.(8-byte aligned)
	u_int8_t *pu8BitstreamAddr; 				// The bitstream buffer address while encoding one single frame provided by user (16-byte aligned)

	void *bitstream;						// Bitstream Buffer address for driver to write bitstream  		

	/**********************************************************************************************/
	// Below filed is updated by driver to application.
	/**********************************************************************************************/   		
	u_int32_t  bitstream_size;					// Bitstream length for current frame (Updated by driver)
	int32_t keyframe;  						// This parameter is indicated the Slice type of frame. (Updated by Driver, 1-> I slice, 0-> P slice) 
	int32_t frame_cost;  					// frame_cout is updated by driver. 
	
	/**********************************************************************************************/
	// Below filed is used by driver internally. Application doesn' need to care it.
	/**********************************************************************************************/  		
	u_int32_t no_frames; 					// The number of frames to be encoded (Not used now)
	u_int32_t threshold_disable; 				// The transform coefficients threshold.
	u_int32_t chroma_threshold;  				// The chroma coefficients threshold (0 ~ 7).
	u_int32_t luma_threshold;    				// The luma coefficients threshold (0 ~ 7).
	u_int32_t beta_offset;       				// The beta offset for in-loop filter.
	u_int32_t alpha_offset;      				// The alpha offset for in-loop filter.
	u_int32_t chroma_qp_offset;  				// The chroma qp offset (-12 to 12 inclusively).
	u_int32_t disable_ilf;       					// To disable in-loop filter or not
	u_int32_t watermark_enable;      			// To enable watermark function or not (Don't enable it now)
	u_int32_t watermark_interval;    			// To specify the watermark interval if watermark function is enabled 
	u_int32_t watermark_init_pattern;			// To specify the initial watermark pattern if watermark function is enabled   		

	u_int8_t*pu8ReConstructFrame;			// The address of reconstruct frame buffer. (16-byte aligned)(size = u32FrameWidth * @ref u32FrameHeight * 3/2)
	u_int8_t*pu8ReferenceFrame;  			// The address of reference frame buffer.(16-byte aligned)(size = u32FrameWidth * @ref u32FrameHeight * 3/2)
	u_int8_t *pu8SysInfoBuffer; 				// The address of system info buffer.(4-byte aligned)(size = MBs_Count_Width+1)/2) *64 *2)
	u_int8_t*pu8DMABuffer_virt; 				// The virtual address of DMA buffer, which size is equal to ((4*19+2)*sizeof(unsigned int));(4-byte aligned)
	u_int8_t *pu8DMABuffer_phy;  			// The physical address of DMA buffer, which size is equal to ((4*19+2)*sizeof(unsigned int));(4-byte aligned)
	int32_t nvop_ioctl; 						// This parameter is valid only on FAVC_IOCTL_ENCODE_NVOP. (Not implemented)
	u_int32_t multi_slice;
	u_int32_t pic_height; 					// This parameter is used to keep the frame height for sps and pps on Multi Slice mode
	u_int32_t pic_width;	 					// This parameter is used to keep the frame width for sps and pps on Multi Slice mode
	u_int32_t img_fmt;						// 0: 2D format, CbCr interleave, named H264_2D (VideoIn supported only)
	u_int32_t  control;						// 0 : Do NOT force one frame as one slice(default), 1 : Force one frame as one slice
} FAVC_ENC_PARAM;

typedef struct 
{
	u_int32_t  video_format;
	u_int8_t colour_description_present_flag;
	u_int32_t colour_primaries;
	u_int32_t  transfer_characteristics;
	u_int32_t  matrix_coefficients;
	u_int8_t chroma_location_info_present_flag;
	u_int32_t  chroma_sample_loc_type_top_field;
	u_int32_t  chroma_sample_loc_type_bottom_field;
}FAVC_VUI_PARAM ;

typedef struct 
{
	u_int32_t left_offset;
	u_int32_t right_offset;
	u_int32_t top_offset;
	u_int32_t buttom_offset;
}FAVC_CROP_PARAM;
#endif
