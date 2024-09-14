#ifndef __LinkUsb_H
#define __LinkUsb_H


#define PC_DOWN_CMD_SIZE sizeof(DOWN_FMT)
#define UPLOAD_CONTENT_LEN  255
#define MSG_BUFF_WRITE_TIMEOUT 5000

typedef struct _FS_LINK_CMD_FMT{
 	uint8_t cmd;
	uint8_t	target;
	uint16_t para;
	uint16_t DataLength;
	char* Data;
}FS_LINK_CMD_FMT ;


typedef struct RUN_SCRIPT_CMD_FMT{
	uint16_t DataLength;
	char* Data;
}RUN_SCRIPT_CMD_FMT ;

typedef struct _DOWN_FMT{
	uint8_t startFlag;
	uint8_t pktSn;
	uint8_t cmd;
	uint8_t	target;
	uint16_t para;
	uint8_t handleFlag;
	uint16_t DataLength;
	char* Data;
}DOWN_FMT;

typedef struct _UP_FMT{
	uint8_t startFlag;
	uint8_t pktSn;
	uint8_t msgType;	//
	uint8_t src;		//refer to INFO_CATEGORY
	uint32_t msgLenth;
	uint32_t ts;
	uint8_t content[UPLOAD_CONTENT_LEN+1];
}UP_FMT;


#endif
