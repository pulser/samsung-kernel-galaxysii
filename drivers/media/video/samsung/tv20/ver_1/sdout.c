/* linux/drivers/media/video/samsung/tv20/ver_1/sdout.c
 *
 * Copyright (c) 2009 Samsung Electronics
 * 	http://www.samsung.com/
 *
 * tv encoder raw ftn  file for Samsung TVOut driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/memory.h>
#include <linux/slab.h>

#include <plat/clock.h>
#include <mach/regs-clock.h>

#include "tv_out.h"

#include <mach/regs-sdaout.h>

#ifdef COFIG_TVOUT_RAW_DBG
#define S5P_SDAOUT_DEBUG 1
#endif

#ifdef S5P_SDAOUT_DEBUG
#define SDPRINTK(fmt, args...)	\
	printk(KERN_INFO "\t\t[SDOUT] %s: " fmt, __func__ , ## args)
#else
#define SDPRINTK(fmt, args...)
#endif

static struct resource	*sdout_mem;
void __iomem		*sdout_base;

void s5p_tv_power_set_dac_onoff(unsigned short on)
{
	if (on)
		writel(S5P_DAC_ENABLE, S5P_DAC_CONTROL);
	else
		writel(S5P_DAC_DISABLE, S5P_DAC_CONTROL);
}

int s5p_sdout_init_video_scale_cfg(enum s5p_sd_level component_level,
				enum s5p_sd_vsync_ratio component_ratio,
				enum s5p_sd_level composite_level,
				enum s5p_sd_vsync_ratio composite_ratio)
{
	u32 temp_reg = 0;

	SDPRINTK("%d, %d, %d, %d\n\r", component_level, component_ratio,
		composite_level, composite_ratio);

	switch (component_level) {
	case S5P_TV_SD_LEVEL_0IRE:
		temp_reg = S5P_SDO_COMPONENT_LEVEL_SEL_0IRE;
		break;

	case S5P_TV_SD_LEVEL_75IRE:
		temp_reg = S5P_SDO_COMPONENT_LEVEL_SEL_75IRE;
		break;

	default:
		SDPRINTK("invalid component_level parameter(%d)\n\r",
			component_level);
		return -1;
	}

	switch (composite_level) {
	case SDOUT_VTOS_RATIO_10_4:
		temp_reg |= S5P_SDO_COMPONENT_VTOS_RATIO_10_4;
		break;

	case SDOUT_VTOS_RATIO_7_3:
		temp_reg |= S5P_SDO_COMPONENT_VTOS_RATIO_7_3;
		break;

	default:
		SDPRINTK(" invalid composite_level parameter(%d)\n\r",
			composite_level);
		return -1;
	}

	switch (composite_level) {
	case S5P_TV_SD_LEVEL_0IRE:
		temp_reg |= S5P_SDO_COMPOSITE_LEVEL_SEL_0IRE;
		break;

	case S5P_TV_SD_LEVEL_75IRE:
		temp_reg |= S5P_SDO_COMPOSITE_LEVEL_SEL_75IRE;
		break;

	default:
		SDPRINTK("invalid composite_ratio parameter(%d)\n\r",
			composite_ratio);
		return -1;
	}

	switch (composite_ratio) {
	case SDOUT_VTOS_RATIO_10_4:
		temp_reg |= S5P_SDO_COMPOSITE_VTOS_RATIO_10_4;
		break;

	case SDOUT_VTOS_RATIO_7_3:
		temp_reg |= S5P_SDO_COMPOSITE_VTOS_RATIO_7_3;
		break;

	default:
		SDPRINTK("invalid component_ratio parameter(%d)\n\r",
			component_ratio);
		return -1;
	}

	writel(temp_reg, sdout_base + S5P_SDO_SCALE);

	return 0;
}

int s5p_sdout_init_sync_signal_pin(enum s5p_sd_sync_sig_pin pin)
{
	SDPRINTK("%d\n\r", pin);

	switch (pin) {
	case SDOUT_SYNC_SIG_NO:
		writel(S5P_SDO_COMPONENT_SYNC_ABSENT,
			sdout_base + S5P_SDO_SYNC);
		break;

	case SDOUT_SYNC_SIG_YG:
		writel(S5P_SDO_COMPONENT_SYNC_YG,
			sdout_base + S5P_SDO_SYNC);
		break;

	case SDOUT_SYNC_SIG_ALL:
		writel(S5P_SDO_COMPONENT_SYNC_ALL,
			sdout_base + S5P_SDO_SYNC);
		break;

	default:
		SDPRINTK("invalid pin parameter(%d)\n\r", pin);
		return -1;
	}

	return 0;
}

int s5p_sdout_init_vbi(bool wss_cvbs,
			enum s5p_sd_closed_caption_type caption_cvbs,
			bool wss_y_sideo,
			enum s5p_sd_closed_caption_type caption_y_sideo,
			bool cgmsa_rgb,
			bool wss_rgb,
			enum s5p_sd_closed_caption_type caption_rgb,
			bool cgmsa_y_ppr,
			bool wss_y_ppr,
			enum s5p_sd_closed_caption_type caption_y_ppr)
{
	u32 temp_reg = 0;

	SDPRINTK(" %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n\r",
		wss_cvbs, caption_cvbs,
		wss_y_sideo, caption_y_sideo, cgmsa_rgb, wss_rgb, caption_rgb,
		cgmsa_y_ppr, wss_y_ppr, caption_y_ppr);

	if (wss_cvbs)
		temp_reg = S5P_SDO_CVBS_WSS_INS;
	else
		temp_reg = S5P_SDO_CVBS_NO_WSS;


	switch (caption_cvbs) {
	case SDOUT_NO_INS:
		temp_reg |= S5P_SDO_CVBS_NO_CLOSED_CAPTION;
		break;

	case SDOUT_INS_1:
		temp_reg |= S5P_SDO_CVBS_21H_CLOSED_CAPTION;
		break;

	case SDOUT_INS_2:
		temp_reg |= S5P_SDO_CVBS_21H_284H_CLOSED_CAPTION;
		break;

	case SDOUT_INS_OTHERS:
		temp_reg |= S5P_SDO_CVBS_USE_OTHERS;
		break;

	default:
		SDPRINTK(" invalid caption_cvbs parameter(%d)\n\r",
			caption_cvbs);
		return -1;
	}

	if (wss_y_sideo)
		temp_reg |= S5P_SDO_SVIDEO_WSS_INS;
	else
		temp_reg |= S5P_SDO_SVIDEO_NO_WSS;


	switch (caption_y_sideo) {
	case SDOUT_NO_INS:
		temp_reg |= S5P_SDO_SVIDEO_NO_CLOSED_CAPTION;
		break;

	case SDOUT_INS_1:
		temp_reg |= S5P_SDO_SVIDEO_21H_CLOSED_CAPTION;
		break;

	case SDOUT_INS_2:
		temp_reg |= S5P_SDO_SVIDEO_21H_284H_CLOSED_CAPTION;
		break;

	case SDOUT_INS_OTHERS:
		temp_reg |= S5P_SDO_SVIDEO_USE_OTHERS;
		break;

	default:
		SDPRINTK("invalid caption_y_sideo parameter(%d)\n\r",
			caption_y_sideo);
		return -1;
	}

	if (cgmsa_rgb)
		temp_reg |= S5P_SDO_RGB_CGMSA_INS;
	else
		temp_reg |= S5P_SDO_RGB_NO_CGMSA;


	if (wss_rgb)
		temp_reg |= S5P_SDO_RGB_WSS_INS;
	else
		temp_reg |= S5P_SDO_RGB_NO_WSS;


	switch (caption_rgb) {
	case SDOUT_NO_INS:
		temp_reg |= S5P_SDO_RGB_NO_CLOSED_CAPTION;
		break;

	case SDOUT_INS_1:
		temp_reg |= S5P_SDO_RGB_21H_CLOSED_CAPTION;
		break;

	case SDOUT_INS_2:
		temp_reg |= S5P_SDO_RGB_21H_284H_CLOSED_CAPTION;
		break;

	case SDOUT_INS_OTHERS:
		temp_reg |= S5P_SDO_RGB_USE_OTHERS;
		break;

	default:
		SDPRINTK(" invalid caption_rgb parameter(%d)\n\r",
			caption_rgb);
		return -1;
	}

	if (cgmsa_y_ppr)
		temp_reg |= S5P_SDO_YPBPR_CGMSA_INS;
	else
		temp_reg |= S5P_SDO_YPBPR_NO_CGMSA;


	if (wss_y_ppr)
		temp_reg |= S5P_SDO_YPBPR_WSS_INS;
	else
		temp_reg |= S5P_SDO_YPBPR_NO_WSS;


	switch (caption_y_ppr) {
	case SDOUT_NO_INS:
		temp_reg |= S5P_SDO_YPBPR_NO_CLOSED_CAPTION;
		break;

	case SDOUT_INS_1:
		temp_reg |= S5P_SDO_YPBPR_21H_CLOSED_CAPTION;
		break;

	case SDOUT_INS_2:
		temp_reg |= S5P_SDO_YPBPR_21H_284H_CLOSED_CAPTION;
		break;

	case SDOUT_INS_OTHERS:
		temp_reg |= S5P_SDO_YPBPR_USE_OTHERS;
		break;

	default:
		SDPRINTK("invalid caption_y_ppr parameter(%d)\n\r",
			caption_y_ppr);
		return -1;
	}

	writel(temp_reg, sdout_base + S5P_SDO_VBI);

	return 0;
}

int s5p_sdout_init_offset_gain(enum s5p_sd_channel_sel channel,
				u32 offset, u32 gain)
{
	SDPRINTK("%d, %d, %d\n\r", channel, offset, gain);

	switch (channel) {
	case SDOUT_CHANNEL_0:
		writel(S5P_SDO_SCALE_CONV_OFFSET(offset) |
			S5P_SDO_SCALE_CONV_GAIN(gain),
		       sdout_base + S5P_SDO_SCALE_CH0);
		SDPRINTK("0x%08x\n\r", readl(sdout_base + S5P_SDO_SCALE_CH0));
		break;

	case SDOUT_CHANNEL_1:
		writel(S5P_SDO_SCALE_CONV_OFFSET(offset) |
			S5P_SDO_SCALE_CONV_GAIN(gain),
		       sdout_base + S5P_SDO_SCALE_CH1);
		SDPRINTK(" 0x%08x\n\r", readl(sdout_base + S5P_SDO_SCALE_CH1));
		break;

	case SDOUT_CHANNEL_2:
		writel(S5P_SDO_SCALE_CONV_OFFSET(offset) |
			S5P_SDO_SCALE_CONV_GAIN(gain),
		       sdout_base + S5P_SDO_SCALE_CH2);
		SDPRINTK(" 0x%08x\n\r", readl(sdout_base + S5P_SDO_SCALE_CH2));
		break;

	default:
		SDPRINTK(" invalid channel parameter(%d)\n\r", channel);
		return -1;
	}

	return 0;
}

void s5p_sdout_init_delay(u32 delay_y,
			u32 offset_video_start,
			u32 offset_video_end)
{
	SDPRINTK("%d, %d, %d\n\r", delay_y, offset_video_start,
		offset_video_end);

	writel(S5P_SDO_DELAY_YTOC(delay_y) |
		S5P_SDO_ACTIVE_START_OFFSET(offset_video_start) |
		S5P_SDO_ACTIVE_END_OFFSET(offset_video_end),
		sdout_base + S5P_SDO_YCDELAY);
}

void s5p_sdout_init_schlock(bool color_sucarrier_pha_adj)
{
	SDPRINTK("%d\n\r", color_sucarrier_pha_adj);

	if (color_sucarrier_pha_adj)
		writel(S5P_SDO_COLOR_SC_PHASE_ADJ,
			sdout_base + S5P_SDO_SCHLOCK);
	else
		writel(S5P_SDO_COLOR_SC_PHASE_NOADJ,
			sdout_base + S5P_SDO_SCHLOCK);

}

int s5p_sdout_init_dac_power_onoff(enum s5p_sd_channel_sel channel, bool dac_on)
{
	u32 temp_on_off;

	SDPRINTK("%d, %d)\n\r", channel, dac_on);

	switch (channel) {
	case SDOUT_CHANNEL_0:
		temp_on_off = S5P_SDO_POWER_ON_DAC0;
		break;

	case SDOUT_CHANNEL_1:
		temp_on_off = S5P_SDO_POWER_ON_DAC1;
		break;

	case SDOUT_CHANNEL_2:
		temp_on_off = S5P_SDO_POWER_ON_DAC2;
		break;

	default:
		SDPRINTK("invalid channel parameter(%d)\n\r", channel);
		return -1;
	}

	if (dac_on)
		writel(readl(sdout_base + S5P_SDO_DAC) | temp_on_off,
			sdout_base + S5P_SDO_DAC);
	else
		writel(readl(sdout_base + S5P_SDO_DAC) & ~temp_on_off,
			sdout_base + S5P_SDO_DAC);

	return 0;
}

void s5p_sdout_init_color_compensaton_onoff(bool bright_hue_saturation_adj,
					bool y_ppr_color_compensation,
					bool rgcolor_compensation,
					bool y_c_color_compensation,
					bool y_cvbs_color_compensation)
{
	u32 temp_reg = 0;

	SDPRINTK("%d, %d, %d, %d, %d)\n\r", bright_hue_saturation_adj,
		y_ppr_color_compensation, rgcolor_compensation,
		y_c_color_compensation, y_cvbs_color_compensation);

	if (bright_hue_saturation_adj)
		temp_reg &= ~S5P_SDO_COMPONENT_BHS_ADJ_OFF;
	else
		temp_reg |= S5P_SDO_COMPONENT_BHS_ADJ_OFF;


	if (y_ppr_color_compensation)
		temp_reg &= ~S5P_SDO_COMPONENT_YPBPR_COMP_OFF;
	else
		temp_reg |= S5P_SDO_COMPONENT_YPBPR_COMP_OFF;


	if (rgcolor_compensation)
		temp_reg &= ~S5P_SDO_COMPONENT_RGB_COMP_OFF;
	else
		temp_reg |= S5P_SDO_COMPONENT_RGB_COMP_OFF;


	if (y_c_color_compensation)
		temp_reg &= ~S5P_SDO_COMPONENT_YC_COMP_OFF;
	else
		temp_reg |= S5P_SDO_COMPONENT_YC_COMP_OFF;


	if (y_cvbs_color_compensation)
		temp_reg &= ~S5P_SDO_COMPONENT_CVBS_COMP_OFF;
	else
		temp_reg |= S5P_SDO_COMPONENT_CVBS_COMP_OFF;


	writel(temp_reg, sdout_base + S5P_SDO_CCCON);
}

void s5p_sdout_init_brightness_hue_saturation(u32 gain_brightness,
					u32 offset_brightness,
					u32 gain0_cb_hue_saturation,
					u32 gain1_cb_hue_saturation,
					u32 gain0_cr_hue_saturation,
					u32 gain1_cr_hue_saturation,
					u32 offset_cb_hue_saturation,
					u32 offset_cr_hue_saturation)
{
	SDPRINTK(" %d, %d, %d, %d, %d, %d, %d, %d)\n\r", gain_brightness,
		offset_brightness, gain0_cb_hue_saturation,
		gain1_cb_hue_saturation, gain0_cr_hue_saturation,
		gain1_cr_hue_saturation, offset_cb_hue_saturation,
		offset_cr_hue_saturation);

	writel(S5P_SDO_BRIGHTNESS_GAIN(gain_brightness) |
		S5P_SDO_BRIGHTNESS_OFFSET(offset_brightness),
			sdout_base + S5P_SDO_YSCALE);

	writel(S5P_SDO_HS_CB_GAIN0(gain0_cb_hue_saturation) |
		S5P_SDO_HS_CB_GAIN1(gain1_cb_hue_saturation),
			sdout_base + S5P_SDO_CBSCALE);

	writel(S5P_SDO_HS_CR_GAIN0(gain0_cr_hue_saturation) |
		S5P_SDO_HS_CR_GAIN1(gain1_cr_hue_saturation),
			sdout_base + S5P_SDO_CRSCALE);

	writel(S5P_SDO_HS_CR_OFFSET(offset_cr_hue_saturation) |
		S5P_SDO_HS_CB_OFFSET(offset_cb_hue_saturation),
			sdout_base + S5P_SDO_CB_CR_OFFSET);
}

void s5p_sdout_init_rgb_color_compensation(u32 max_rgbcube, u32 min_rgbcube)
{
	SDPRINTK("0x%08x, 0x%08x\n\r", max_rgbcube, min_rgbcube);

	writel(S5P_SDO_MAX_RGB_CUBE(max_rgbcube) |
		S5P_SDO_MIN_RGB_CUBE(min_rgbcube),
			sdout_base + S5P_SDO_RGB_CC);
}

void s5p_sdout_init_cvbs_color_compensation(u32 y_lower_mid,
					u32 y_bottom,
					u32 y_top,
					u32 y_upper_mid,
					u32 radius)
{
	SDPRINTK("%d, %d, %d, %d, %d\n\r", y_lower_mid, y_bottom, y_top,
		y_upper_mid, radius);

	writel(S5P_SDO_Y_LOWER_MID_CVBS_CORN(y_lower_mid) |
		S5P_SDO_Y_BOTTOM_CVBS_CORN(y_bottom),
			sdout_base + S5P_SDO_CVBS_CC_Y1);
	writel(S5P_SDO_Y_TOP_CVBS_CORN(y_top) |
		S5P_SDO_Y_UPPER_MID_CVBS_CORN(y_upper_mid),
			sdout_base + S5P_SDO_CVBS_CC_Y2);
	writel(S5P_SDO_RADIUS_CVBS_CORN(radius),
			sdout_base + S5P_SDO_CVBS_CC_C);

}

void s5p_sdout_init_svideo_color_compensation(u32 y_top, u32 y_bottom,
						u32 y_c_cylinder)
{
	SDPRINTK(" %d, %d, %d)\n\r", y_top, y_bottom, y_c_cylinder);

	writel(S5P_SDO_Y_TOP_YC_CYLINDER(y_top) |
		S5P_SDO_Y_BOTOM_YC_CYLINDER(y_bottom),
			sdout_base + S5P_SDO_YC_CC_Y);
	writel(S5P_SDO_RADIUS_YC_CYLINDER(y_c_cylinder),
			sdout_base + S5P_SDO_YC_CC_C);

}

void s5p_sdout_init_component_porch(u32 back_525, u32 front_525,
					u32 back_625, u32 front_625)
{
	SDPRINTK(" %d, %d, %d, %d)\n\r",
			back_525, front_525, back_625, front_625);

	writel(S5P_SDO_COMPONENT_525_BP(back_525) |
		S5P_SDO_COMPONENT_525_FP(front_525),
			sdout_base + S5P_SDO_CSC_525_PORCH);
	writel(S5P_SDO_COMPONENT_625_BP(back_625) |
		S5P_SDO_COMPONENT_625_FP(front_625),
			sdout_base + S5P_SDO_CSC_625_PORCH);

}

int s5p_sdout_init_vesa_rgb_sync(enum s5p_sd_vesa_rgb_sync_type sync_type,
				enum s5p_tv_active_polarity v_sync_active,
				enum s5p_tv_active_polarity h_sync_active)
{
	u32 temp_reg = 0;

	SDPRINTK("%d, %d, %d\n\r", sync_type, v_sync_active, h_sync_active);

	switch (sync_type) {
	case SDOUT_VESA_RGB_SYNC_COMPOSITE:
		temp_reg |= S5P_SDO_RGB_SYNC_COMPOSITE;
		break;

	case SDOUT_VESA_RGB_SYNC_SEPARATE:
		temp_reg |= S5P_SDO_RGB_SYNC_SEPERATE;
		break;

	default:
		SDPRINTK(" invalid sync_type parameter(%d)\n\r", sync_type);
		return -1;
	}

	switch (v_sync_active) {
	case TVOUT_POL_ACTIVE_LOW:
		temp_reg |= S5P_SDO_RGB_VSYNC_LOW_ACT;
		break;

	case TVOUT_POL_ACTIVE_HIGH:
		temp_reg |= S5P_SDO_RGB_VSYNC_HIGH_ACT;
		break;

	default:
		SDPRINTK(" invalid v_sync_active parameter(%d)\n\r",
			v_sync_active);
		return -1;
	}

	switch (h_sync_active) {
	case TVOUT_POL_ACTIVE_LOW:
		temp_reg |= S5P_SDO_RGB_HSYNC_LOW_ACT;
		break;

	case TVOUT_POL_ACTIVE_HIGH:
		temp_reg |= S5P_SDO_RGB_HSYNC_HIGH_ACT;
		break;

	default:
		SDPRINTK(" invalid h_sync_active parameter(%d)\n\r",
			h_sync_active);
		return -1;
	}

	writel(temp_reg, sdout_base + S5P_SDO_RGBSYNC);

	return 0;
}

int s5p_sdout_init_ch_xtalk_cancel_coef(enum s5p_sd_channel_sel channel,
					u32 coeff2, u32 coeff1)
{
	SDPRINTK(" %d, %d, %d\n\r", channel, coeff2, coeff1);

	switch (channel) {
	case SDOUT_CHANNEL_0:
		writel(S5P_SDO_XTALK_COEF02(coeff2) |
			S5P_SDO_XTALK_COEF01(coeff1),
				sdout_base + S5P_SDO_XTALK0);
		break;

	case SDOUT_CHANNEL_1:
		writel(S5P_SDO_XTALK_COEF02(coeff2) |
			S5P_SDO_XTALK_COEF01(coeff1),
				sdout_base + S5P_SDO_XTALK1);
		break;

	case SDOUT_CHANNEL_2:
		writel(S5P_SDO_XTALK_COEF02(coeff2) |
			S5P_SDO_XTALK_COEF01(coeff1),
				sdout_base + S5P_SDO_XTALK2);
		break;

	default:
		SDPRINTK(" invalid channel parameter(%d)\n\r", channel);
		return -1;
	}

	return 0;
}

void s5p_sdout_init_closed_caption(u32 display_cc, u32 non_display_cc)
{
	SDPRINTK("%d, %d\n\r", display_cc, non_display_cc);

	writel(S5P_SDO_DISPLAY_CC_CAPTION(display_cc) |
		S5P_SDO_NON_DISPLAY_CC_CAPTION(non_display_cc),
		sdout_base + S5P_SDO_ARMCC);
}

static u32 s5p_sdout_init_wss_cgms_crc(u32 value)
{
	u8 i;
	u8 CGMS[14], CRC[6], OLD_CRC;
	u32 temp_in;

	temp_in = value;

	for (i = 0; i < 14; i++)
		CGMS[i] = (u8)(temp_in >> i) & 0x1 ;

	/* initialize state */
	for (i = 0; i < 6; i++)
		CRC[i] = 0x1;

	/* round 20 */
	for (i = 0; i < 14; i++) {
		OLD_CRC = CRC[0];
		CRC[0] = CRC[1];
		CRC[1] = CRC[2];
		CRC[2] = CRC[3];
		CRC[3] = CRC[4];
		CRC[4] = OLD_CRC ^ CGMS[i] ^ CRC[5];
		CRC[5] = OLD_CRC ^ CGMS[i];
	}

	/* recompose to return crc */
	temp_in &= 0x3fff;

	for (i = 0; i < 6; i++)
		temp_in |= ((u32)(CRC[i] & 0x1) << i);

	return temp_in;
}

