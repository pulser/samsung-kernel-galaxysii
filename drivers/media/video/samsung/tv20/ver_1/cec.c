/* linux/drivers/media/video/samsung/tv20/ver_1/cec.c
 *
 * Copyright (c) 2009 Samsung Electronics
 * 	http://www.samsung.com/
 *
 * cec ftn file for Samsung TVOut driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/slab.h>

#include <mach/map.h>
#include <mach/regs-clock.h>
#include <mach/regs-cec.h>

#include "../cec.h"
#include "cec.h"

#ifdef CECDEBUG
#define CECPRINTK(fmt, args...) \
	printk(KERN_INFO "\t\t[CEC] %s: " fmt, __func__ , ## args)
#else
#define CECPRINTK(fmt, args...)
#endif

static struct resource	*cec_mem;
void __iomem		*cec_base;

void s5p_cec_set_divider(void)
{
	u32 div_ratio, reg, div_val;

	div_ratio  = S5P_HDMI_FIN / CEC_DIV_RATIO - 1;

	reg = readl(S5P_HDMI_PHY_CONTROL);
	reg = (reg & ~(0x3FF << 16)) | (div_ratio << 16);

	writel(reg, S5P_HDMI_PHY_CONTROL);

	div_val = CEC_DIV_RATIO * 0.00005 - 1;

	writeb(0x0, cec_base + S5P_CES_DIVISOR_3);
	writeb(0x0, cec_base + S5P_CES_DIVISOR_2);
	writeb(0x0, cec_base + S5P_CES_DIVISOR_1);
	writeb(div_val, cec_base + S5P_CES_DIVISOR_0);
}

void s5p_cec_enable_rx(void)
{
	u8 reg;

	reg = readb(cec_base + S5P_CES_RX_CTRL);
	reg |= S5P_CES_RX_CTRL_ENABLE;
	writeb(reg, cec_base + S5P_CES_RX_CTRL);
}

void s5p_cec_mask_rx_interrupts(void)
{
	u8 reg;

	reg = readb(cec_base + S5P_CES_IRQ_MASK);
	reg |= S5P_CES_IRQ_RX_DONE;
	reg |= S5P_CES_IRQ_RX_ERROR;
	writeb(reg, cec_base + S5P_CES_IRQ_MASK);
}

void s5p_cec_unmask_rx_interrupts(void)
{
	u8 reg;

	reg = readb(cec_base + S5P_CES_IRQ_MASK);
	reg &= ~S5P_CES_IRQ_RX_DONE;
	reg &= ~S5P_CES_IRQ_RX_ERROR;
	writeb(reg, cec_base + S5P_CES_IRQ_MASK);
}

void s5p_cec_mask_tx_interrupts(void)
{
	u8 reg;
	reg = readb(cec_base + S5P_CES_IRQ_MASK);
	reg |= S5P_CES_IRQ_TX_DONE;
	reg |= S5P_CES_IRQ_TX_ERROR;
	writeb(reg, cec_base + S5P_CES_IRQ_MASK);

}

void s5p_cec_unmask_tx_interrupts(void)
{
	u8 reg;

	reg = readb(cec_base + S5P_CES_IRQ_MASK);
	reg &= ~S5P_CES_IRQ_TX_DONE;
	reg &= ~S5P_CES_IRQ_TX_ERROR;
	writeb(reg, cec_base + S5P_CES_IRQ_MASK);
}

void s5p_cec_reset(void)
{
	writeb(S5P_CES_RX_CTRL_RESET, cec_base + S5P_CES_RX_CTRL);
	writeb(S5P_CES_TX_CTRL_RESET, cec_base + S5P_CES_TX_CTRL);
}

void s5p_cec_tx_reset(void)
{
	writeb(S5P_CES_TX_CTRL_RESET, cec_base + S5P_CES_TX_CTRL);
}

void s5p_cec_rx_reset(void)
{
	writeb(S5P_CES_RX_CTRL_RESET, cec_base + S5P_CES_RX_CTRL);
}

void s5p_cec_threshold(void)
{
	writeb(CEC_FILTER_THRESHOLD, cec_base + S5P_CES_RX_FILTER_TH);
	writeb(0, cec_base + S5P_CES_RX_FILTER_CTRL);
}

void s5p_cec_copy_packet(char *data, size_t count)
{
	int i = 0;
	u8 reg;

	while (i < count) {
		writeb(data[i], cec_base + (S5P_CES_TX_BUFF0 + (i * 4)));
		i++;
	}

	writeb(count, cec_base + S5P_CES_TX_BYTES);
	s5p_cec_set_tx_state(STATE_TX);
	reg = readb(cec_base + S5P_CES_TX_CTRL);
	reg |= S5P_CES_TX_CTRL_START;

	if ((data[0] & CEC_MESSAGE_BROADCAST_MASK) == CEC_MESSAGE_BROADCAST)
		reg |= S5P_CES_TX_CTRL_BCAST;
	else
		reg &= ~S5P_CES_TX_CTRL_BCAST;

	reg |= 0x50;
	writeb(reg, cec_base + S5P_CES_TX_CTRL);
}

void s5p_cec_set_addr(u32 addr)
{
	writeb(addr & 0x0F, cec_base + S5P_CES_LOGIC_ADDR);
}

u32 s5p_cec_get_status(void)
{
	u32 status = 0;

	status = readb(cec_base + S5P_CES_STATUS_0);
	status |= readb(cec_base + S5P_CES_STATUS_1) << 8;
	status |= readb(cec_base + S5P_CES_STATUS_2) << 16;
	status |= readb(cec_base + S5P_CES_STATUS_3) << 24;

	CECPRINTK("status = 0x%x!\n", status);

	return status;
}

void s5p_clr_pending_tx(void)
{
	writeb(S5P_CES_IRQ_TX_DONE | S5P_CES_IRQ_TX_ERROR,
					cec_base + S5P_CES_IRQ_CLEAR);
}

void s5p_clr_pending_rx(void)
{
	writeb(S5P_CES_IRQ_RX_DONE | S5P_CES_IRQ_RX_ERROR,
					cec_base + S5P_CES_IRQ_CLEAR);
}

void s5p_cec_get_rx_buf(u32 size, u8 *buffer)
{
	u32 i = 0;

	while (i < size) {
		buffer[i] = readb(cec_base + S5P_CES_RX_BUFF0 + (i * 4));
		i++;
	}
}

void __init s5p_cec_mem_probe(struct platform_device *pdev)
{
	struct resource *res;
	size_t	size;
	int 	ret;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	if (res == NULL) {
		dev_err(&pdev->dev,
			"failed to get memory region resource for cec\n");
		ret = -ENOENT;
	}

	size = (res->end - res->start) + 1;
	cec_mem = request_mem_region(res->start, size, pdev->name);

	if (cec_mem == NULL) {
		dev_err(&pdev->dev,
			"failed to get memory region for cec\n");
		ret = -ENOENT;
	}

	cec_base = ioremap(res->start, size);

	if (cec_base == NULL) {
		dev_err(&pdev->dev,
			"failed to ioremap address region for cec\n");
		ret = -ENOENT;
	}
}

int __init s5p_cec_mem_release(struct platform_device *pdev)
{
	iounmap(cec_base);

	if (cec_mem != NULL) {
		if (release_resource(cec_mem))
			dev_err(&pdev->dev,
				"Can't remove tvout drv !!\n");

		kfree(cec_mem);

		cec_mem = NULL;
	}

	return 0;
}