// SPDX-License-Identifier: BSD-2-Clause
/**
 * @copyright 2019 NXP
 *
 * @file    pta_hantro_vpu.c
 *
 * @brief   Pseudo Trusted Application.\n
 *			Secure VPU functionality (i.MX8M platform)
 */
/* Standard includes */
#include <stdlib.h>
#include <string.h>

#include <io.h>

/* Library kernel includes */
#include <kernel/interrupt.h>
#include <kernel/misc.h>
#include <kernel/pseudo_ta.h>
#include <kernel/tee_time.h>
#include <kernel/delay.h>
#include <mm/core_mmu.h>
#include <mm/core_memprot.h>

#include <pta_hantro_vpu.h>

/* Library tee includes */
#include <tee_api_types.h>

/* Global includes */
#include <tee_api_defines.h>

/** @brief    PTA name */
#define HANTRO_VPU_PTA_NAME "hantro_vpu.pta"

#define IS_ALIGN(x,b) (!(x & (b - 1)))

static vaddr_t get_vpu_base(vpu_session *session)
{
	vaddr_t ctrl_base = NULL;
	paddr_t vpu_base;

	if (session == NULL)
	{
		vpu_base = VPU_BLK_CTL_BASE;
	} else {
		switch (session->coreId)
		{
			case 0:
				vpu_base = VPU_CORE0_BASE;
				break;
			case 1:
				vpu_base = VPU_CORE1_BASE;
				break;
			default:
				return NULL;
		}
	}
	ctrl_base = (vaddr_t)phys_to_virt(vpu_base, MEM_AREA_IO_SEC);

	if (!ctrl_base)
	{
		DMSG("Map VPU Registers %p",vpu_base);

		if (!core_mmu_add_mapping(MEM_AREA_IO_SEC, vpu_base,
				ROUNDUP(VPU_SIZE, CORE_MMU_PGDIR_SIZE))) {
			EMSG("Unable to map VPU Registers");
			goto out;
		}

		ctrl_base = (vaddr_t)phys_to_virt(vpu_base, MEM_AREA_IO_SEC);
	}

out:
	return ctrl_base;
}

#ifdef IT_MODE
static enum itr_return __maybe_unused hantrodec_isr(struct itr_handler *handler)
{
	uint32_t irq_status_dec;
	vpu_session *session;
	vaddr_t ctrl_base;

	session = (vpu_session*)handler->data;

	ctrl_base = get_vpu_base(session);
	if (!ctrl_base) {
		EMSG("Unable to get the VPU Base address");
		return ITRR_NONE;
	}
	irq_status_dec = io_read32(ctrl_base + HANTRODEC_IRQ_STAT_DEC_OFF);

	if (irq_status_dec & HANTRODEC_DEC_IRQ) {
		/* clear dec IRQ */
		irq_status_dec &= (~HANTRODEC_DEC_IRQ);
		io_write32(ctrl_base + HANTRODEC_IRQ_STAT_DEC_OFF, irq_status_dec);

		session->wait_flag = 0;
	}

	return ITRR_HANDLED;
}

struct itr_handler vpu_g1_handler = {
	.it = VPU_G1_DECODER_IT,
	.flags   = ITRF_TRIGGER_LEVEL,
	.handler = hantrodec_isr,
};

struct itr_handler vpu_g2_handler = {
	.it = VPU_G2_DECODER_IT,
	.flags   = ITRF_TRIGGER_LEVEL,
	.handler = hantrodec_isr,
};
#endif