int s5p_sdout_init_wss525_data(enum s5p_sd_525_copy_permit copy_permit,
				enum s5p_sd_525_mv_psp mv_psp,
				enum s5p_sd_525_copy_info copy_info,
				bool analog_on,
				enum s5p_sd_525_aspect_ratio display_ratio)
{
	u32 temp_reg = 0;

	SDPRINTK("%d, %d, %d, %d\n\r", copy_permit, mv_psp, copy_info,
		display_ratio);

	switch (copy_permit) {
	case SDO_525_COPY_PERMIT:
		temp_reg = S5P_SDO_WORD2_WSS525_COPY_PERMIT;
		break;

	case SDO_525_ONECOPY_PERMIT:
		temp_reg = S5P_SDO_WORD2_WSS525_ONECOPY_PERMIT;
		break;

	case SDO_525_NOCOPY_PERMIT:
		temp_reg = S5P_SDO_WORD2_WSS525_NOCOPY_PERMIT;
		break;

	default:
		SDPRINTK(" invalid copy_permit parameter(%d)\n\r", copy_permit);
		return -1;
	}

	switch (mv_psp) {
	case SDO_525_MV_PSP_OFF:
		temp_reg |= S5P_SDO_WORD2_WSS525_MV_PSP_OFF;
		break;

	case SDO_525_MV_PSP_ON_2LINE_BURST:
		temp_reg |= S5P_SDO_WORD2_WSS525_MV_PSP_ON_2LINE_BURST;
		break;

	case SDO_525_MV_PSP_ON_BURST_OFF:
		temp_reg |= S5P_SDO_WORD2_WSS525_MV_PSP_ON_BURST_OFF;
		break;

	case SDO_525_MV_PSP_ON_4LINE_BURST:
		temp_reg |= S5P_SDO_WORD2_WSS525_MV_PSP_ON_4LINE_BURST;
		break;

	default:
		SDPRINTK(" invalid mv_psp parameter(%d)\n\r", mv_psp);
		return -1;
	}

