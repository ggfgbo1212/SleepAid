/*
* Copyright 2026 NXP
* NXP Confidential and Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"
#include "lvgl_action.h"   /* 控件事件回调（setup 末尾绑定本页控件） */


void setup_scr_SettingsPage4Info(lv_ui *ui)
{
	//Write codes SettingsPage4Info
	ui->SettingsPage4Info = lv_obj_create(NULL);
	lv_obj_set_size(ui->SettingsPage4Info, 240, 320);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4Info, LV_SCROLLBAR_MODE_OFF);

	//Write style for SettingsPage4Info, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->SettingsPage4Info, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->SettingsPage4Info, lv_color_hex(0xebeef5), LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SettingsPage4Info_settings_p4_info_image
	ui->SettingsPage4Info_settings_p4_info_image = lv_img_create(ui->SettingsPage4Info);
	lv_obj_add_flag(ui->SettingsPage4Info_settings_p4_info_image, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->SettingsPage4Info_settings_p4_info_image, &_trademark_alpha_34x34);
	lv_img_set_pivot(ui->SettingsPage4Info_settings_p4_info_image, 50,50);
	lv_img_set_angle(ui->SettingsPage4Info_settings_p4_info_image, 0);
	lv_obj_set_pos(ui->SettingsPage4Info_settings_p4_info_image, 201, 2);
	lv_obj_set_size(ui->SettingsPage4Info_settings_p4_info_image, 34, 34);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4Info_settings_p4_info_image, LV_SCROLLBAR_MODE_OFF);

	//Write style for SettingsPage4Info_settings_p4_info_image, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_opa(ui->SettingsPage4Info_settings_p4_info_image, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SettingsPage4Info_settings_p4_info_title
	ui->SettingsPage4Info_settings_p4_info_title = lv_label_create(ui->SettingsPage4Info);
	lv_label_set_text(ui->SettingsPage4Info_settings_p4_info_title, "本机信息");
	lv_label_set_long_mode(ui->SettingsPage4Info_settings_p4_info_title, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(ui->SettingsPage4Info_settings_p4_info_title, 5, 16);
	lv_obj_set_size(ui->SettingsPage4Info_settings_p4_info_title, 100, 20);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4Info_settings_p4_info_title, LV_SCROLLBAR_MODE_OFF);

	//Write style for SettingsPage4Info_settings_p4_info_title, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->SettingsPage4Info_settings_p4_info_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->SettingsPage4Info_settings_p4_info_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->SettingsPage4Info_settings_p4_info_title, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->SettingsPage4Info_settings_p4_info_title, &lv_font_simsun_16, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->SettingsPage4Info_settings_p4_info_title, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->SettingsPage4Info_settings_p4_info_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->SettingsPage4Info_settings_p4_info_title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->SettingsPage4Info_settings_p4_info_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->SettingsPage4Info_settings_p4_info_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->SettingsPage4Info_settings_p4_info_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->SettingsPage4Info_settings_p4_info_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->SettingsPage4Info_settings_p4_info_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->SettingsPage4Info_settings_p4_info_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SettingsPage4Info_settings_info_device_model_panel
	ui->SettingsPage4Info_settings_info_device_model_panel = lv_obj_create(ui->SettingsPage4Info);
	lv_obj_set_pos(ui->SettingsPage4Info_settings_info_device_model_panel, 12, 79);
	lv_obj_set_size(ui->SettingsPage4Info_settings_info_device_model_panel, 217, 55);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4Info_settings_info_device_model_panel, LV_SCROLLBAR_MODE_OFF);

	//Write style for SettingsPage4Info_settings_info_device_model_panel, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->SettingsPage4Info_settings_info_device_model_panel, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->SettingsPage4Info_settings_info_device_model_panel, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->SettingsPage4Info_settings_info_device_model_panel, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->SettingsPage4Info_settings_info_device_model_panel, 9, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->SettingsPage4Info_settings_info_device_model_panel, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->SettingsPage4Info_settings_info_device_model_panel, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->SettingsPage4Info_settings_info_device_model_panel, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->SettingsPage4Info_settings_info_device_model_panel, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->SettingsPage4Info_settings_info_device_model_panel, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->SettingsPage4Info_settings_info_device_model_panel, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->SettingsPage4Info_settings_info_device_model_panel, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SettingsPage4Info_STM32F4_label
	ui->SettingsPage4Info_STM32F4_label = lv_label_create(ui->SettingsPage4Info_settings_info_device_model_panel);
	lv_label_set_text(ui->SettingsPage4Info_STM32F4_label, "STM32F407");
	lv_label_set_long_mode(ui->SettingsPage4Info_STM32F4_label, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(ui->SettingsPage4Info_STM32F4_label, 96, 17);
	lv_obj_set_size(ui->SettingsPage4Info_STM32F4_label, 116, 21);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4Info_STM32F4_label, LV_SCROLLBAR_MODE_OFF);

	//Write style for SettingsPage4Info_STM32F4_label, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->SettingsPage4Info_STM32F4_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->SettingsPage4Info_STM32F4_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->SettingsPage4Info_STM32F4_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->SettingsPage4Info_STM32F4_label, &lv_font_simsun_16, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->SettingsPage4Info_STM32F4_label, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->SettingsPage4Info_STM32F4_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->SettingsPage4Info_STM32F4_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->SettingsPage4Info_STM32F4_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->SettingsPage4Info_STM32F4_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->SettingsPage4Info_STM32F4_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->SettingsPage4Info_STM32F4_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->SettingsPage4Info_STM32F4_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->SettingsPage4Info_STM32F4_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SettingsPage4Info_settings_info_device_model_label
	ui->SettingsPage4Info_settings_info_device_model_label = lv_label_create(ui->SettingsPage4Info_settings_info_device_model_panel);
	lv_label_set_text(ui->SettingsPage4Info_settings_info_device_model_label, "设备型号：");
	lv_label_set_long_mode(ui->SettingsPage4Info_settings_info_device_model_label, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(ui->SettingsPage4Info_settings_info_device_model_label, 6, 17);
	lv_obj_set_size(ui->SettingsPage4Info_settings_info_device_model_label, 94, 15);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4Info_settings_info_device_model_label, LV_SCROLLBAR_MODE_OFF);

	//Write style for SettingsPage4Info_settings_info_device_model_label, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->SettingsPage4Info_settings_info_device_model_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->SettingsPage4Info_settings_info_device_model_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->SettingsPage4Info_settings_info_device_model_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->SettingsPage4Info_settings_info_device_model_label, &lv_font_simsun_16, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->SettingsPage4Info_settings_info_device_model_label, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->SettingsPage4Info_settings_info_device_model_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->SettingsPage4Info_settings_info_device_model_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->SettingsPage4Info_settings_info_device_model_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->SettingsPage4Info_settings_info_device_model_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->SettingsPage4Info_settings_info_device_model_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->SettingsPage4Info_settings_info_device_model_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->SettingsPage4Info_settings_info_device_model_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->SettingsPage4Info_settings_info_device_model_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SettingsPage4Info_settings_info_hw_version_pane
	ui->SettingsPage4Info_settings_info_hw_version_pane = lv_obj_create(ui->SettingsPage4Info);
	lv_obj_set_pos(ui->SettingsPage4Info_settings_info_hw_version_pane, 12, 143);
	lv_obj_set_size(ui->SettingsPage4Info_settings_info_hw_version_pane, 217, 55);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4Info_settings_info_hw_version_pane, LV_SCROLLBAR_MODE_OFF);

	//Write style for SettingsPage4Info_settings_info_hw_version_pane, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->SettingsPage4Info_settings_info_hw_version_pane, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->SettingsPage4Info_settings_info_hw_version_pane, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->SettingsPage4Info_settings_info_hw_version_pane, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->SettingsPage4Info_settings_info_hw_version_pane, 9, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->SettingsPage4Info_settings_info_hw_version_pane, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->SettingsPage4Info_settings_info_hw_version_pane, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->SettingsPage4Info_settings_info_hw_version_pane, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->SettingsPage4Info_settings_info_hw_version_pane, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->SettingsPage4Info_settings_info_hw_version_pane, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->SettingsPage4Info_settings_info_hw_version_pane, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->SettingsPage4Info_settings_info_hw_version_pane, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SettingsPage4Info_settings_info_hw_version_label
	ui->SettingsPage4Info_settings_info_hw_version_label = lv_label_create(ui->SettingsPage4Info_settings_info_hw_version_pane);
	lv_label_set_text(ui->SettingsPage4Info_settings_info_hw_version_label, "硬件版本：");
	lv_label_set_long_mode(ui->SettingsPage4Info_settings_info_hw_version_label, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(ui->SettingsPage4Info_settings_info_hw_version_label, 9, 17);
	lv_obj_set_size(ui->SettingsPage4Info_settings_info_hw_version_label, 103, 18);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4Info_settings_info_hw_version_label, LV_SCROLLBAR_MODE_OFF);

	//Write style for SettingsPage4Info_settings_info_hw_version_label, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->SettingsPage4Info_settings_info_hw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->SettingsPage4Info_settings_info_hw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->SettingsPage4Info_settings_info_hw_version_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->SettingsPage4Info_settings_info_hw_version_label, &lv_font_simsun_16, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->SettingsPage4Info_settings_info_hw_version_label, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->SettingsPage4Info_settings_info_hw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->SettingsPage4Info_settings_info_hw_version_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->SettingsPage4Info_settings_info_hw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->SettingsPage4Info_settings_info_hw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->SettingsPage4Info_settings_info_hw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->SettingsPage4Info_settings_info_hw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->SettingsPage4Info_settings_info_hw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->SettingsPage4Info_settings_info_hw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SettingsPage4Info_hw_version_label
	ui->SettingsPage4Info_hw_version_label = lv_label_create(ui->SettingsPage4Info_settings_info_hw_version_pane);
	lv_label_set_text(ui->SettingsPage4Info_hw_version_label, "V1.0.0");
	lv_label_set_long_mode(ui->SettingsPage4Info_hw_version_label, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(ui->SettingsPage4Info_hw_version_label, 98, 18);
	lv_obj_set_size(ui->SettingsPage4Info_hw_version_label, 116, 21);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4Info_hw_version_label, LV_SCROLLBAR_MODE_OFF);

	//Write style for SettingsPage4Info_hw_version_label, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->SettingsPage4Info_hw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->SettingsPage4Info_hw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->SettingsPage4Info_hw_version_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->SettingsPage4Info_hw_version_label, &lv_font_simsun_16, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->SettingsPage4Info_hw_version_label, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->SettingsPage4Info_hw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->SettingsPage4Info_hw_version_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->SettingsPage4Info_hw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->SettingsPage4Info_hw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->SettingsPage4Info_hw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->SettingsPage4Info_hw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->SettingsPage4Info_hw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->SettingsPage4Info_hw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SettingsPage4Info_settings_info_sw_version_panel
	ui->SettingsPage4Info_settings_info_sw_version_panel = lv_obj_create(ui->SettingsPage4Info);
	lv_obj_set_pos(ui->SettingsPage4Info_settings_info_sw_version_panel, 12, 209);
	lv_obj_set_size(ui->SettingsPage4Info_settings_info_sw_version_panel, 217, 55);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4Info_settings_info_sw_version_panel, LV_SCROLLBAR_MODE_OFF);

	//Write style for SettingsPage4Info_settings_info_sw_version_panel, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->SettingsPage4Info_settings_info_sw_version_panel, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->SettingsPage4Info_settings_info_sw_version_panel, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->SettingsPage4Info_settings_info_sw_version_panel, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->SettingsPage4Info_settings_info_sw_version_panel, 9, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->SettingsPage4Info_settings_info_sw_version_panel, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->SettingsPage4Info_settings_info_sw_version_panel, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->SettingsPage4Info_settings_info_sw_version_panel, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->SettingsPage4Info_settings_info_sw_version_panel, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->SettingsPage4Info_settings_info_sw_version_panel, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->SettingsPage4Info_settings_info_sw_version_panel, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->SettingsPage4Info_settings_info_sw_version_panel, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SettingsPage4Info_settings_info_sw_version_label
	ui->SettingsPage4Info_settings_info_sw_version_label = lv_label_create(ui->SettingsPage4Info_settings_info_sw_version_panel);
	lv_label_set_text(ui->SettingsPage4Info_settings_info_sw_version_label, "软件版本：");
	lv_label_set_long_mode(ui->SettingsPage4Info_settings_info_sw_version_label, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(ui->SettingsPage4Info_settings_info_sw_version_label, 7, 15);
	lv_obj_set_size(ui->SettingsPage4Info_settings_info_sw_version_label, 103, 17);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4Info_settings_info_sw_version_label, LV_SCROLLBAR_MODE_OFF);

	//Write style for SettingsPage4Info_settings_info_sw_version_label, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->SettingsPage4Info_settings_info_sw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->SettingsPage4Info_settings_info_sw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->SettingsPage4Info_settings_info_sw_version_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->SettingsPage4Info_settings_info_sw_version_label, &lv_font_simsun_16, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->SettingsPage4Info_settings_info_sw_version_label, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->SettingsPage4Info_settings_info_sw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->SettingsPage4Info_settings_info_sw_version_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->SettingsPage4Info_settings_info_sw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->SettingsPage4Info_settings_info_sw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->SettingsPage4Info_settings_info_sw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->SettingsPage4Info_settings_info_sw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->SettingsPage4Info_settings_info_sw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->SettingsPage4Info_settings_info_sw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SettingsPage4Info_sw_version_label
	ui->SettingsPage4Info_sw_version_label = lv_label_create(ui->SettingsPage4Info_settings_info_sw_version_panel);
	lv_label_set_text(ui->SettingsPage4Info_sw_version_label, "V1.0.0");
	lv_label_set_long_mode(ui->SettingsPage4Info_sw_version_label, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(ui->SettingsPage4Info_sw_version_label, 98, 18);
	lv_obj_set_size(ui->SettingsPage4Info_sw_version_label, 116, 21);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4Info_sw_version_label, LV_SCROLLBAR_MODE_OFF);

	//Write style for SettingsPage4Info_sw_version_label, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->SettingsPage4Info_sw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->SettingsPage4Info_sw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->SettingsPage4Info_sw_version_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->SettingsPage4Info_sw_version_label, &lv_font_simsun_16, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->SettingsPage4Info_sw_version_label, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->SettingsPage4Info_sw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->SettingsPage4Info_sw_version_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->SettingsPage4Info_sw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->SettingsPage4Info_sw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->SettingsPage4Info_sw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->SettingsPage4Info_sw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->SettingsPage4Info_sw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->SettingsPage4Info_sw_version_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SettingsPage4Info_check_update_btn
	ui->SettingsPage4Info_check_update_btn = lv_btn_create(ui->SettingsPage4Info);
	ui->SettingsPage4Info_check_update_btn_label = lv_label_create(ui->SettingsPage4Info_check_update_btn);
	lv_label_set_text(ui->SettingsPage4Info_check_update_btn_label, "检查更新");
	lv_label_set_long_mode(ui->SettingsPage4Info_check_update_btn_label, LV_LABEL_LONG_WRAP);
	lv_obj_align(ui->SettingsPage4Info_check_update_btn_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(ui->SettingsPage4Info_check_update_btn, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->SettingsPage4Info_check_update_btn, 63, 275);
	lv_obj_set_size(ui->SettingsPage4Info_check_update_btn, 100, 38);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4Info_check_update_btn, LV_SCROLLBAR_MODE_OFF);

	//Write style for SettingsPage4Info_check_update_btn, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->SettingsPage4Info_check_update_btn, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->SettingsPage4Info_check_update_btn, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->SettingsPage4Info_check_update_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->SettingsPage4Info_check_update_btn, 13, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->SettingsPage4Info_check_update_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->SettingsPage4Info_check_update_btn, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->SettingsPage4Info_check_update_btn, &lv_font_simsun_16, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->SettingsPage4Info_check_update_btn, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SettingsPage4Info_aliyun_status_led
	ui->SettingsPage4Info_aliyun_status_led = lv_led_create(ui->SettingsPage4Info);
	lv_led_set_brightness(ui->SettingsPage4Info_aliyun_status_led, 255);
	lv_led_set_color(ui->SettingsPage4Info_aliyun_status_led, lv_color_hex(0x098D6B));
	lv_obj_set_pos(ui->SettingsPage4Info_aliyun_status_led, 187, 48);
	lv_obj_set_size(ui->SettingsPage4Info_aliyun_status_led, 16, 16);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4Info_aliyun_status_led, LV_SCROLLBAR_MODE_OFF);

	//Write codes SettingsPage4Info_wifi_status_label
	ui->SettingsPage4Info_wifi_status_label = lv_label_create(ui->SettingsPage4Info);
	lv_label_set_text(ui->SettingsPage4Info_wifi_status_label, "WIFI状态");
	lv_label_set_long_mode(ui->SettingsPage4Info_wifi_status_label, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(ui->SettingsPage4Info_wifi_status_label, 5, 47);
	lv_obj_set_size(ui->SettingsPage4Info_wifi_status_label, 63, 17);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4Info_wifi_status_label, LV_SCROLLBAR_MODE_OFF);

	//Write style for SettingsPage4Info_wifi_status_label, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->SettingsPage4Info_wifi_status_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->SettingsPage4Info_wifi_status_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->SettingsPage4Info_wifi_status_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->SettingsPage4Info_wifi_status_label, &lv_font_simsun_12, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->SettingsPage4Info_wifi_status_label, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->SettingsPage4Info_wifi_status_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->SettingsPage4Info_wifi_status_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->SettingsPage4Info_wifi_status_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->SettingsPage4Info_wifi_status_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->SettingsPage4Info_wifi_status_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->SettingsPage4Info_wifi_status_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->SettingsPage4Info_wifi_status_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->SettingsPage4Info_wifi_status_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SettingsPage4Info_aliyun_status_label
	ui->SettingsPage4Info_aliyun_status_label = lv_label_create(ui->SettingsPage4Info);
	lv_label_set_text(ui->SettingsPage4Info_aliyun_status_label, "入云状态");
	lv_label_set_long_mode(ui->SettingsPage4Info_aliyun_status_label, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(ui->SettingsPage4Info_aliyun_status_label, 116, 47);
	lv_obj_set_size(ui->SettingsPage4Info_aliyun_status_label, 63, 17);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4Info_aliyun_status_label, LV_SCROLLBAR_MODE_OFF);

	//Write style for SettingsPage4Info_aliyun_status_label, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->SettingsPage4Info_aliyun_status_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->SettingsPage4Info_aliyun_status_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->SettingsPage4Info_aliyun_status_label, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->SettingsPage4Info_aliyun_status_label, &lv_font_simsun_12, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->SettingsPage4Info_aliyun_status_label, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->SettingsPage4Info_aliyun_status_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->SettingsPage4Info_aliyun_status_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->SettingsPage4Info_aliyun_status_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->SettingsPage4Info_aliyun_status_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->SettingsPage4Info_aliyun_status_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->SettingsPage4Info_aliyun_status_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->SettingsPage4Info_aliyun_status_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->SettingsPage4Info_aliyun_status_label, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SettingsPage4Info_wifi_status_led
	ui->SettingsPage4Info_wifi_status_led = lv_led_create(ui->SettingsPage4Info);
	lv_led_set_brightness(ui->SettingsPage4Info_wifi_status_led, 255);
	lv_led_set_color(ui->SettingsPage4Info_wifi_status_led, lv_color_hex(0x098D6B));
	lv_obj_set_pos(ui->SettingsPage4Info_wifi_status_led, 74, 48);
	lv_obj_set_size(ui->SettingsPage4Info_wifi_status_led, 16, 16);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4Info_wifi_status_led, LV_SCROLLBAR_MODE_OFF);

	//Update current screen layout.
	lv_obj_update_layout(ui->SettingsPage4Info);

	/* 本页控件事件绑定：检查更新按钮点击 → 检查是否有新版本（回调在 lvgl_action.c） */
	lv_obj_add_event_cb(ui->SettingsPage4Info_check_update_btn,
	                    settings_p4_info_check_update, LV_EVENT_CLICKED, NULL);


}
