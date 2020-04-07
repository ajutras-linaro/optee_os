/**
 * @copyright 2019 NXP
 *
 * @file    pta_imx_hdcp.c
 *
 * @brief   Pseudo Trusted Application.\n
 *			Secure HDCP control (i.MX 8MQ platform)
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
#include <mm/core_mmu.h>
#include <mm/core_memprot.h>

#include <pta_imx_hdcp.h>

/* Library tee includes */
#include <tee_api_types.h>

/* Global includes */
#include <tee_api_defines.h>

/** @brief    PTA name */
#define HDCP_PTA_NAME "hdcp.pta"

#define IS_ALIGN(x,b) (!(x & (b - 1)))

static vaddr_t get_hdcp_base(uint32_t busId);

static vaddr_t get_hdcp_base(uint32_t busId)
{
	vaddr_t vhdcp_base = (vaddr_t) NULL;
	paddr_t phdcp_base = (paddr_t) NULL;
	size_t phdcp_size = 0;

	switch (busId)
	{
		case PTA_HDCP_HDP_BUSID:
			phdcp_base = PTA_HDCP_HDP_REGISTER_BA;
			phdcp_size = PTA_HDCP_HDP_REGISTER_SIZE;
		break;

		case PTA_HDCP_HDP_SEC_BUSID:
			phdcp_base = PTA_HDCP_HDP_SEC_REGISTER_BA;
			phdcp_size = PTA_HDCP_HDP_SEC_REGISTER_SIZE;
		break;

		case PTA_HDCP_RESET_BUSID:
			phdcp_base = PTA_HDCP_RESET_REGISTER_BA;
			phdcp_size = PTA_HDCP_RESET_REGISTER_SIZE;
		break;

		default:
			EMSG("Unknown bus");
			return (vaddr_t) NULL;
	}

	vhdcp_base = (vaddr_t) phys_to_virt(phdcp_base, MEM_AREA_IO_SEC);
	if (!vhdcp_base)
	{
		DMSG("Map HDCP Registers for bus Id %d", busId);
		if (!core_mmu_add_mapping(MEM_AREA_IO_SEC, phdcp_base, ROUNDUP(phdcp_size, CORE_MMU_PGDIR_SIZE)))
                {
			EMSG("Unable to map HDCP Registers for busId %d", busId );
			goto out;
		}
		vhdcp_base = (vaddr_t) phys_to_virt(phdcp_base, MEM_AREA_IO_SEC);
	}

out:
	return (vaddr_t) vhdcp_base;
}

static TEE_Result openSessionEntryPoint(uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS], void **sess_ctx)
{
	uint32_t exp_param_types;
	hdcp_session *ctx = NULL;

	exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
					TEE_PARAM_TYPE_NONE,
					TEE_PARAM_TYPE_NONE,
					TEE_PARAM_TYPE_NONE);

	DMSG("IMX HDCP openSessionEntryPoint");

	if ((param_types != exp_param_types))
	{
		EMSG("BAD PARAMETERS");
		return TEE_ERROR_BAD_PARAMETERS;
	}

	ctx = malloc(sizeof(hdcp_session));
	if (!ctx)
	{
		return TEE_ERROR_OUT_OF_MEMORY;
	}
	memset(ctx,0,sizeof(hdcp_session));

	ctx->busId = params[0].value.a;

	DMSG("Open session for bus %d",ctx->busId);

	*sess_ctx = ctx;

	return TEE_SUCCESS;
}

static TEE_Result pta_hdcp_write(hdcp_session *session,uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS])
{
	TEE_Result res = TEE_SUCCESS;
	uint32_t exp_param_types;
	uint32_t offset, value, busId;

	exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
					TEE_PARAM_TYPE_VALUE_INPUT,
					TEE_PARAM_TYPE_NONE,
					TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types)
	{
		EMSG("BAD PARAMETERS");
		return TEE_ERROR_BAD_PARAMETERS;
	}

	busId = params[0].value.a;
	offset = params[1].value.a;
	value = params[1].value.b;

	vaddr_t hdcp_base = get_hdcp_base(busId);
	if (!hdcp_base)
        {
		EMSG("Unable to get the HDCP base address");
		res = TEE_ERROR_ITEM_NOT_FOUND;
		goto out;
	}

	if (!IS_ALIGN(offset,sizeof(uint32_t)))
	{
		EMSG("Offset not aligned on 32bits (%d)",offset);
		res = TEE_ERROR_BAD_PARAMETERS;
		goto out;
	}

	switch (busId)
	{
		case PTA_HDCP_HDP_BUSID:
			DMSG("HDCP bus");
			io_write32(hdcp_base + offset, value);
		break;

		case PTA_HDCP_HDP_SEC_BUSID:
			DMSG("HDCP SECURE bus");
			io_write32(hdcp_base + offset, value);
		break;

		case PTA_HDCP_RESET_BUSID:
			DMSG("HDCP RESET bus");
			io_write32(hdcp_base + offset, value);
		break;

		default:
			EMSG("Unknown HDCP bus (%d)",session->busId);
			return TEE_ERROR_BAD_PARAMETERS;
	}

out:
	return res;
}

static TEE_Result pta_hdcp_read(hdcp_session *session, uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS])
{
	TEE_Result res = TEE_SUCCESS;
	uint32_t exp_param_types;
	uint32_t offset, busId;

	exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
					TEE_PARAM_TYPE_VALUE_OUTPUT,
					TEE_PARAM_TYPE_NONE,
					TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types)
	{
		EMSG("BAD PARAMETERS");
		return TEE_ERROR_BAD_PARAMETERS;
	}

	busId = params[0].value.a;
	vaddr_t hdcp_base = get_hdcp_base(busId);
	if (!hdcp_base)
	{
		EMSG("Unable to get the HDCP base address for busId %d",busId);
		res = TEE_ERROR_ITEM_NOT_FOUND;
		goto out;
	}

	offset = params[0].value.b;
	if (!IS_ALIGN(offset,sizeof(uint32_t)))
	{
		EMSG("Offset not aligned on 32bits (%d)",offset);
		res = TEE_ERROR_BAD_PARAMETERS;
		goto out;
	}

	params[1].value.a = io_read32(hdcp_base + offset);

out:
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
static TEE_Result invokeCommandEntryPoint(void *sess_ctx, uint32_t cmd_id,
	uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS])
{
	hdcp_session *session = (hdcp_session*)sess_ctx;

	switch (cmd_id)
        {
		case PTA_HDCP_CMD_WRITE:
			return pta_hdcp_write(session,param_types,params);

		case PTA_HDCP_CMD_READ:
			return pta_hdcp_read(session,param_types,params);

		default:
			EMSG("Unknown command");
			return TEE_ERROR_BAD_PARAMETERS;
	}
}

static void closeSessionEntryPoint(void *sess_ctx)
{
	if (sess_ctx)
	{
		free(sess_ctx);
	}
}

pseudo_ta_register(.uuid = HDCP_PTA_UUID,
		   .name = HDCP_PTA_NAME,
		   .flags = PTA_DEFAULT_FLAGS,
		   .open_session_entry_point = openSessionEntryPoint,
		   .close_session_entry_point = closeSessionEntryPoint,
		   .invoke_command_entry_point = invokeCommandEntryPoint);
