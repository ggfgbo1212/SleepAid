/*
* Copyright 2026 NXP
* NXP Confidential and Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef GUI_GUIDER_H
#define GUI_GUIDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

typedef struct
{
  
	lv_obj_t *SettingsPage4Info;
	bool SettingsPage4Info_del;
	lv_obj_t *SettingsPage4Info_settings_p4_info_image;
	lv_obj_t *SettingsPage4Info_settings_p4_info_title;
	lv_obj_t *SettingsPage4Info_settings_info_device_model_panel;
	lv_obj_t *SettingsPage4Info_STM32F4_label;
	lv_obj_t *SettingsPage4Info_settings_info_device_model_label;
	lv_obj_t *SettingsPage4Info_settings_info_hw_version_pane;
	lv_obj_t *SettingsPage4Info_settings_info_hw_version_label;
	lv_obj_t *SettingsPage4Info_hw_version_label;
	lv_obj_t *SettingsPage4Info_settings_info_sw_version_panel;
	lv_obj_t *SettingsPage4Info_settings_info_sw_version_label;
	lv_obj_t *SettingsPage4Info_sw_version_label;
	lv_obj_t *SettingsPage4Info_check_update_btn;
	lv_obj_t *SettingsPage4Info_check_update_btn_label;
	lv_obj_t *SettingsPage4Info_aliyun_status_led;
	lv_obj_t *SettingsPage4Info_wifi_status_label;
	lv_obj_t *SettingsPage4Info_aliyun_status_label;
	lv_obj_t *SettingsPage4Info_wifi_status_led;
	lv_obj_t *SettingsPage4CheckUpdate1;
	bool SettingsPage4CheckUpdate1_del;
	lv_obj_t *SettingsPage4CheckUpdate1_settings_p4_checkupdate1_title;
	lv_obj_t *SettingsPage4CheckUpdate1_settings_p4_checkupdate1_content;
	lv_obj_t *SettingsPage4CheckUpdate2;
	bool SettingsPage4CheckUpdate2_del;
	lv_obj_t *SettingsPage4CheckUpdate2_settings_p4_checkupdate2_title;
	lv_obj_t *SettingsPage4CheckUpdate2_settings_p4_checkupdate2_content;
	lv_obj_t *SettingsPage4CheckUpdate2_setting_checkupdate2_confirm_btn;
	lv_obj_t *SettingsPage4CheckUpdate2_setting_checkupdate2_confirm_btn_label;
	lv_obj_t *SettingsPage4CheckUpdate3;
	bool SettingsPage4CheckUpdate3_del;
	lv_obj_t *SettingsPage4CheckUpdate3_settings_p4_checkupdate3_title;
	lv_obj_t *SettingsPage4CheckUpdate3_settings_p4_checkupdate3_content;
	lv_obj_t *SettingsPage4CheckUpdate3_setting_checkupdate3_confirm_btn;
	lv_obj_t *SettingsPage4CheckUpdate3_setting_checkupdate3_confirm_btn_label;
	lv_obj_t *SettingsPage4Updating;
	bool SettingsPage4Updating_del;
	lv_obj_t *SettingsPage4Updating_settings_p4_updating_title;
	lv_obj_t *SettingsPage4Updating_settings_p4_updating_content;
	lv_obj_t *SettingsPage4Updating_settings_p4_updating_bar;
	lv_obj_t *SettingsPage4UpdateCplt;
	bool SettingsPage4UpdateCplt_del;
	lv_obj_t *SettingsPage4UpdateCplt_settings_updatecplt_confirm_btn;
	lv_obj_t *SettingsPage4UpdateCplt_settings_updatecplt_confirm_btn_label;
	lv_obj_t *SettingsPage4UpdateCplt_settings_p4_updatecplt_icon;
	lv_obj_t *SettingsPage4UpdateError;
	bool SettingsPage4UpdateError_del;
	lv_obj_t *SettingsPage4UpdateError_settings_p4_updateerror_icon;
	lv_obj_t *SettingsPage4UpdateError_settings_updateerror_confirm_btn;
	lv_obj_t *SettingsPage4UpdateError_settings_updateerror_confirm_btn_label;
}lv_ui;

void ui_init_style(lv_style_t * style);
void init_scr_del_flag(lv_ui *ui);
void setup_ui(lv_ui *ui);
extern lv_ui guider_ui;

void setup_scr_SettingsPage4Info(lv_ui *ui);
void setup_scr_SettingsPage4CheckUpdate1(lv_ui *ui);
void setup_scr_SettingsPage4CheckUpdate2(lv_ui *ui);
void setup_scr_SettingsPage4CheckUpdate3(lv_ui *ui);
void setup_scr_SettingsPage4Updating(lv_ui *ui);
void setup_scr_SettingsPage4UpdateCplt(lv_ui *ui);
void setup_scr_SettingsPage4UpdateError(lv_ui *ui);
LV_IMG_DECLARE(_trademark_alpha_34x34);
LV_IMG_DECLARE(_download_alpha_185x135);
LV_IMG_DECLARE(_downloadx_alpha_185x135);

LV_FONT_DECLARE(lv_font_simsun_16)
LV_FONT_DECLARE(lv_font_montserratMedium_16)
LV_FONT_DECLARE(lv_font_montserratMedium_12)
LV_FONT_DECLARE(lv_font_simsun_12)
LV_FONT_DECLARE(lv_font_simsun_25)


#ifdef __cplusplus
}
#endif
#endif