	switch (copy_info) {
	case SDO_525_COPY_INFO:
		temp_reg |= S5P_SDO_WORD1_WSS525_COPY_INFO;
		break;

	case SDO_525_DEFAULT:
		temp_reg |= S5P_SDO_WORD1_WSS525_DEFAULT;
		break;

	default:
		SDPRINTK(" invalid copy_info parameter(%d)\n\r", copy_info);
		return -1;
	}

	if (analog_on)
		temp_reg |= S5P_SDO_WORD2_WSS525_ANALOG_ON;
	else
		temp_reg |= S5P_SDO_WORD2_WSS525_ANALOG_OFF;

	switch (display_ratio) {
	case SDO_525_COPY_PERMIT:
		temp_reg |= S5P_SDO_WORD0_WSS525_4_3_NORMAL;
		break;

	case SDO_525_ONECOPY_PERMIT:
		temp_reg |= S5P_SDO_WORD0_WSS525_16_9_ANAMORPIC;
		break;

	case SDO_525_NOCOPY_PERMIT:
		temp_reg |= S5P_SDO_WORD0_WSS525_4_3_LETTERBOX;
		break;

	default:
		SDPRINTK(" invalid display_ratio parameter(%d)\n\r",
			display_ratio);
		return -1;
	}

	writel(temp_reg |
		S5P_SDO_CRC_WSS525(s5p_sdout_init_wss_cgms_crc(temp_reg)),
		sdout_base + S5P_SDO_WSS525);

