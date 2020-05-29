// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright 2017-2020 NXP
 *
 */

#include <imx.h>
#include <initcall.h>
#include <io.h>
#include <kernel/panic.h>
#include <kernel/pm.h>
#include <mm/core_memprot.h>

#if TRACE_LEVEL >= TRACE_DEBUG
void csu_dump_state(void);
#else
static inline void csu_dump_state(void)
{
}
#endif

struct csu_setting {
	int csu_index;
	uint32_t value;
};

const struct csu_setting csu_setting_imx6[] = {
	{13, 0xFF0033},		/* Protect ROMCP */
	{16, 0x330033},		/* Protect TZASC */
	{26, 0xFF0033},		/* Protect OCRAM */
	{(-1), 0},
};

const struct csu_setting csu_setting_imx6ul[] = {
	{13, 0xFF0033},		/* Protect ROMCP */
	{16, 0x3300FF},		/* Protect TZASC */
	{39, 0x3300FF},		/* Protect OCRAM */
	{(-1), 0},
};

const struct csu_setting csu_setting_imx6sl[] = {
	{13, 0xFF0033},		/* Protect ROMCP */
	{16, 0xFF0033},		/* Protect TZASC */
	{26, 0xFF0033},		/* Protect OCRAM */
	{(-1), 0},
};

const struct csu_setting csu_setting_imx6sx[] = {
	{13, 0xFF0033},		/* Protect ROMCP */
	{15, 0xFF0033},		/* Protect RDC   */
	{16, 0x3300FF},		/* Protect TZASC */
	{34, 0x3300FF},		/* Protect OCRAM */
	{(-1), 0},
};

const struct csu_setting csu_setting_imx7ds[] = {
	{14, 0x3300FF},		/* Protect RDC     */
	{15, 0xFF0033},		/* Protect CSU     */
	{28, 0xFF0033},		/* Protect TZASC   */
	{59, 0x3300FF},		/* Protect OCRAM_S */
	{(-1), 0},
};

const struct csu_setting csu_setting_imx8m[] = {
	{14, 0x3300FF},		/* Protect RDC     */
	{15, 0xFF0033},		/* Protect CSU     */
	{28, 0xFF0033},		/* Protect TZASC   */
	{59, 0x3300FF},		/* Protect OCRAM_S */
	{(-1), 0},
};

static TEE_Result csu_configure(void)
{
	vaddr_t csu_base;
	vaddr_t offset;
	const struct csu_setting *csu_setting = NULL;

	csu_base = core_mmu_get_va(CSU_BASE, MEM_AREA_IO_SEC);
	if (!csu_base)
		panic();

	if (soc_is_imx6sx())
		csu_setting = csu_setting_imx6sx;
	else if (soc_is_imx6ul() || soc_is_imx6ull())
		csu_setting = csu_setting_imx6ul;
	else if (soc_is_imx6sll() || soc_is_imx6sl())
		csu_setting = csu_setting_imx6sl;
	else if (soc_is_imx6())
		csu_setting = csu_setting_imx6;
	else if (soc_is_imx7ds())
		csu_setting = csu_setting_imx7ds;
	else if (soc_is_imx8m())
		csu_setting = csu_setting_imx8m;
	else
		return TEE_SUCCESS;

	/* first grant all peripherals */
	for (offset = CSU_CSL_START; offset < CSU_CSL_END; offset += 4)
		io_write32(csu_base + offset, CSU_ACCESS_ALL);

	while (csu_setting->csu_index > 0) {
		io_write32(csu_base + (csu_setting->csu_index * 4),
				csu_setting->value);

		csu_setting++;
	}

	/* lock the settings */
	for (offset = CSU_CSL_START; offset < CSU_CSL_END; offset += 4) {
		io_write32(csu_base + offset,
			io_read32(csu_base + offset) | CSU_SETTING_LOCK);
	}

	csu_dump_state();

	return TEE_SUCCESS;
}

static TEE_Result
pm_enter_resume(enum pm_op op, uint32_t pm_hint __unused,
		const struct pm_callback_handle *pm_handle __unused)
{
	if (op == PM_OP_RESUME)
		csu_configure();

	return TEE_SUCCESS;
}

#if TRACE_LEVEL >= TRACE_DEBUG

void csu_dump_state(void)
{
	uint32_t n;
	uint32_t temp_32reg;
	vaddr_t csu_base;

	csu_base = core_mmu_get_va(CSU_BASE, MEM_AREA_IO_SEC);
	if (!csu_base)
		panic();

	DMSG("enter");
	for (n = CSU_CSL_START; n < CSU_CSL_END; n += 4) {
		temp_32reg = io_read32(csu_base + n);
		if ((temp_32reg == (CSU_ACCESS_ALL | CSU_SETTING_LOCK)))
			continue;

		DMSG("");
		DMSG("CSU_CSL%d", n/4);
		DMSG(" 0x%08X", temp_32reg);
	}
	DMSG("exit");
}

#endif /* CFG_TRACE_LEVEL >= TRACE_DEBUG */

static TEE_Result csu_init(void)
{
	csu_configure();
	register_pm_driver_cb(pm_enter_resume, NULL);

	return TEE_SUCCESS;
}
driver_init(csu_init);
