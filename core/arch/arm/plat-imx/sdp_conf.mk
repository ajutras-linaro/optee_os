ifeq ($(CFG_SECURE_DATA_PATH), y)

ifneq (,$(filter y, $(CFG_MX8MM) $(CFG_MX8MN) $(CFG_MX8MP) $(CFG_MX8MQ)))
CFG_IMX_TZC_SDP_START ?= ($(CFG_TEE_SDP_MEM_BASE) - $(CFG_DRAM_BASE))
else
CFG_IMX_TZC_SDP_START ?= $(CFG_TEE_SDP_MEM_BASE)
endif

# SDP layout configuration :
#  *  +----------------------------------+ <-- CFG_TEE_SDP_MEM_BASE
#  *  | SDP RAM                          |
#  *  +----------------------------------+ <-- CFG_TZDRAM_START = CFG_TEE_SDP_MEM_BASE + CFG_TEE_SDP_MEM_SIZE
#  *  | TEE core secure RAM (TEE_RAM)    |
#  *  +----------------------------------+
#  *  | Trusted Application RAM (TA_RAM) |
#  *  +----------------------------------+
CFG_IMX_TZC_SDP_SIZE ?= ($(CFG_TZDRAM_START) - $(CFG_TEE_SDP_MEM_BASE))
endif
