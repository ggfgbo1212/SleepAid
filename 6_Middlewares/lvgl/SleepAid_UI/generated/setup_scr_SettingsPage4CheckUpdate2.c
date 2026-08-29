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


void setup_scr_SettingsPage4CheckUpdate2(lv_ui *ui)
{
	//Write codes SettingsPage4CheckUpdate2
	ui->SettingsPage4CheckUpdate2 = lv_obj_create(NULL);
	lv_obj_set_size(ui->SettingsPage4CheckUpdate2, 240, 320);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4CheckUpdate2, LV_SCROLLBAR_MODE_OFF);

	//Write style for SettingsPage4CheckUpdate2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->SettingsPage4CheckUpdate2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->SettingsPage4CheckUpdate2, lv_color_hex(0xebeef5), LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SettingsPage4CheckUpdate2_settings_p4_checkupdate2_title
	ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_title = lv_label_create(ui->SettingsPage4CheckUpdate2);
	lv_label_set_text(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_title, "检查更新");
	lv_label_set_long_mode(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_title, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_title, 63, 57);
	lv_obj_set_size(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_title, 114, 28);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_title, LV_SCROLLBAR_MODE_OFF);

	//Write style for SettingsPage4CheckUpdate2_settings_p4_checkupdate2_title, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_title, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_title, &lv_font_simsun_25, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_title, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SettingsPage4CheckUpdate2_settings_p4_checkupdate2_content
	ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_content = lv_label_create(ui->SettingsPage4CheckUpdate2);
	lv_label_set_text(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_content, "你的软件已是最新版本，无需更新");
	lv_label_set_long_mode(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_content, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_content, 58, 109);
	lv_obj_set_size(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_content, 125, 52);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_content, LV_SCROLLBAR_MODE_OFF);

	//Write style for SettingsPage4CheckUpdate2_settings_p4_checkupdate2_content, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_content, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_content, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_content, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_content, &lv_font_simsun_16, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_content, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_content, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_content, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_content, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_content, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_content, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_content, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_content, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->SettingsPage4CheckUpdate2_settings_p4_checkupdate2_content, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SettingsPage4CheckUpdate2_setting_checkupdate2_confirm_btn
	ui->SettingsPage4CheckUpdate2_setting_checkupdate2_confirm_btn = lv_btn_create(ui->SettingsPage4CheckUpdate2);
	ui->SettingsPage4CheckUpdate2_setting_checkupdate2_confirm_btn_label = lv_label_create(ui->SettingsPage4CheckUpdate2_setting_checkupdate2_confirm_btn);
	lv_label_set_text(ui->SettingsPage4CheckUpdate2_setting_checkupdate2_confirm_btn_label, "确认");
	lv_label_set_long_mode(ui->SettingsPage4CheckUpdate2_setting_checkupdate2_confirm_btn_label, LV_LABEL_LONG_WRAP);
	lv_obj_align(ui->SettingsPage4CheckUpdate2_setting_checkupdate2_confirm_btn_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(ui->SettingsPage4CheckUpdate2_setting_checkupdate2_confirm_btn, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->SettingsPage4CheckUpdate2_setting_checkupdate2_confirm_btn, 70, 196);
	lv_obj_set_size(ui->SettingsPage4CheckUpdate2_setting_checkupdate2_confirm_btn, 100, 38);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4CheckUpdate2_setting_checkupdate2_confirm_btn, LV_SCROLLBAR_MODE_OFF);

	//Write style for SettingsPage4CheckUpdate2_setting_checkupdate2_confirm_btn, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->SettingsPage4CheckUpdate2_setting_checkupdate2_confirm_btn, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->SettingsPage4CheckUpdate2_setting_checkupdate2_confirm_btn, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->SettingsPage4CheckUpdate2_setting_checkupdate2_confirm_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->SettingsPage4CheckUpdate2_setting_checkupdate2_confirm_btn, 13, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->SettingsPage4CheckUpdate2_setting_checkupdate2_confirm_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->SettingsPage4CheckUpdate2_setting_checkupdate2_confirm_btn, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->SettingsPage4CheckUpdate2_setting_checkupdate2_confirm_btn, &lv_font_simsun_16, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->SettingsPage4CheckUpdate2_setting_checkupdate2_confirm_btn, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Update current screen layout.
	lv_obj_update_layout(ui->SettingsPage4CheckUpdate2);

	
}
