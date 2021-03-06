/* linux/arch/arm/plat-samsung/dev-mshc.c
 *
 * Copyright (c) 2008 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *	http://armlinux.simtec.co.uk/
 *
 * Based on arch/arm/plat-samsung/dev-hsmmc1.c
 *
 * Device definition for mshc devices
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/mmc/host.h>

#include <mach/map.h>
#include <plat/mshci.h>
#include <plat/devs.h>

#define S5P_SZ_MSHC	(0x1000)

static struct resource s3c_mshci_resource[] = {
	[0] = {
		.start = S5P_PA_MSHC,
		.end   = S5P_PA_MSHC + S5P_SZ_MSHC - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_MSHC,
		.end   = IRQ_MSHC,
		.flags = IORESOURCE_IRQ,
	}
};

static u64 s3c_device_hsmmc_dmamask = 0xffffffffUL;

struct s3c_mshci_platdata s3c_mshci_def_platdata = {
	.max_width	= 4,
	.host_caps	= (MMC_CAP_4_BIT_DATA |
#if defined(CONFIG_MMC_MSHC_CLOCK_GATING)
			MMC_CAP_CLOCK_GATING |
#endif
			   MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED),
};

struct platform_device s3c_device_mshci = {
	.name		= "s3c-mshci",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(s3c_mshci_resource),
	.resource	= s3c_mshci_resource,
	.dev		= {
		.dma_mask		= &s3c_device_hsmmc_dmamask,
		.coherent_dma_mask	= 0xffffffffUL,
		.platform_data		= &s3c_mshci_def_platdata,
	},
};

void s3c_mshci_set_platdata(struct s3c_mshci_platdata *pd)
{
	struct s3c_mshci_platdata *set = &s3c_mshci_def_platdata;

	set->cd_type = pd->cd_type;
	set->ext_cd_init = pd->ext_cd_init;
	set->ext_cd_cleanup = pd->ext_cd_cleanup;
	set->ext_cd_gpio = pd->ext_cd_gpio;
	set->ext_cd_gpio_invert = pd->ext_cd_gpio_invert;
	set->wp_gpio = pd->wp_gpio;
	set->has_wp_gpio = pd->has_wp_gpio;

	if (pd->max_width)
		set->max_width = pd->max_width;
	if (pd->host_caps)
		set->host_caps |= pd->host_caps;
	if (pd->cfg_gpio)
		set->cfg_gpio = pd->cfg_gpio;
	if (pd->cfg_card)
		set->cfg_card = pd->cfg_card;
#if defined (CONFIG_S5PV310_MSHC_VPLL_46MHZ) || \
	defined (CONFIG_S5PV310_MSHC_EPLL_45MHZ)
	if (pd->cfg_ddr)
		set->cfg_ddr= pd->cfg_ddr;
#endif
#ifdef CONFIG_MACH_C1
	if (pd->init_card)
		set->init_card= pd->init_card;
#endif

	if (pd->shutdown)
		set->shutdown = pd->shutdown;
}