static TEE_Result openSessionEntryPoint(uint32_t param_types,
			       TEE_Param params[TEE_NUM_PARAMS],
			       void **sess_ctx)
{
	uint32_t core_exp_param_types;
	uint32_t ctl_exp_param_types;
	vpu_session *ctx = NULL;

	core_exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
					TEE_PARAM_TYPE_NONE,
					TEE_PARAM_TYPE_NONE,
					TEE_PARAM_TYPE_NONE);

	ctl_exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
					TEE_PARAM_TYPE_NONE,
					TEE_PARAM_TYPE_NONE,
					TEE_PARAM_TYPE_NONE);

	if ((param_types != core_exp_param_types)
		&& (param_types != ctl_exp_param_types))
		return TEE_ERROR_BAD_PARAMETERS;

	if (param_types == core_exp_param_types)
	{
		ctx = malloc(sizeof(vpu_session));
		if (!ctx)
			return TEE_ERROR_OUT_OF_MEMORY;

		memset(ctx,0,sizeof(vpu_session));

		ctx->coreId = params[0].value.a;

#ifdef IT_MODE
		ctx->wait_flag = 1;

		switch (params[0].value.a)
		{
			case 0:
				DMSG("Add it handler for core %d",ctx->coreId);
				vpu_g1_handler.data = ctx;
				itr_add(&vpu_g1_handler);
				itr_enable(vpu_g1_handler.it);
				break;
			case 1:
				DMSG("Add it handler for core %d",ctx->coreId);
				vpu_g2_handler.data = ctx;
				itr_add(&vpu_g2_handler);
				itr_enable(vpu_g2_handler.it);
				break;
			default:
			break;
		}
#endif
		DMSG("Open session for core %d",ctx->coreId);
	} else {
		DMSG("Open session for block control");
	}

	*sess_ctx = ctx;

	return TEE_SUCCESS;
}

static TEE_Result pta_vpu_write(vpu_session *session,uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS])
{
	TEE_Result res = TEE_SUCCESS;
	uint32_t exp_param_types;
	uint32_t offset, value;

	exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
					TEE_PARAM_TYPE_NONE,
					TEE_PARAM_TYPE_NONE,
					TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	vaddr_t ctrl_base = get_vpu_base(session);
	offset = params[0].value.a;
	value = params[0].value.b;

	if (!ctrl_base) {
		EMSG("Unable to get the VPU Base address");
		res = TEE_ERROR_ITEM_NOT_FOUND;
		goto out;
	}
	if (!IS_ALIGN(offset,sizeof(uint32_t)))
	{
		EMSG("Offset not aligned on 32bits (%d)",offset);
		res = TEE_ERROR_BAD_PARAMETERS;
		goto out;
	}

#if defined(CFG_SECURE_DATA_PATH) &&  defined(CFG_RDC_SECURE_DATA_PATH)
	if ((session) && ((offset == HANTRODEC_IRQ_STAT_DEC_OFF) && (value)))
	{
		paddr_t decoded_phys_addr = 0;
		paddr_t encoded_phys_addr = 0;
		bool isSecureContent = false;

		switch (session->coreId)
		{
			case 0:
				DMSG("VPU G1");
				encoded_phys_addr = io_read32(ctrl_base + HANTRODEC_G1_BASE_ADDR_IN_OFF);
				decoded_phys_addr = io_read32(ctrl_base + HANTRODEC_G1_BASE_ADDR_OUT_OFF);
				break;
			case 1:
			{
				uint32_t use_compression;
				uint32_t raster_enabled;

				DMSG("VPU G2");
				use_compression = (io_read32(ctrl_base + HANTRODEC_G2_DEC_CTRL_REG_0) & HANTRODEC_G2_HWIF_DEC_OUT_EC_BYPASS) == HANTRODEC_G2_HWIF_DEC_OUT_EC_BYPASS;
				raster_enabled = (io_read32(ctrl_base + HANTRODEC_G2_DEC_CTRL_REG_0) & HANTRODEC_G2_HWIF_DEC_OUT_RS_E) == HANTRODEC_G2_HWIF_DEC_OUT_RS_E;

				encoded_phys_addr = io_read32(ctrl_base + HANTRODEC_G2_BASE_ADDR_IN_OFF);

				if (raster_enabled)
					decoded_phys_addr = io_read32(ctrl_base + HANTRODEC_G2_BASE_ADDR_RS_OUT_OFF);
				else if (use_compression)
					decoded_phys_addr = io_read32(ctrl_base + HANTRODEC_G2_BASE_ADDR_COMPRESS_OUT_OFF);
				else
					decoded_phys_addr = io_read32(ctrl_base + HANTRODEC_G2_BASE_ADDR_OUT_OFF);
				break;
			}
			default:
				EMSG("Unknown VPU Core (%d)",session->coreId);
				return TEE_ERROR_BAD_PARAMETERS;
		}
		DMSG("encoded_phys_addr 0x%08X",encoded_phys_addr);
		DMSG("decoded_phys_addr 0x%08X",decoded_phys_addr);

		if ((encoded_phys_addr >= CFG_TEE_SDP_MEM_BASE) && (encoded_phys_addr < CFG_TEE_SDP_MEM_BASE + CFG_TEE_SDP_MEM_SIZE))
		{
			DMSG("Encoded video in SDP");
			isSecureContent = true;
		} else
			DMSG("Encoded video in non secure DDR");

		if ((decoded_phys_addr >= CFG_RDC_DECODED_BUFFER_BASE) && (decoded_phys_addr < CFG_RDC_DECODED_BUFFER_BASE + CFG_RDC_DECODED_BUFFER_SIZE))
			DMSG("Decoded video in SDP");
		else {
			DMSG("Decoded video in non secure DDR");
			if (isSecureContent)
			{
				EMSG("Decode secure content in non-secure memory is forbidden.");
				res = TEE_ERROR_ACCESS_DENIED;
				goto out;
			}
		}
	}
#endif
	io_write32(ctrl_base + offset, value);

out:
	return res;
}