	return 0;
}

int s5p_sdout_init_wss625_data(bool surround_sound,
				 bool copyright,
				 bool copy_protection,
				 bool text_subtitles,
				 enum s5p_sd_625_subtitles open_subtitles,
				 enum s5p_sd_625_camera_film camera_film,
				 enum s5p_sd_625_color_encoding color_encoding,
				 bool helper_signal,
				 enum s5p_sd_625_aspect_ratio display_ratio)
{
	u32 temp_reg = 0;

	SDPRINTK("%d, %d, %d, %d, %d, %d, %d, %d, %d\n\r",
		 surround_sound, copyright, copy_protection,
		 text_subtitles, open_subtitles, camera_film,
		 color_encoding, helper_signal, display_ratio);

	if (surround_sound)
		temp_reg = S5P_SDO_WSS625_SURROUND_SOUND_ENABLE;
	else
		temp_reg = S5P_SDO_WSS625_SURROUND_SOUND_DISABLE;

	if (copyright)
		temp_reg |= S5P_SDO_WSS625_COPYRIGHT;
	else
		temp_reg |= S5P_SDO_WSS625_NO_COPYRIGHT;

	if (copy_protection)
		temp_reg |= S5P_SDO_WSS625_COPY_RESTRICTED;
	else
		temp_reg |= S5P_SDO_WSS625_COPY_NOT_RESTRICTED;

	if (text_subtitles)
		temp_reg |= S5P_SDO_WSS625_TELETEXT_SUBTITLES;
	else
		temp_reg |= S5P_SDO_WSS625_TELETEXT_NO_SUBTITLES;

	switch (open_subtitles) {
	case SDO_625_NO_OPEN_SUBTITLES:
		temp_reg |= S5P_SDO_WSS625_NO_OPEN_SUBTITLES;
		break;

	case SDO_625_INACT_OPEN_SUBTITLES:
		temp_reg |= S5P_SDO_WSS625_INACT_OPEN_SUBTITLES;
		break;

	case SDO_625_OUTACT_OPEN_SUBTITLES:
		temp_reg |= S5P_SDO_WSS625_OUTACT_OPEN_SUBTITLES;
		break;

	default:
		SDPRINTK(" invalid open_subtitles parameter(%d)\n\r",
			open_subtitles);
		return -1;
	}

	switch (camera_film) {
	case SDO_625_CAMERA:
		temp_reg |= S5P_SDO_WSS625_CAMERA;
		break;

	case SDO_625_FILM:
		temp_reg |= S5P_SDO_WSS625_FILM;
		break;

	default:
		SDPRINTK("invalid camera_film parameter(%d)\n\r",
			camera_film);
		return -1;
	}

	switch (color_encoding) {
	case SDO_625_NORMAL_PAL:
		temp_reg |= S5P_SDO_WSS625_NORMAL_PAL;
		break;

	case SDO_625_MOTION_ADAPTIVE_COLORPLUS:
		temp_reg |= S5P_SDO_WSS625_MOTION_ADAPTIVE_COLORPLUS;
		break;

	default:
		SDPRINTK("invalid color_encoding parameter(%d)\n\r",
			color_encoding);
		return -1;
	}

	if (helper_signal)
		temp_reg |= S5P_SDO_WSS625_HELPER_SIG;
	else
		temp_reg |= S5P_SDO_WSS625_HELPER_NO_SIG;

	switch (display_ratio) {
	case SDO_625_4_3_FULL_576:
		temp_reg |= S5P_SDO_WSS625_4_3_FULL_576;
		break;

	case SDO_625_14_9_LETTERBOX_CENTER_504:
		temp_reg |= S5P_SDO_WSS625_14_9_LETTERBOX_CENTER_504;
		break;

	case SDO_625_14_9_LETTERBOX_TOP_504:
		temp_reg |= S5P_SDO_WSS625_14_9_LETTERBOX_TOP_504;
		break;

	case SDO_625_16_9_LETTERBOX_CENTER_430:
		temp_reg |= S5P_SDO_WSS625_16_9_LETTERBOX_CENTER_430;
		break;

	case SDO_625_16_9_LETTERBOX_TOP_430:
		temp_reg |= S5P_SDO_WSS625_16_9_LETTERBOX_TOP_430;
		break;

	case SDO_625_16_9_LETTERBOX_CENTER:
		temp_reg |= S5P_SDO_WSS625_16_9_LETTERBOX_CENTER;
		break;

	case SDO_625_14_9_FULL_CENTER_576:
		temp_reg |= S5P_SDO_WSS625_14_9_FULL_CENTER_576;
		break;

	case SDO_625_16_9_ANAMORPIC_576:
		temp_reg |= S5P_SDO_WSS625_16_9_ANAMORPIC_576;
		break;

	default:
		SDPRINTK("invalid display_ratio parameter(%d)\n\r",
			display_ratio);
		return -1;
	}

	writel(temp_reg, sdout_base + S5P_SDO_WSS625);

	return 0;
}

int s5p_sdout_init_cgmsa525_data(enum s5p_sd_525_copy_permit copy_permit,
				enum s5p_sd_525_mv_psp mv_psp,
				enum s5p_sd_525_copy_info copy_info,
				bool analog_on,
				enum s5p_sd_525_aspect_ratio display_ratio)
{
	u32 temp_reg = 0;

	SDPRINTK("%d, %d, %d, %d)\n\r", copy_permit, mv_psp, copy_info,
		display_ratio);

	switch (copy_permit) {
	case SDO_525_COPY_PERMIT:
		temp_reg = S5P_SDO_WORD2_CGMS525_COPY_PERMIT;
		break;

	case SDO_525_ONECOPY_PERMIT:
		temp_reg = S5P_SDO_WORD2_CGMS525_ONECOPY_PERMIT;
		break;

	case SDO_525_NOCOPY_PERMIT:
		temp_reg = S5P_SDO_WORD2_CGMS525_NOCOPY_PERMIT;
		break;

	default:
		SDPRINTK("invalid copy_permit parameter(%d)\n\r", copy_permit);
		return -1;
	}

	switch (mv_psp) {
	case SDO_525_MV_PSP_OFF:
		temp_reg |= S5P_SDO_WORD2_CGMS525_MV_PSP_OFF;
		break;

	case SDO_525_MV_PSP_ON_2LINE_BURST:
		temp_reg |= S5P_SDO_WORD2_CGMS525_MV_PSP_ON_2LINE_BURST;
		break;

	case SDO_525_MV_PSP_ON_BURST_OFF:
		temp_reg |= S5P_SDO_WORD2_CGMS525_MV_PSP_ON_BURST_OFF;
		break;

	case SDO_525_MV_PSP_ON_4LINE_BURST:
		temp_reg |= S5P_SDO_WORD2_CGMS525_MV_PSP_ON_4LINE_BURST;
		break;

	default:
		SDPRINTK(" invalid mv_psp parameter(%d)\n\r", mv_psp);
		return -1;
	}

	switch (copy_info) {
	case SDO_525_COPY_INFO:
		temp_reg |= S5P_SDO_WORD1_CGMS525_COPY_INFO;
		break;

	case SDO_525_DEFAULT:
		temp_reg |= S5P_SDO_WORD1_CGMS525_DEFAULT;
		break;

	default:
		SDPRINTK("invalid copy_info parameter(%d)\n\r", copy_info);
		return -1;
	}

	if (analog_on)
		temp_reg |= S5P_SDO_WORD2_CGMS525_ANALOG_ON;
	else
		temp_reg |= S5P_SDO_WORD2_CGMS525_ANALOG_OFF;

	switch (display_ratio) {
	case SDO_525_COPY_PERMIT:
		temp_reg |= S5P_SDO_WORD0_CGMS525_4_3_NORMAL;
		break;

	case SDO_525_ONECOPY_PERMIT:
		temp_reg |= S5P_SDO_WORD0_CGMS525_16_9_ANAMORPIC;
		break;

	case SDO_525_NOCOPY_PERMIT:
		temp_reg |= S5P_SDO_WORD0_CGMS525_4_3_LETTERBOX;
		break;

	default:
		SDPRINTK(" invalid display_ratio parameter(%d)\n\r",
			display_ratio);
		return -1;
	}

