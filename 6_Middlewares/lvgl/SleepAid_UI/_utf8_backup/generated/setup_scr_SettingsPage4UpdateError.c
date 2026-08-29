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


void setup_scr_SettingsPage4UpdateError(lv_ui *ui)
{
	//Write codes SettingsPage4UpdateError
	ui->SettingsPage4UpdateError = lv_obj_create(NULL);
	lv_obj_set_size(ui->SettingsPage4UpdateError, 240, 320);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4UpdateError, LV_SCROLLBAR_MODE_OFF);

	//Write style for SettingsPage4UpdateError, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->SettingsPage4UpdateError, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->SettingsPage4UpdateError, lv_color_hex(0xebeef5), LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SettingsPage4UpdateError_settings_p4_updateerror_icon
	ui->SettingsPage4UpdateError_settings_p4_updateerror_icon = lv_img_create(ui->SettingsPage4UpdateError);
	lv_obj_add_flag(ui->SettingsPage4UpdateError_settings_p4_updateerror_icon, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->SettingsPage4UpdateError_settings_p4_updateerror_icon, &_downloadx_alpha_185x135);
	lv_img_set_pivot(ui->SettingsPage4UpdateError_settings_p4_updateerror_icon, 50,50);
	lv_img_set_angle(ui->SettingsPage4UpdateError_settings_p4_updateerror_icon, 0);
	lv_obj_set_pos(ui->SettingsPage4UpdateError_settings_p4_updateerror_icon, 28, 40);
	lv_obj_set_size(ui->SettingsPage4UpdateError_settings_p4_updateerror_icon, 185, 135);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4UpdateError_settings_p4_updateerror_icon, LV_SCROLLBAR_MODE_OFF);

	//Write style for SettingsPage4UpdateError_settings_p4_updateerror_icon, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_opa(ui->SettingsPage4UpdateError_settings_p4_updateerror_icon, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes SettingsPage4UpdateError_settings_updateerror_confirm_btn
	ui->SettingsPage4UpdateError_settings_updateerror_confirm_btn = lv_btn_create(ui->SettingsPage4UpdateError);
	ui->SettingsPage4UpdateError_settings_updateerror_confirm_btn_label = lv_label_create(ui->SettingsPage4UpdateError_settings_updateerror_confirm_btn);
	lv_label_set_text(ui->SettingsPage4UpdateError_settings_updateerror_confirm_btn_label, "确认");
	lv_label_set_long_mode(ui->SettingsPage4UpdateError_settings_updateerror_confirm_btn_label, LV_LABEL_LONG_WRAP);
	lv_obj_align(ui->SettingsPage4UpdateError_settings_updateerror_confirm_btn_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(ui->SettingsPage4UpdateError_settings_updateerror_confirm_btn, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->SettingsPage4UpdateError_settings_updateerror_confirm_btn, 70, 200);
	lv_obj_set_size(ui->SettingsPage4UpdateError_settings_updateerror_confirm_btn, 100, 38);
	lv_obj_set_scrollbar_mode(ui->SettingsPage4UpdateError_settings_updateerror_confirm_btn, LV_SCROLLBAR_MODE_OFF);

	//Write style for SettingsPage4UpdateError_settings_updateerror_confirm_btn, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->SettingsPage4UpdateError_settings_updateerror_confirm_btn, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->SettingsPage4UpdateError_settings_updateerror_confirm_btn, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->SettingsPage4UpdateError_settings_updateerror_confirm_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->SettingsPage4UpdateError_settings_updateerror_confirm_btn, 13, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->SettingsPage4UpdateError_settings_updateerror_confirm_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->SettingsPage4UpdateError_settings_updateerror_confirm_btn, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->SettingsPage4UpdateError_settings_updateerror_confirm_btn, &lv_font_simsun_16, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->SettingsPage4UpdateError_settings_updateerror_confirm_btn, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Update current screen layout.
	lv_obj_update_layout(ui->SettingsPage4UpdateError);

	
}
