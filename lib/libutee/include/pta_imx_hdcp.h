/**
 * @copyright 2019 NXP
 *
 * @file    pta_imx_hdcp.h
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

#define PTA_HDCP_HEADER_CMD_SIZE	4

#define PTA_HDCP_HDP_BUSID		0
#define PTA_HDCP_HDP_SEC_BUSID		1
#define PTA_HDCP_RESET_BUSID		2

#define PTA_HDCP_CMD_WRITE		0
#define PTA_HDCP_CMD_READ		1
#define PTA_HDCP_CMD_GET_STATUS		2
#define PTA_HDCP_CMD_GET_SRM_STATUS	3
#define PTA_HDCP_CMD_OPEN_DRM_SESSION	4
#define PTA_HDCP_CMD_CLOSE_DRM_SESSION	5

#define TA_SECURE_HDCP_CONTROL_HDCP_PORT 1
#define TA_SECURE_HDCP_CONTROL_MAX_SESSION 65535

typedef enum e_hdcp_set_capability {
	HDCP_V2_2_ONLY = 0, 		// force HDCP version 2.2
	HDCP_V1_4_ONLY = 1, 		// force HDCP version 1.4
	HDCP_V2_2_V1_4 = 2, 		// support both HDCP - It will always try the first 2.2 and if the receiver does not support 2.2, then try 1.4
	HDCP_CAPABILITY_NOT_SET
} t_hdcp_set_capability;

typedef enum e_hdcp_set_mode {
	DISABLE_HDCP = 0,
	ENABLE_HDCP = 1,
	HDCP_MODE_NOT_SET
} t_hdcp_set_mode;

typedef enum e_hdcp_set_type {
	HDCP_TYPE0 = 0,
	HDCP_TYPE1 = 1,
	HDCP_TYPE_NOT_SET
} t_hdcp_set_type;

typedef enum e_hdcp_set_km_key_encrytion {
	HDCP_NO_KM_KEY_ENCRYPTION = 0,
	HDCP_KM_KEY_ENCRYPTION = 1,
	HDCP_KM_KEY_ENCRYPTION_NOT_SET
} t_hdcp_set_km_key_encrytion;

typedef enum e_hdcp_status_authenticated {
	HDCP_STATUS_NOT_AUTENTICATED=0,
	HDCP_STATUS_AUTENTICATED=1,
	HDCP_STATUS_AUTENTICATED_NOT_SET
} t_hdcp_status_authenticated;

typedef enum e_hdcp_status_receiver_type {
	HDCP_STATUS_RECEIVER_TYPE0=0,
	HDCP_STATUS_RECEIVER_TYPE1=1,
	HDCP_STATUS_RECEIVER_TYPE_NOT_SET
} t_hdcp_status_receiver_type;

typedef enum e_hdcp_status_receiver_hdcp_capability {
	HDCP_STATUS_RECEIVER_HDCP_NOT_SET=0,
	HDCP_STATUS_RECEIVER_HDCP1=1,
	HDCP_STATUS_RECEIVER_HDCP2=2
} t_hdcp_status_receiver_hdcp_capability;

typedef enum e_hdcp_status_AuthStreamId {
	HDCP_STATUS_AUTHSTREAMDID_SUCCESS=0,
	HDCP_STATUS_AUTHSTREAMDID_FAILED=1,
	HDCP_STATUS_AUTHSTREAMDID_NOT_SET
} t_hdcp_status_AuthStreamId;

typedef enum e_hdcp_status_work_with_enable_1_1_features {
	HDCP_STATUS_DO_NOT_WORK_WITH_ENABLE_1_1_FEATURES=0,
	HDCP_STATUS_WORK_WITH_ENABLE_1_1_FEATURES=1,
	HDCP_STATUS_WORK_WITH_ENABLE_1_1_FEATURES_NOT_SET
} t_hdcp_status_work_with_enable_1_1_features;

typedef enum e_hdcp_status_error_type{
	HDCP_TX_ERR_NO_ERROR=0,
	HDCP_TX_ERR_HPD_IS_DOWN=1,
	HDCP_TX_ERR_SRM_FAILER=2,
	HDCP_TX_ERR_SIGNATURE_VERIFICATION=3,
	HDCP_TX_ERR_H_TAG_DIFF_H=4,
	HDCP_TX_ERR_V_TAG_DIFF_V=5,
	HDCP_TX_ERR_LOCALITY_CHECK=6,
	HDCP_TX_ERR_DDC=7,
	HDCP_TX_ERR_REAUTH_REQ=8,
	HDCP_TX_ERR_TOPOLOGY=9,
	HDCP_TX_ERR_VERIFY_RECEIVER_ID_LIST_FAILED=0xa,
	HDCP_TX_ERR_HDCP_RSVD1=0xb,
	HDCP_TX_ERR_HDMI_CAPABILITY=0xc,
	HDCP_TX_ERR_RI=0xd,
	HDCP_TX_ERR_WATCHDOG_EXPIRED=0xe,
	HDCP_TX_ERR_REAPEATER_INTEGRITY_FAILED=0xf
} t_hdcp_status_error_type;

typedef enum e_HDCP_Capability {
  HDCP_NONE	= 0,  // No HDCP supported, no secure data path.
  HDCP_V1	= 1,  // HDCP version 1.0
  HDCP_V2	= 2,  // HDCP version 2.0
  HDCP_V2_1	= 3,  // HDCP version 2.1
  HDCP_V2_2	= 4,   // HDCP version 2.2 (type 1 required)
  HDCP_NO_DIGITAL_OUTPUT = 0xff  // No digital output.
} t_HDCP_Capability;

typedef struct hdcp_session {
	uint32_t busId;
} hdcp_session;

#endif /* PTA_HDCP_H */
