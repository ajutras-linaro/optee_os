/* SPDX-License-Identifier: BSD-2-Clause */
/**
 * @copyright 2019 NXP
 *
 * @file    pta_hantro_vpu.h
 *
 * @brief   Generator of the PTA UUID (i.MX8M platform).
 */
#ifndef PTA_HANTRO_VPU_H
#define PTA_HANTRO_VPU_H

/** @brief  PTA UUID generated at https://www.gguid.com/ */
#define PTA_HANTRO_VPU_PTA_UUID {0xf45a8128,0x23ff,0x4949,{0x98,0xa4,0x58,0xcb,0x8a,0xef,0x5a,0x75}}

#define PTA_HANTRO_VPU_CMD_WAIT				0
#define PTA_HANTRO_VPU_CMD_READ				1
#define PTA_HANTRO_VPU_CMD_WRITE			2
#define PTA_HANTRO_VPU_CMD_WRITE_MULTIPLE	3
#define PTA_HANTRO_VPU_CMD_READ_MULTIPLE	4

#define VPU_G1_DECODER_IT			7
#define VPU_G2_DECODER_IT 			8
#define HANTRODEC_IRQ_STAT_DEC 		1
#define HANTRODEC_IRQ_STAT_DEC_OFF 	(HANTRODEC_IRQ_STAT_DEC * 4)
#define HANTRODEC_DEC_IRQ_DISABLE 	0x10
#define HANTRODEC_DEC_IRQ 			0x100

#define HANTRODEC_G1_BASE_ADDR_IN_OFF 	0x30
#define HANTRODEC_G1_BASE_ADDR_OUT_OFF 	0x34

#define HANTRODEC_G2_DEC_CTRL_REG_0 			0xC
#define HANTRODEC_G2_HWIF_DEC_OUT_EC_BYPASS		0x20000
#define HANTRODEC_G2_HWIF_DEC_OUT_RS_E			0x10000

#define HANTRODEC_G2_BASE_ADDR_IN_OFF 			0x2A4
#define HANTRODEC_G2_BASE_ADDR_OUT_OFF 			0x104
#define HANTRODEC_G2_BASE_ADDR_RS_OUT_OFF		0x2C4
#define HANTRODEC_G2_BASE_ADDR_COMPRESS_OUT_OFF 0x2F8

typedef struct vpu_session {
	uint32_t coreId;
	uint32_t wait_flag;
} vpu_session;

#endif /* PTA_HANTRO_VPU_H */
