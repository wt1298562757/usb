

#include "PED_CMD.h"





/*��ʼ��USB���պͷ��Ͷ���*/
void USB_COM_MSG_Init()
{
    uint8_t i;
	for(i=0,i<DOWN_FMT_SIZE,i++)
	{
		USB_Rx_Buffer[i] = 0;
	}
		for(i=0,i<UP_FMT_SIZE,i++)
	{
		USB_Tx_Buffer[i] = 0;
	}
}