	writel(temp_reg | S5P_SDO_CRC_CGMS525(
		s5p_sdout_init_wss_cgms_crc(temp_reg)),
		sdout_base + S5P_SDO_CGMS525);

	return 0;
}

int s5p_sdout_init_cgmsa625_data(bool surround_sound,
				bool copyright,
				bool copy_protection,
				bool text_subtitles,
				enum s5p_sd_625_subtitles open_subtitles,
				enum s5p_sd_625_camera_film camera_film,
				enum s5p_sd_625_color_encoding color_encoding,
				bool helper_signal,
				enum s5p_sd_625_aspect_ratio display_ratio)
{
	u32 temp_reg = 0;

	SDPRINTK("%d, %d, %d, %d, %d, %d, %d, %d, %d)\n\r", surround_sound,
		copyright, copy_protection,
		text_subtitles, open_subtitles,
		camera_film, color_encoding, helper_signal,
		display_ratio);

	if (surround_sound)
		temp_reg = S5P_SDO_CGMS625_SURROUND_SOUND_ENABLE;
	else
		temp_reg = S5P_SDO_CGMS625_SURROUND_SOUND_DISABLE;

	if (copyright)
		temp_reg |= S5P_SDO_CGMS625_COPYRIGHT;
	else
		temp_reg |= S5P_SDO_CGMS625_NO_COPYRIGHT;

	if (copy_protection)
		temp_reg |= S5P_SDO_CGMS625_COPY_RESTRICTED;
	else
		temp_reg |= S5P_SDO_CGMS625_COPY_NOT_RESTRICTED;

	if (text_subtitles)
		temp_reg |= S5P_SDO_CGMS625_TELETEXT_SUBTITLES;
	else
		temp_reg |= S5P_SDO_CGMS625_TELETEXT_NO_SUBTITLES;

	switch (open_subtitles) {
	case SDO_625_NO_OPEN_SUBTITLES:
		temp_reg |= S5P_SDO_CGMS625_NO_OPEN_SUBTITLES;
		break;

	case SDO_625_INACT_OPEN_SUBTITLES:
		temp_reg |= S5P_SDO_CGMS625_INACT_OPEN_SUBTITLES;
		break;

	case SDO_625_OUTACT_OPEN_SUBTITLES:
		temp_reg |= S5P_SDO_CGMS625_OUTACT_OPEN_SUBTITLES;
		break;

	default:
		SDPRINTK("invalid open_subtitles parameter(%d)\n\r",
			open_subtitles);
		return -1;
	}

	switch (camera_film) {
	case SDO_625_CAMERA:
		temp_reg |= S5P_SDO_CGMS625_CAMERA;
		break;

	case SDO_625_FILM:
		temp_reg |= S5P_SDO_CGMS625_FILM;
		break;

	default:
		SDPRINTK(" invalid camera_film parameter(%d)\n\r",
			camera_film);
		return -1;
	}

	switch (color_encoding) {
	case SDO_625_NORMAL_PAL:
		temp_reg |= S5P_SDO_CGMS625_NORMAL_PAL;
		break;

	case SDO_625_MOTION_ADAPTIVE_COLORPLUS:
		temp_reg |= S5P_SDO_CGMS625_MOTION_ADAPTIVE_COLORPLUS;
		break;

	default:
		SDPRINTK(" invalid color_encoding parameter(%d)\n\r",
			color_encoding);
		return -1;
	}

	if (helper_signal)
		temp_reg |= S5P_SDO_CGMS625_HELPER_SIG;
	else
		temp_reg |= S5P_SDO_CGMS625_HELPER_NO_SIG;

	switch (display_ratio) {
	case SDO_625_4_3_FULL_576:
		temp_reg |= S5P_SDO_CGMS625_4_3_FULL_576;
		break;

	case SDO_625_14_9_LETTERBOX_CENTER_504:
		temp_reg |= S5P_SDO_CGMS625_14_9_LETTERBOX_CENTER_504;
		break;

	case SDO_625_14_9_LETTERBOX_TOP_504:
		temp_reg |= S5P_SDO_CGMS625_14_9_LETTERBOX_TOP_504;
		break;

	case SDO_625_16_9_LETTERBOX_CENTER_430:
		temp_reg |= S5P_SDO_CGMS625_16_9_LETTERBOX_CENTER_430;
		break;

	case SDO_625_16_9_LETTERBOX_TOP_430:
		temp_reg |= S5P_SDO_CGMS625_16_9_LETTERBOX_TOP_430;
		break;

	case SDO_625_16_9_LETTERBOX_CENTER:
		temp_reg |= S5P_SDO_CGMS625_16_9_LETTERBOX_CENTER;
		break;

	case SDO_625_14_9_FULL_CENTER_576:
		temp_reg |= S5P_SDO_CGMS625_14_9_FULL_CENTER_576;
		break;

	case SDO_625_16_9_ANAMORPIC_576:
		temp_reg |= S5P_SDO_CGMS625_16_9_ANAMORPIC_576;
		break;

	default:
		SDPRINTK("invalid display_ratio parameter(%d)\n\r",
			display_ratio);
		return -1;
	}

	writel(temp_reg, sdout_base + S5P_SDO_CGMS625);

	return 0;
}

static int s5p_sdout_init_antialias_filter_coeff_default(
					enum s5p_sd_level composite_level,
					enum s5p_sd_vsync_ratio composite_ratio,
					enum s5p_tv_o_mode out_mode)
{
	SDPRINTK("%d, %d, %d\n\r", composite_level, composite_ratio, out_mode);

