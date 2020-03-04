// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2019, NXP
 */
#include <compiler.h>
#include <stdio.h>
#include <trace.h>
#include <kernel/pseudo_ta.h>
#include <tee_fs.h>

#define TA_NAME		"config.ta"

#define TA_CONFIG_UUID \
		{ 0x57e72d42, 0xdd00, 0x4930, \
			{ 0xb6, 0xfa, 0x31, 0xe4, 0x7e, 0x91, 0x96, 0xc0 } }

#define CONFIG_CMD_SET_RPMB_CONFIG 0

#define CONFIG_RPMB_UNINITIALIZED 0
#define CONFIG_RPMB_ALLOWED		  1
#define CONFIG_RPMB_BLOCKED 	  2

static unsigned int wConfig = CONFIG_RPMB_UNINITIALIZED;

#ifdef CFG_RPMB_FS
static TEE_Result set_rpmb_config(uint32_t type, TEE_Param p[TEE_NUM_PARAMS])
{
	unsigned int config = CONFIG_RPMB_UNINITIALIZED;

	if (TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
			    TEE_PARAM_TYPE_NONE,
			    TEE_PARAM_TYPE_NONE,
			    TEE_PARAM_TYPE_NONE) != type) {
		return TEE_ERROR_BAD_PARAMETERS;
	}

	if(wConfig != CONFIG_RPMB_UNINITIALIZED) {
		EMSG("RPMB configuration is already set");
		return TEE_ERROR_ACCESS_DENIED;
	}

	config = p[0].value.a;
	if(config == CONFIG_RPMB_ALLOWED) {
		wConfig = CONFIG_RPMB_ALLOWED;
		tee_rpmb_fs_configure(false); /* allowed */
	} else {
		wConfig = CONFIG_RPMB_BLOCKED;
		tee_rpmb_fs_configure(true); /* blocked */
	}

	return TEE_SUCCESS;
}
#endif
/*
 * Trusted Application Entry Points
 */

static TEE_Result invoke_command(void *psess __unused,
				 uint32_t cmd, uint32_t ptypes,
				 TEE_Param params[TEE_NUM_PARAMS])
{
	switch (cmd) {
	case CONFIG_CMD_SET_RPMB_CONFIG:
#ifdef CFG_RPMB_FS
		return set_rpmb_config(ptypes, params);
#else
		return TEE_ERROR_NOT_SUPPORTED;
#endif
	default:
		break;
	}
	return TEE_ERROR_BAD_PARAMETERS;
}

pseudo_ta_register(.uuid = TA_CONFIG_UUID, .name = TA_NAME,
		   .flags = PTA_DEFAULT_FLAGS,
		   .invoke_command_entry_point = invoke_command);
