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


void setup_scr_SettingsPage4Updating(lv_ui *ui)
{
	//Write codes SettingsPage4Updating
	ui->SettingsPage4Updating = lv_obj_create(NULL);
	lv_obj_set_size(ui->SettingsPage4Updating, 240, 320);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4Updating, LV_SCROLLBAR_MODE_OFF);

	//Write style for SettingsPage4Updating, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->SettingsPage4Updating, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->SettingsPage4Updating, lv_color_hex(0xebeef5), LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SettingsPage4Updating_settings_p4_updating_title
	ui->SettingsPage4Updating_settings_p4_updating_title = lv_label_create(ui->SettingsPage4Updating);
	lv_label_set_text(ui->SettingsPage4Updating_settings_p4_updating_title, "升级版本");
	lv_label_set_long_mode(ui->SettingsPage4Updating_settings_p4_updating_title, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(ui->SettingsPage4Updating_settings_p4_updating_title, 63, 57);
	lv_obj_set_size(ui->SettingsPage4Updating_settings_p4_updating_title, 114, 28);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4Updating_settings_p4_updating_title, LV_SCROLLBAR_MODE_OFF);

	//Write style for SettingsPage4Updating_settings_p4_updating_title, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->SettingsPage4Updating_settings_p4_updating_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->SettingsPage4Updating_settings_p4_updating_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->SettingsPage4Updating_settings_p4_updating_title, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->SettingsPage4Updating_settings_p4_updating_title, &lv_font_simsun_25, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->SettingsPage4Updating_settings_p4_updating_title, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->SettingsPage4Updating_settings_p4_updating_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->SettingsPage4Updating_settings_p4_updating_title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->SettingsPage4Updating_settings_p4_updating_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->SettingsPage4Updating_settings_p4_updating_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->SettingsPage4Updating_settings_p4_updating_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->SettingsPage4Updating_settings_p4_updating_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->SettingsPage4Updating_settings_p4_updating_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->SettingsPage4Updating_settings_p4_updating_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SettingsPage4Updating_settings_p4_updating_content
	ui->SettingsPage4Updating_settings_p4_updating_content = lv_label_create(ui->SettingsPage4Updating);
	lv_label_set_text(ui->SettingsPage4Updating_settings_p4_updating_content, "升级中，请耐心等待");
	lv_label_set_long_mode(ui->SettingsPage4Updating_settings_p4_updating_content, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(ui->SettingsPage4Updating_settings_p4_updating_content, 33, 113);
	lv_obj_set_size(ui->SettingsPage4Updating_settings_p4_updating_content, 175, 23);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4Updating_settings_p4_updating_content, LV_SCROLLBAR_MODE_OFF);

	//Write style for SettingsPage4Updating_settings_p4_updating_content, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->SettingsPage4Updating_settings_p4_updating_content, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->SettingsPage4Updating_settings_p4_updating_content, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->SettingsPage4Updating_settings_p4_updating_content, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->SettingsPage4Updating_settings_p4_updating_content, &lv_font_simsun_16, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->SettingsPage4Updating_settings_p4_updating_content, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->SettingsPage4Updating_settings_p4_updating_content, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->SettingsPage4Updating_settings_p4_updating_content, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->SettingsPage4Updating_settings_p4_updating_content, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->SettingsPage4Updating_settings_p4_updating_content, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->SettingsPage4Updating_settings_p4_updating_content, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->SettingsPage4Updating_settings_p4_updating_content, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->SettingsPage4Updating_settings_p4_updating_content, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->SettingsPage4Updating_settings_p4_updating_content, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SettingsPage4Updating_settings_p4_updating_bar
	ui->SettingsPage4Updating_settings_p4_updating_bar = lv_bar_create(ui->SettingsPage4Updating);
	lv_obj_set_style_anim_time(ui->SettingsPage4Updating_settings_p4_updating_bar, 1000, 0);
	lv_bar_set_mode(ui->SettingsPage4Updating_settings_p4_updating_bar, LV_BAR_MODE_NORMAL);
	lv_bar_set_value(ui->SettingsPage4Updating_settings_p4_updating_bar, 0, LV_ANIM_OFF);
	lv_obj_set_pos(ui->SettingsPage4Updating_settings_p4_updating_bar, 37, 191);
	lv_obj_set_size(ui->SettingsPage4Updating_settings_p4_updating_bar, 167, 18);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4Updating_settings_p4_updating_bar, LV_SCROLLBAR_MODE_OFF);

	//Write style for SettingsPage4Updating_settings_p4_updating_bar, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->SettingsPage4Updating_settings_p4_updating_bar, 60, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->SettingsPage4Updating_settings_p4_updating_bar, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->SettingsPage4Updating_settings_p4_updating_bar, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->SettingsPage4Updating_settings_p4_updating_bar, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for SettingsPage4Updating_settings_p4_updating_bar, Part: LV_PART_INDICATOR, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->SettingsPage4Updating_settings_p4_updating_bar, 255, LV_PART_INDICATOR|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->SettingsPage4Updating_settings_p4_updating_bar, lv_color_hex(0x2195f6), LV_PART_INDICATOR|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->SettingsPage4Updating_settings_p4_updating_bar, 10, LV_PART_INDICATOR|LV_STATE_DEFAULT);

	//Update current screen layout.
	lv_obj_update_layout(ui->SettingsPage4Updating);

	
}