	switch (composite_level) {
	case S5P_TV_SD_LEVEL_0IRE:
		switch (composite_ratio) {
		case SDOUT_VTOS_RATIO_10_4:
			switch (out_mode) {
			case TVOUT_OUTPUT_COMPOSITE:
			case TVOUT_OUTPUT_SVIDEO:
				writel(0x00000000 , sdout_base + S5P_SDO_Y3);
				writel(0x00000000 , sdout_base + S5P_SDO_Y4);
				writel(0x00000000 , sdout_base + S5P_SDO_Y5);
				writel(0x00000000 , sdout_base + S5P_SDO_Y6);
				writel(0x00000000 , sdout_base + S5P_SDO_Y7);
				writel(0x00000000 , sdout_base + S5P_SDO_Y8);
				writel(0x00000000 , sdout_base + S5P_SDO_Y9);
				writel(0x00000000 , sdout_base + S5P_SDO_Y10);
				writel(0x0000029a , sdout_base + S5P_SDO_Y11);
				writel(0x00000000 , sdout_base + S5P_SDO_CB0);
				writel(0x00000000 , sdout_base + S5P_SDO_CB1);
				writel(0x00000000 , sdout_base + S5P_SDO_CB2);
				writel(0x00000000 , sdout_base + S5P_SDO_CB3);
				writel(0x00000000 , sdout_base + S5P_SDO_CB4);
				writel(0x00000001 , sdout_base + S5P_SDO_CB5);
				writel(0x00000007 , sdout_base + S5P_SDO_CB6);
				writel(0x00000015 , sdout_base + S5P_SDO_CB7);
				writel(0x0000002b , sdout_base + S5P_SDO_CB8);
				writel(0x00000045 , sdout_base + S5P_SDO_CB9);
				writel(0x00000059 , sdout_base + S5P_SDO_CB10);
				writel(0x00000061 , sdout_base + S5P_SDO_CB11);
				writel(0x00000000 , sdout_base + S5P_SDO_CR1);
				writel(0x00000000 , sdout_base + S5P_SDO_CR2);
				writel(0x00000000 , sdout_base + S5P_SDO_CR3);
				writel(0x00000000 , sdout_base + S5P_SDO_CR4);
				writel(0x00000002 , sdout_base + S5P_SDO_CR5);
				writel(0x0000000a , sdout_base + S5P_SDO_CR6);
				writel(0x0000001e , sdout_base + S5P_SDO_CR7);
				writel(0x0000003d , sdout_base + S5P_SDO_CR8);
				writel(0x00000061 , sdout_base + S5P_SDO_CR9);
				writel(0x0000007a , sdout_base + S5P_SDO_CR10);
				writel(0x0000008f , sdout_base + S5P_SDO_CR11);
				break;

			case TVOUT_OUTPUT_COMPONENT_YPBPR_INERLACED:
			case TVOUT_OUTPUT_COMPONENT_YPBPR_PROGRESSIVE:
			case TVOUT_OUTPUT_COMPONENT_RGB_PROGRESSIVE:
				writel(0x00000000, sdout_base + S5P_SDO_Y0);
				writel(0x00000000, sdout_base + S5P_SDO_Y1);
				writel(0x00000000, sdout_base + S5P_SDO_Y2);
				writel(0x00000000, sdout_base + S5P_SDO_Y3);
				writel(0x00000000, sdout_base + S5P_SDO_Y4);
				writel(0x00000000, sdout_base + S5P_SDO_Y5);
				writel(0x00000000, sdout_base + S5P_SDO_Y6);
				writel(0x00000000, sdout_base + S5P_SDO_Y7);
				writel(0x00000000, sdout_base + S5P_SDO_Y8);
				writel(0x00000000, sdout_base + S5P_SDO_Y9);
				writel(0x00000000, sdout_base + S5P_SDO_Y10);
				writel(0x0000029a, sdout_base + S5P_SDO_Y11);
				writel(0x00000000, sdout_base + S5P_SDO_CB0);
				writel(0x00000000, sdout_base + S5P_SDO_CB1);
				writel(0x00000000, sdout_base + S5P_SDO_CB2);
				writel(0x00000000, sdout_base + S5P_SDO_CB3);
				writel(0x00000000, sdout_base + S5P_SDO_CB4);
				writel(0x00000001, sdout_base + S5P_SDO_CB5);
				writel(0x00000007, sdout_base + S5P_SDO_CB6);
				writel(0x00000015, sdout_base + S5P_SDO_CB7);
				writel(0x0000002b, sdout_base + S5P_SDO_CB8);
				writel(0x00000045, sdout_base + S5P_SDO_CB9);
				writel(0x00000059, sdout_base + S5P_SDO_CB10);
				writel(0x00000061, sdout_base + S5P_SDO_CB11);
				writel(0x00000000, sdout_base + S5P_SDO_CR1);
				writel(0x00000000, sdout_base + S5P_SDO_CR2);
				writel(0x00000000, sdout_base + S5P_SDO_CR3);
				writel(0x00000000, sdout_base + S5P_SDO_CR4);
				writel(0x00000002, sdout_base + S5P_SDO_CR5);
				writel(0x0000000a, sdout_base + S5P_SDO_CR6);
				writel(0x0000001e, sdout_base + S5P_SDO_CR7);
				writel(0x0000003d, sdout_base + S5P_SDO_CR8);
				writel(0x00000061, sdout_base + S5P_SDO_CR9);
				writel(0x0000007a, sdout_base + S5P_SDO_CR10);
				writel(0x0000008f, sdout_base + S5P_SDO_CR11);
				break;

			default:
				SDPRINTK("invalid out_mode parameter(%d)\n\r",
					out_mode);
				return -1;
			}

			break;

		case SDOUT_VTOS_RATIO_7_3:
			writel(0x00000000, sdout_base + S5P_SDO_Y0);
			writel(0x00000000, sdout_base + S5P_SDO_Y1);
			writel(0x00000000, sdout_base + S5P_SDO_Y2);
			writel(0x00000000, sdout_base + S5P_SDO_Y3);
			writel(0x00000000, sdout_base + S5P_SDO_Y4);
			writel(0x00000000, sdout_base + S5P_SDO_Y5);
			writel(0x00000000, sdout_base + S5P_SDO_Y6);
			writel(0x00000000, sdout_base + S5P_SDO_Y7);
			writel(0x00000000, sdout_base + S5P_SDO_Y8);
			writel(0x00000000, sdout_base + S5P_SDO_Y9);
			writel(0x00000000, sdout_base + S5P_SDO_Y10);
			writel(0x00000281, sdout_base + S5P_SDO_Y11);
			writel(0x00000000, sdout_base + S5P_SDO_CB0);
			writel(0x00000000, sdout_base + S5P_SDO_CB1);
			writel(0x00000000, sdout_base + S5P_SDO_CB2);
			writel(0x00000000, sdout_base + S5P_SDO_CB3);
			writel(0x00000000, sdout_base + S5P_SDO_CB4);
			writel(0x00000001, sdout_base + S5P_SDO_CB5);
			writel(0x00000007, sdout_base + S5P_SDO_CB6);
			writel(0x00000015, sdout_base + S5P_SDO_CB7);
			writel(0x0000002a, sdout_base + S5P_SDO_CB8);
			writel(0x00000044, sdout_base + S5P_SDO_CB9);
			writel(0x00000057, sdout_base + S5P_SDO_CB10);
			writel(0x0000005f, sdout_base + S5P_SDO_CB11);
			writel(0x00000000, sdout_base + S5P_SDO_CR1);
			writel(0x00000000, sdout_base + S5P_SDO_CR2);
			writel(0x00000000, sdout_base + S5P_SDO_CR3);
			writel(0x00000000, sdout_base + S5P_SDO_CR4);
			writel(0x00000002, sdout_base + S5P_SDO_CR5);
			writel(0x0000000a, sdout_base + S5P_SDO_CR6);
			writel(0x0000001d, sdout_base + S5P_SDO_CR7);
			writel(0x0000003c, sdout_base + S5P_SDO_CR8);
			writel(0x0000005f, sdout_base + S5P_SDO_CR9);
			writel(0x0000007b, sdout_base + S5P_SDO_CR10);
			writel(0x00000086, sdout_base + S5P_SDO_CR11);
			break;

		default:
			SDPRINTK("invalid composite_ratio parameter(%d)\n\r",
				composite_ratio);
			return -1;
		}

		break;

	case S5P_TV_SD_LEVEL_75IRE:
		switch (composite_ratio) {
		case SDOUT_VTOS_RATIO_10_4:
			writel(0x00000000, sdout_base + S5P_SDO_Y0);
			writel(0x00000000, sdout_base + S5P_SDO_Y1);
			writel(0x00000000, sdout_base + S5P_SDO_Y2);
			writel(0x00000000, sdout_base + S5P_SDO_Y3);
			writel(0x00000000, sdout_base + S5P_SDO_Y4);
			writel(0x00000000, sdout_base + S5P_SDO_Y5);
			writel(0x00000000, sdout_base + S5P_SDO_Y6);
			writel(0x00000000, sdout_base + S5P_SDO_Y7);
			writel(0x00000000, sdout_base + S5P_SDO_Y8);
			writel(0x00000000, sdout_base + S5P_SDO_Y9);
			writel(0x00000000, sdout_base + S5P_SDO_Y10);
			writel(0x0000025d, sdout_base + S5P_SDO_Y11);
			writel(0x00000000, sdout_base + S5P_SDO_CB0);
			writel(0x00000000, sdout_base + S5P_SDO_CB1);
			writel(0x00000000, sdout_base + S5P_SDO_CB2);
			writel(0x00000000, sdout_base + S5P_SDO_CB3);
			writel(0x00000000, sdout_base + S5P_SDO_CB4);
			writel(0x00000001, sdout_base + S5P_SDO_CB5);
			writel(0x00000007, sdout_base + S5P_SDO_CB6);
			writel(0x00000014, sdout_base + S5P_SDO_CB7);
			writel(0x00000028, sdout_base + S5P_SDO_CB8);
			writel(0x0000003f, sdout_base + S5P_SDO_CB9);
			writel(0x00000052, sdout_base + S5P_SDO_CB10);
			writel(0x0000005a, sdout_base + S5P_SDO_CB11);
			writel(0x00000000, sdout_base + S5P_SDO_CR1);
			writel(0x00000000, sdout_base + S5P_SDO_CR2);
			writel(0x00000000, sdout_base + S5P_SDO_CR3);
			writel(0x00000000, sdout_base + S5P_SDO_CR4);
			writel(0x00000001, sdout_base + S5P_SDO_CR5);
			writel(0x00000009, sdout_base + S5P_SDO_CR6);
			writel(0x0000001c, sdout_base + S5P_SDO_CR7);
			writel(0x00000039, sdout_base + S5P_SDO_CR8);
			writel(0x0000005a, sdout_base + S5P_SDO_CR9);
			writel(0x00000074, sdout_base + S5P_SDO_CR10);
			writel(0x0000007e, sdout_base + S5P_SDO_CR11);
			break;

		case SDOUT_VTOS_RATIO_7_3:
			writel(0x00000000, sdout_base + S5P_SDO_Y0);
			writel(0x00000000, sdout_base + S5P_SDO_Y1);
			writel(0x00000000, sdout_base + S5P_SDO_Y2);
			writel(0x00000000, sdout_base + S5P_SDO_Y3);
			writel(0x00000000, sdout_base + S5P_SDO_Y4);
			writel(0x00000000, sdout_base + S5P_SDO_Y5);
			writel(0x00000000, sdout_base + S5P_SDO_Y6);
			writel(0x00000000, sdout_base + S5P_SDO_Y7);
			writel(0x00000000, sdout_base + S5P_SDO_Y8);
			writel(0x00000000, sdout_base + S5P_SDO_Y9);
			writel(0x00000000, sdout_base + S5P_SDO_Y10);
			writel(0x00000251, sdout_base + S5P_SDO_Y11);
			writel(0x00000000, sdout_base + S5P_SDO_CB0);
			writel(0x00000000, sdout_base + S5P_SDO_CB1);
			writel(0x00000000, sdout_base + S5P_SDO_CB2);
			writel(0x00000000, sdout_base + S5P_SDO_CB3);
			writel(0x00000000, sdout_base + S5P_SDO_CB4);
			writel(0x00000001, sdout_base + S5P_SDO_CB5);
			writel(0x00000006, sdout_base + S5P_SDO_CB6);
			writel(0x00000013, sdout_base + S5P_SDO_CB7);
			writel(0x00000028, sdout_base + S5P_SDO_CB8);
			writel(0x0000003f, sdout_base + S5P_SDO_CB9);
			writel(0x00000051, sdout_base + S5P_SDO_CB10);
			writel(0x00000056, sdout_base + S5P_SDO_CB11);
			writel(0x00000000, sdout_base + S5P_SDO_CR1);
			writel(0x00000000, sdout_base + S5P_SDO_CR2);
			writel(0x00000000, sdout_base + S5P_SDO_CR3);
			writel(0x00000000, sdout_base + S5P_SDO_CR4);
			writel(0x00000002, sdout_base + S5P_SDO_CR5);
			writel(0x00000005, sdout_base + S5P_SDO_CR6);
			writel(0x00000018, sdout_base + S5P_SDO_CR7);
			writel(0x00000037, sdout_base + S5P_SDO_CR8);
			writel(0x0000005A, sdout_base + S5P_SDO_CR9);
			writel(0x00000076, sdout_base + S5P_SDO_CR10);
			writel(0x0000007e, sdout_base + S5P_SDO_CR11);
			break;

		default:
			SDPRINTK("  invalid composite_ratio parameter(%d)\n\r",
				composite_ratio);
			return -1;
		}

		break;

	default:
		SDPRINTK("  invalid composite_level parameter(%d)\n\r",
			composite_level);
		return -1;
	}

