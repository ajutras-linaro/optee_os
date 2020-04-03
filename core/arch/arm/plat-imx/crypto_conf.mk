#
# Define the cryptographic algorithm to be built
#

#
# CAAM Debug: define 3x32 bits value (same bit used to debug a module)
# CFG_DBG_CAAM_TRACE  Module print trace
# CFG_DBG_CAAM_DESC   Module descriptor dump
# CFG_DBG_CAAM_BUF    Module buffer dump
#
# DBG_HAL    BIT32(0)  // HAL trace
# DBG_CTRL   BIT32(1)  // Controller trace
# DBG_MEM    BIT32(2)  // Memory utility trace
# DBG_SGT    BIT32(3)  // Scatter Gather trace
# DBG_PWR    BIT32(4)  // Power trace
# DBG_JR     BIT32(5)  // Job Ring trace
# DBG_RNG    BIT32(6)  // RNG trace
# DBG_HASH   BIT32(7)  // Hash trace
# DBG_BLOB   BIT32(8)  // BLOB trace
# DBG_CIPHER BIT32(9)  // Cipher trace
# DBG_ECC    BIT32(10) // ECC trace
# DBG_RSA    BIT32(11) // RSA trace
# DBG_MP     BIT32(12) // MP trace
# DBG_SM     BIT32(13) // Secure Memory trace
# DBG_DH     BIT32(14) // DH Trace
CFG_DBG_CAAM_TRACE ?= 0x2
CFG_DBG_CAAM_DESC ?= 0x0
CFG_DBG_CAAM_BUF ?= 0x0

#
# CAAM Job Ring configuration
#  - Normal boot settings
#  - HAB support boot settings
#
$(call force, CFG_JR_BLOCK_SIZE,0x1000)

ifneq (,$(filter y, $(CFG_MX8MM) $(CFG_MX8MN) $(CFG_MX8MP) $(CFG_MX8MQ)))
# On i.MX8 mscale devices OP-TEE runs before u-boot.
# HAB can still be reuse in u-boot to authenticate linux
# Use another Job ring other than the one used by HAB.
$(call force, CFG_JR_INDEX,2)  # Default JR index used
$(call force, CFG_JR_INT,146)  # Default JR IT Number (114 + 32) = 146
else
$(call force, CFG_JR_INDEX,0)  # Default JR index used
$(call force, CFG_JR_INT,137)  # Default JR IT Number (105 + 32) = 137
endif

#
# Enable HUK CAAM Generation
#
CFG_NXP_CAAM_HUK_DRV ?= y

#
# Configuration of the Crypto Driver
#
ifeq ($(CFG_CRYPTO_DRIVER), y)

$(call force, CFG_NXP_CAAM_RUNTIME_JR, y)

#
# Definition of all HW accelerations for all i.MX
#
$(call force, CFG_NXP_CAAM_RNG_DRV, y)
$(call force, CFG_WITH_SOFTWARE_PRNG,n)

# Force to 'y' the CFG_NXP_CAAM_xxx_DRV to enable the CAAM HW driver
# and enable the associated CFG_CRYPTO_DRV_xxx Crypto driver
# API
#
# Example: Enable CFG_CRYPTO_DRV_HASH and CFG_NXP_CAAM_HASH_DRV
#     $(eval $(call cryphw-enable-drv-hw, HASH))
define cryphw-enable-drv-hw
_var := $(strip $(1))
$$(call force, CFG_NXP_CAAM_$$(_var)_DRV, y)
$$(call force, CFG_CRYPTO_DRV_$$(_var), y)
endef

# Return 'y' if at least one of the variable
# CFG_CRYPTO_xxx_HW is 'y'
cryphw-one-enabled = $(call cfg-one-enabled, \
                        $(foreach v,$(1), CFG_NXP_CAAM_$(v)_DRV))


# Definition of the HW and Cryto Driver Algorithm supported by all i.MX
$(eval $(call cryphw-enable-drv-hw, HASH))
$(eval $(call cryphw-enable-drv-hw, SM))
$(eval $(call cryphw-enable-drv-hw, BLOB))
$(eval $(call cryphw-enable-drv-hw, HMAC))

ifneq ($(filter y, $(CFG_MX8MQ) $(CFG_MX8MM)), y)
$(eval $(call cryphw-enable-drv-hw, CIPHER))
$(eval $(call cryphw-enable-drv-hw, CMAC))
endif

ifneq ($(filter y, $(CFG_MX6QP) $(CFG_MX6Q) $(CFG_MX6D) $(CFG_MX6DL) \
	$(CFG_MX6S) $(CFG_MX6SX) $(CFG_MX7ULP)), y)
$(eval $(call cryphw-enable-drv-hw, MP))
$(eval $(call cryphw-enable-drv-hw, DH))
CFG_PTA_MP ?= y
endif

ifneq ($(filter y, $(CFG_MX6QP) $(CFG_MX6Q) $(CFG_MX6D) $(CFG_MX6DL) \
	$(CFG_MX6S) $(CFG_MX6SX) $(CFG_MX7ULP) $(CFG_MX8MQ) $(CFG_MX8MM)), y)
$(eval $(call cryphw-enable-drv-hw, ECC))
$(eval $(call cryphw-enable-drv-hw, RSA))
endif

$(call force, CFG_NXP_CAAM_ACIPHER_DRV, $(call cryphw-one-enabled, ECC RSA DH))

ifeq ($(CFG_IMX_DEK_HAB),y)
CFG_PTA_DEK ?= y
endif

#
# Enable Cryptographic Driver interface
#
CFG_CRYPTO_DRV_ACIPHER ?= $(CFG_NXP_CAAM_ACIPHER_DRV)
$(call force, CFG_CRYPTO_DRV_MAC, $(call cryphw-one-enabled, HMAC CMAC))
endif