static TEE_Result pta_vpu_write_multiple(vpu_session *session,uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS])
{
	TEE_Result res = TEE_SUCCESS;
	uint32_t exp_param_types;
	uint32_t offset, count, index;
	uint32_t *dec_regs = NULL;

	exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
					TEE_PARAM_TYPE_MEMREF_INPUT,
					TEE_PARAM_TYPE_NONE,
					TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	vaddr_t ctrl_base = get_vpu_base(session);
	offset = params[0].value.a;
	count = (params[1].memref.size)/sizeof(uint32_t);
	dec_regs = (uint32_t*)params[1].memref.buffer;

	if (!ctrl_base) {
		EMSG("Unable to get the VPU Base address");
		res = TEE_ERROR_ITEM_NOT_FOUND;
		goto out;
	}
	if (!IS_ALIGN(offset,sizeof(uint32_t)))
	{
		EMSG("Offset not aligned on 32bits (%d)",offset);
		res = TEE_ERROR_BAD_PARAMETERS;
		goto out;
	}

	// secure access is check when writing status register
	// prevent status register update to skip check operation
	if (offset > HANTRODEC_IRQ_STAT_DEC_OFF)
	{
		for (index = 0; index < count; index++)
		{
			io_write32(ctrl_base + offset, *dec_regs);
			offset += sizeof(uint32_t);
			dec_regs++;
		}
	} else {
		res = TEE_ERROR_ACCESS_DENIED;
	}

out:
	return res;
}

static TEE_Result pta_vpu_read(vpu_session *session,uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS])
{
	TEE_Result res = TEE_SUCCESS;
	uint32_t exp_param_types;
	uint32_t offset;

	exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
					TEE_PARAM_TYPE_VALUE_OUTPUT,
					TEE_PARAM_TYPE_NONE,
					TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	vaddr_t ctrl_base = get_vpu_base(session);
	offset = params[0].value.a;

	if (!ctrl_base) {
		EMSG("Unable to get the VPU Base address");
		res = TEE_ERROR_ITEM_NOT_FOUND;
		goto out;
	}
	if (!IS_ALIGN(offset,sizeof(uint32_t)))
	{
		EMSG("Offset not aligned on 32bits (%d)",offset);
		res = TEE_ERROR_BAD_PARAMETERS;
		goto out;
	}

	params[1].value.a = io_read32(ctrl_base + offset);

out:
	return res;
}

static TEE_Result pta_vpu_read_multiple(vpu_session *session,uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS])
{
	TEE_Result res = TEE_SUCCESS;
	uint32_t exp_param_types;
	uint32_t offset, count, index;
	uint32_t *dec_regs = NULL;

	exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
					TEE_PARAM_TYPE_MEMREF_OUTPUT,
					TEE_PARAM_TYPE_NONE,
					TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	vaddr_t ctrl_base = get_vpu_base(session);
	offset = params[0].value.a;
	count = (params[1].memref.size)/sizeof(uint32_t);
	dec_regs = (uint32_t*)params[1].memref.buffer;

	if (!ctrl_base) {
		EMSG("Unable to get the VPU Base address");
		res = TEE_ERROR_ITEM_NOT_FOUND;
		goto out;
	}
	if (!IS_ALIGN(offset,sizeof(uint32_t)))
	{
		EMSG("Offset not aligned on 32bits (%d)",offset);
		res = TEE_ERROR_BAD_PARAMETERS;
		goto out;
	}

	// secure access is check when writing status register
	// prevent status register update to skip check operation
	for (index = 0; index < count; index++)
	{
		*dec_regs = io_read32(ctrl_base + offset);
		offset += sizeof(uint32_t);
		dec_regs++;
	}

out:
	return res;
}