	return 0;
}

static int s5p_sdout_init_oversampling_filter_coeff_default(
						enum s5p_tv_o_mode out_mode)
{

	SDPRINTK("%d\n\r", out_mode);

	switch (out_mode) {
	case TVOUT_OUTPUT_COMPOSITE:
	case TVOUT_OUTPUT_SVIDEO:
	case TVOUT_OUTPUT_COMPONENT_YPBPR_INERLACED:
		break;

	default:
		SDPRINTK("invalid out_mode parameter(%d)\n\r", out_mode);
		return -1;
	}

	return 0;
}

int s5p_sdout_init_display_mode(enum s5p_tv_disp_mode disp_mode,
				enum s5p_tv_o_mode out_mode,
				enum s5p_sd_order order)
{
	u32 temp_reg = 0;

	SDPRINTK(" %d, %d, %d\n\r", disp_mode, out_mode, order);

	switch (disp_mode) {
	case TVOUT_NTSC_M:
		temp_reg |= S5P_SDO_NTSC_M;
		s5p_sdout_init_video_scale_cfg(S5P_TV_SD_LEVEL_0IRE,
				SDOUT_VTOS_RATIO_7_3, S5P_TV_SD_LEVEL_75IRE,
				SDOUT_VTOS_RATIO_10_4);

		s5p_sdout_init_antialias_filter_coeff_default(
			S5P_TV_SD_LEVEL_75IRE,
			SDOUT_VTOS_RATIO_10_4,
			out_mode);
		break;

	case TVOUT_PAL_BDGHI:
		temp_reg |= S5P_SDO_PAL_BGHID;
		s5p_sdout_init_video_scale_cfg(S5P_TV_SD_LEVEL_0IRE,
			SDOUT_VTOS_RATIO_7_3, S5P_TV_SD_LEVEL_0IRE,
			SDOUT_VTOS_RATIO_7_3);

		s5p_sdout_init_antialias_filter_coeff_default(
			S5P_TV_SD_LEVEL_0IRE,
			SDOUT_VTOS_RATIO_7_3, out_mode);
		break;

	case TVOUT_PAL_M:
		temp_reg |= S5P_SDO_PAL_M;
		s5p_sdout_init_video_scale_cfg(S5P_TV_SD_LEVEL_0IRE,
				SDOUT_VTOS_RATIO_7_3, S5P_TV_SD_LEVEL_0IRE,
				SDOUT_VTOS_RATIO_7_3);

		s5p_sdout_init_antialias_filter_coeff_default(
			S5P_TV_SD_LEVEL_0IRE,
			SDOUT_VTOS_RATIO_7_3,
			out_mode);
		break;

	case TVOUT_PAL_N:
		temp_reg |= S5P_SDO_PAL_N;
		s5p_sdout_init_video_scale_cfg(S5P_TV_SD_LEVEL_0IRE,
				SDOUT_VTOS_RATIO_7_3, S5P_TV_SD_LEVEL_0IRE,
				SDOUT_VTOS_RATIO_7_3);

		s5p_sdout_init_antialias_filter_coeff_default(
			S5P_TV_SD_LEVEL_75IRE,
			SDOUT_VTOS_RATIO_10_4,
			out_mode);
		break;

	case TVOUT_PAL_NC:
		temp_reg |= S5P_SDO_PAL_NC;
		s5p_sdout_init_video_scale_cfg(S5P_TV_SD_LEVEL_0IRE,
				SDOUT_VTOS_RATIO_7_3, S5P_TV_SD_LEVEL_0IRE,
				SDOUT_VTOS_RATIO_7_3);

		s5p_sdout_init_antialias_filter_coeff_default(
			S5P_TV_SD_LEVEL_0IRE,
			SDOUT_VTOS_RATIO_7_3,
			out_mode);
		break;

	case TVOUT_PAL_60:
		temp_reg |= S5P_SDO_PAL_60;
		s5p_sdout_init_video_scale_cfg(S5P_TV_SD_LEVEL_0IRE,
				SDOUT_VTOS_RATIO_7_3, S5P_TV_SD_LEVEL_0IRE,
				SDOUT_VTOS_RATIO_7_3);
		s5p_sdout_init_antialias_filter_coeff_default(
			S5P_TV_SD_LEVEL_0IRE,
			SDOUT_VTOS_RATIO_7_3,
			out_mode);
		break;

	case TVOUT_NTSC_443:
		temp_reg |= S5P_SDO_NTSC_443;
		s5p_sdout_init_video_scale_cfg(S5P_TV_SD_LEVEL_0IRE,
				SDOUT_VTOS_RATIO_7_3, S5P_TV_SD_LEVEL_75IRE,
				SDOUT_VTOS_RATIO_10_4);
		s5p_sdout_init_antialias_filter_coeff_default(
			S5P_TV_SD_LEVEL_75IRE,
			SDOUT_VTOS_RATIO_10_4,
			out_mode);
		break;

	default:
		SDPRINTK("invalid disp_mode parameter(%d)\n\r", disp_mode);
		return -1;
	}

