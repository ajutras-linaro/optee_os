/**
 * @copyright 2019 NXP
 *
 * @file    pta_hcp.h
 *
 * @brief   Generator of the PTA UUID (i.MX8M platform).
 */
#ifndef PTA_HDCP_H
#define PTA_HDCP_H

/** @brief  PTA UUID generated at https://www.gguid.com/ */
/* aaf0c79e-5ff4-4f8c-bef4-042337f0d418 */
#define HDCP_PTA_UUID {0xaaf0c79e,0x5ff4,0x4f8c,{0xbe,0xf4,0x04,0x23,0x37,0xf0,0xd4,0x18}}

#define PTA_HDCP_BASE_ADDR          	0x32000000
#define PTA_HDCP_HDP_REGISTER_BA       	0x32c00000
#define PTA_HDCP_HDP_REGISTER_SIZE	0x100000
#define PTA_HDCP_HDP_SEC_REGISTER_BA   	0x32e40000
#define PTA_HDCP_HDP_SEC_REGISTER_SIZE  0x40000
#define PTA_HDCP_RESET_REGISTER_BA    	0x32e2f000
#define PTA_HDCP_RESET_REGISTER_SIZE	0x10

#define PTA_HDCP_HDP_BUSID		0
#define PTA_HDCP_HDP_SEC_BUSID		1
#define PTA_HDCP_RESET_BUSID		2

#define PTA_HDCP_CMD_WRITE 		0
#define PTA_HDCP_CMD_READ 		1
#define PTA_HDCP_CMD_GET_CAPABILITY	2

typedef struct hdcp_session {
	uint32_t busId;
} hdcp_session;

#endif /* PTA_HDCP_H */