static TEE_Result pta_vpu_wait(vpu_session *session,uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS])
{
	TEE_Result res = TEE_SUCCESS;
	uint32_t exp_param_types;
	uint32_t irq_status_dec;
	uint32_t nbTimeWait = 0;
	bool     infinite = false;
#ifdef PERF_COUNTERS
	uint32_t pooling_loop = 0;
#endif
	exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
					TEE_PARAM_TYPE_NONE,
					TEE_PARAM_TYPE_NONE,
					TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	if (session == NULL)
		return TEE_ERROR_BAD_PARAMETERS;

	vaddr_t ctrl_base = get_vpu_base(session);
	nbTimeWait = params[0].value.a;

	if (!ctrl_base) {
		EMSG("Unable to get the VPU Base address");
		res = TEE_ERROR_ITEM_NOT_FOUND;
		goto out;
	}

	if (nbTimeWait == (uint32_t)(-1)) {
		infinite = true;
	} else {
		nbTimeWait = nbTimeWait * 2;
	}

	do
	{
#ifdef IT_MODE
		if (!session->wait_flag)
		{
			/* reactivate wait flag */
			session->wait_flag = 1;

			goto out;
		}
#else
		irq_status_dec = io_read32(ctrl_base + HANTRODEC_IRQ_STAT_DEC_OFF);

		if (irq_status_dec & HANTRODEC_DEC_IRQ) {
			/* clear dec IRQ */
			irq_status_dec &= (~HANTRODEC_DEC_IRQ);
			io_write32(ctrl_base + HANTRODEC_IRQ_STAT_DEC_OFF, irq_status_dec);

			goto out;
		}
#endif
		udelay(500);

#ifdef PERF_COUNTERS
		pooling_loop++;
#endif
	} while (infinite || (nbTimeWait--));

	res = TEE_ERROR_CANCEL;
out:
#ifdef PERF_COUNTERS
	params[0].value.b = pooling_loop*500;
#endif
	return res;
}

/**
 * @brief   Called when a pseudo TA is invoked.
 *
 * @param[in]  sess_ctx       Session Identifier
 * @param[in]  cmd_id         Command ID
 * @param[in]  param_types    TEE parameters
 * @param[in]  params         Buffer parameters
 *
 * @retval TEE_ERROR_BAD_PARAMETERS   Bad parameters
 */
static TEE_Result invokeCommandEntryPoint(void *sess_ctx,
	uint32_t cmd_id,
	uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS])
{
	vpu_session *session = (vpu_session*)sess_ctx;

	switch (cmd_id) {
	case PTA_HANTRO_VPU_CMD_WRITE:
		return pta_vpu_write(session,param_types,params);
	case PTA_HANTRO_VPU_CMD_WRITE_MULTIPLE:
		return pta_vpu_write_multiple(session,param_types,params);
	case PTA_HANTRO_VPU_CMD_READ_MULTIPLE:
		return pta_vpu_read_multiple(session,param_types,params);
	case PTA_HANTRO_VPU_CMD_READ:
		return pta_vpu_read(session,param_types,params);
	case PTA_HANTRO_VPU_CMD_WAIT:
		return pta_vpu_wait(session,param_types,params);
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}

static void closeSessionEntryPoint(void *sess_ctx)
{
	if (sess_ctx)
		free(sess_ctx);
}

pseudo_ta_register(.uuid = PTA_HANTRO_VPU_PTA_UUID,
	.name = HANTRO_VPU_PTA_NAME,
		   .flags = PTA_DEFAULT_FLAGS | TA_FLAG_SECURE_DATA_PATH,
		   .open_session_entry_point = openSessionEntryPoint,
		   .close_session_entry_point = closeSessionEntryPoint,
		   .invoke_command_entry_point = invokeCommandEntryPoint);