	switch (out_mode) {
	case TVOUT_OUTPUT_COMPOSITE:
	case TVOUT_OUTPUT_SVIDEO:
		temp_reg |= S5P_SDO_COMPOSITE | S5P_SDO_INTERLACED;

		switch (order) {

		case S5P_TV_SD_O_ORDER_COMPOSITE_CVBS_Y_C:
			temp_reg |= S5P_SDO_DAC2_CVBS | S5P_SDO_DAC1_Y |
					S5P_SDO_DAC0_C;
			break;

		case S5P_TV_SD_O_ORDER_COMPOSITE_CVBS_C_Y:
			temp_reg |= S5P_SDO_DAC2_CVBS | S5P_SDO_DAC1_C |
					S5P_SDO_DAC0_Y;
			break;

		case S5P_TV_SD_O_ORDER_COMPOSITE_Y_C_CVBS:
			temp_reg |= S5P_SDO_DAC2_Y | S5P_SDO_DAC1_C |
					S5P_SDO_DAC0_CVBS;
			break;

		case S5P_TV_SD_O_ORDER_COMPOSITE_Y_CVBS_C:
			temp_reg |= S5P_SDO_DAC2_Y | S5P_SDO_DAC1_CVBS |
					S5P_SDO_DAC0_C;
			break;

		case S5P_TV_SD_O_ORDER_COMPOSITE_C_CVBS_Y:
			temp_reg |= S5P_SDO_DAC2_C | S5P_SDO_DAC1_CVBS |
					S5P_SDO_DAC0_Y;
			break;

		case S5P_TV_SD_O_ORDER_COMPOSITE_C_Y_CVBS:
			temp_reg |= S5P_SDO_DAC2_C | S5P_SDO_DAC1_Y |
					S5P_SDO_DAC0_CVBS;
			break;

		default:
			SDPRINTK(" invalid order parameter(%d)\n\r", order);
			return -1;
		}

		break;

	case TVOUT_OUTPUT_COMPONENT_YPBPR_INERLACED:
		temp_reg |= S5P_SDO_COMPONENT | S5P_SDO_YPBPR |
					S5P_SDO_INTERLACED;

		switch (order) {

		case S5P_TV_SD_O_ORDER_COMPONENT_RGB_PRYPB:
			temp_reg |= S5P_SDO_DAC2_PR_R | S5P_SDO_DAC1_Y_G |
					S5P_SDO_DAC0_PB_B;
			break;

		case S5P_TV_SD_O_ORDER_COMPONENT_RBG_PRPBY:
			temp_reg |= S5P_SDO_DAC2_PR_R | S5P_SDO_DAC1_PB_B |
					S5P_SDO_DAC0_Y_G;
			break;

		case S5P_TV_SD_O_ORDER_COMPONENT_BGR_PBYPR:
			temp_reg |= S5P_SDO_DAC2_PB_B | S5P_SDO_DAC1_Y_G |
					S5P_SDO_DAC0_PR_R;
			break;

		case S5P_TV_SD_O_ORDER_COMPONENT_BRG_PBPRY:
			temp_reg |= S5P_SDO_DAC2_PB_B | S5P_SDO_DAC1_PR_R |
					S5P_SDO_DAC0_Y_G;
			break;

		case S5P_TV_SD_O_ORDER_COMPONENT_GRB_YPRPB:
			temp_reg |= S5P_SDO_DAC2_Y_G | S5P_SDO_DAC1_PR_R |
					S5P_SDO_DAC0_PB_B;
			break;

		case S5P_TV_SD_O_ORDER_COMPONENT_GBR_YPBPR:
			temp_reg |= S5P_SDO_DAC2_Y_G | S5P_SDO_DAC1_PB_B |
					S5P_SDO_DAC0_PR_R;
			break;

		default:
			SDPRINTK(" invalid order parameter(%d)\n\r", order);
			return -1;
		}

		break;

	case TVOUT_OUTPUT_COMPONENT_YPBPR_PROGRESSIVE:
		temp_reg |= S5P_SDO_COMPONENT | S5P_SDO_YPBPR |
					S5P_SDO_PROGRESSIVE;

		switch (order) {

		case S5P_TV_SD_O_ORDER_COMPONENT_RGB_PRYPB:
			temp_reg |= S5P_SDO_DAC2_PR_R | S5P_SDO_DAC1_Y_G |
					S5P_SDO_DAC0_PB_B;
			break;

		case S5P_TV_SD_O_ORDER_COMPONENT_RBG_PRPBY:
			temp_reg |= S5P_SDO_DAC2_PR_R | S5P_SDO_DAC1_PB_B |
					S5P_SDO_DAC0_Y_G;
			break;

		case S5P_TV_SD_O_ORDER_COMPONENT_BGR_PBYPR:
			temp_reg |= S5P_SDO_DAC2_PB_B | S5P_SDO_DAC1_Y_G |
					S5P_SDO_DAC0_PR_R;
			break;

		case S5P_TV_SD_O_ORDER_COMPONENT_BRG_PBPRY:
			temp_reg |= S5P_SDO_DAC2_PB_B | S5P_SDO_DAC1_PR_R |
					S5P_SDO_DAC0_Y_G;
			break;

		case S5P_TV_SD_O_ORDER_COMPONENT_GRB_YPRPB:
			temp_reg |= S5P_SDO_DAC2_Y_G | S5P_SDO_DAC1_PR_R |
					S5P_SDO_DAC0_PB_B;
			break;

		case S5P_TV_SD_O_ORDER_COMPONENT_GBR_YPBPR:
			temp_reg |= S5P_SDO_DAC2_Y_G | S5P_SDO_DAC1_PB_B |
					S5P_SDO_DAC0_PR_R;
			break;

		default:
			SDPRINTK(" invalid order parameter(%d)\n\r", order);
			return -1;
		}

		break;

	case TVOUT_OUTPUT_COMPONENT_RGB_PROGRESSIVE:
		temp_reg |= S5P_SDO_COMPONENT | S5P_SDO_RGB |
				S5P_SDO_PROGRESSIVE;

		switch (order) {

		case S5P_TV_SD_O_ORDER_COMPONENT_RGB_PRYPB:
			temp_reg |= S5P_SDO_DAC2_PR_R | S5P_SDO_DAC1_Y_G |
					S5P_SDO_DAC0_PB_B;
			break;

		case S5P_TV_SD_O_ORDER_COMPONENT_RBG_PRPBY:
			temp_reg |= S5P_SDO_DAC2_PR_R | S5P_SDO_DAC1_PB_B |
					S5P_SDO_DAC0_Y_G;
			break;

		case S5P_TV_SD_O_ORDER_COMPONENT_BGR_PBYPR:
			temp_reg |= S5P_SDO_DAC2_PB_B | S5P_SDO_DAC1_Y_G |
					S5P_SDO_DAC0_PR_R;
			break;

		case S5P_TV_SD_O_ORDER_COMPONENT_BRG_PBPRY:
			temp_reg |= S5P_SDO_DAC2_PB_B | S5P_SDO_DAC1_PR_R |
					S5P_SDO_DAC0_Y_G;
			break;

		case S5P_TV_SD_O_ORDER_COMPONENT_GRB_YPRPB:
			temp_reg |= S5P_SDO_DAC2_Y_G | S5P_SDO_DAC1_PR_R |
					S5P_SDO_DAC0_PB_B;
			break;

		case S5P_TV_SD_O_ORDER_COMPONENT_GBR_YPBPR:
			temp_reg |= S5P_SDO_DAC2_Y_G | S5P_SDO_DAC1_PB_B |
					S5P_SDO_DAC0_PR_R;
			break;

		default:
			SDPRINTK("invalid order parameter(%d)\n\r", order);
			return -1;
		}

		break;

	default:
		SDPRINTK(" invalid out_mode parameter(%d)\n\r", out_mode);
		return -1;
	}

	s5p_sdout_init_oversampling_filter_coeff_default(out_mode);

	writel(temp_reg, sdout_base + S5P_SDO_CONFIG);

	return 0;
}

void s5p_sdout_start(void)
{
	writel(S5P_SDO_TVOUT_CLOCK_ON, sdout_base + S5P_SDO_CLKCON);
}

void s5p_sdout_stop(void)
{
	mdelay(100);

	writel(S5P_SDO_TVOUT_CLOCK_OFF, sdout_base + S5P_SDO_CLKCON);
}

void s5p_sdout_sw_reset(bool active)
{
	SDPRINTK("%d\n\r", active);

	if (active)
		writel(readl(sdout_base + S5P_SDO_CLKCON) |
			S5P_SDO_TVOUT_SW_RESET,
				sdout_base + S5P_SDO_CLKCON);
	else
		writel(readl(sdout_base + S5P_SDO_CLKCON) &
			~S5P_SDO_TVOUT_SW_RESET,
				sdout_base + S5P_SDO_CLKCON);
}

void s5p_sdout_set_interrupt_enable(bool vsync_intr_en)
{
	SDPRINTK("%d)\n\r", vsync_intr_en);

	if (vsync_intr_en)
		writel(readl(sdout_base + S5P_SDO_IRQMASK) &
			~S5P_SDO_VSYNC_IRQ_DISABLE,
				sdout_base + S5P_SDO_IRQMASK);
	else
		writel(readl(sdout_base + S5P_SDO_IRQMASK) |
			S5P_SDO_VSYNC_IRQ_DISABLE,
				sdout_base + S5P_SDO_IRQMASK);
}

void s5p_sdout_clear_interrupt_pending(void)
{
	writel(readl(sdout_base + S5P_SDO_IRQ) | S5P_SDO_VSYNC_IRQ_PEND,
			sdout_base + S5P_SDO_IRQ);
}

int __init s5p_sdout_probe(struct platform_device *pdev, u32 res_num)
{
	struct resource *res;
	size_t	size;

	res = platform_get_resource(pdev, IORESOURCE_MEM, res_num);

	if (res == NULL) {
		dev_err(&pdev->dev, "failed to get memory region resource\n");
		goto error;
	}

	size = (res->end - res->start) + 1;

	sdout_mem = request_mem_region(res->start, size, pdev->name);

	if (sdout_mem == NULL) {
		dev_err(&pdev->dev, "failed to get memory region\n");
		goto error;
	}

	sdout_base = ioremap(res->start, size);

	if (sdout_base == NULL) {
		dev_err(&pdev->dev, "failed to ioremap address region\n");
		goto error;
	}

	return 0;
error:
	return -ENOENT;
}

int __init s5p_sdout_release(struct platform_device *pdev)
{
	iounmap(sdout_base);

	/* remove memory region */
	if (sdout_mem != NULL) {
		if (release_resource(sdout_mem))
			dev_err(&pdev->dev, "Can't remove tvout drv !!\n");

		kfree(sdout_mem);

		sdout_mem = NULL;
	}

	return 0;
}