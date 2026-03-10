/*
 * SPDX-FileCopyrightText: 2021-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_log.h"
#include "hd44780.h"
#include <esp_idf_lib_helpers.h>

#include "esp_bt.h"
#include "bt_app_core.h"
#include "bt_app_av.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"\


#define next GPIO_NUM_22           //Next button pin
#define play GPIO_NUM_21           //Play/Pause button pin
#define leftVol ADC_CHANNEL_4      //Left Volume adjusting potentiometer pin
#define rightVol ADC_CHANNEL_5     //Right volume adjusting potentiometer pin
#define ADC_ATTEN ADC_ATTEN_DB_12  //ADC Attenuation
#define BITWIDTH ADC_BITWIDTH_12   //ADC Bitwidth

/* device name */
static const char local_device_name[] = "BC SPECIALS";

static const uint8_t char_data[] =
{
    0x00, 0x04, 0x06, 0x05, 0x05, 0x04, 0x1C, 0x1C,
    0x00, 0x0A, 0x00, 0x0E, 0x0A, 0x0A, 0x0E, 0x00
};

const char mesg1[] = "hey yall look at me im scrolling wooooooooo look at me go yayyy     ";
const char mesg2[] = "hey yall its me again look at us yayyyyy yipee wahoooo     ";
char TITLE[128];
char ARTIST[128];

typedef enum {INIT, PL_WAIT, NX_WAIT, PLAY, NEXT} State_t;

volatile bool songPlaying = false;

/* event for stack up */
enum {
    BT_APP_EVT_STACK_UP = 0,
};

/********************************
 * STATIC FUNCTION DECLARATIONS
 *******************************/

/* Device callback function */
static void bt_app_dev_cb(esp_bt_dev_cb_event_t event, esp_bt_dev_cb_param_t *param);
/* GAP callback function */
static void bt_app_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param);
/* handler for bluetooth stack enabled events */
static void bt_av_hdl_stack_evt(uint16_t event, void *p_param);

/*******************************
 * STATIC FUNCTION DEFINITIONS
 ******************************/
void avrc_ct_cb(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *param)
{
    if (event == ESP_AVRC_CT_METADATA_RSP_EVT) {

        /* --- TITLE --- */
        if (param->meta_rsp.attr_id == ESP_AVRC_MD_ATTR_TITLE) {

            const uint8_t *src = param->meta_rsp.attr_text;
            size_t len = param->meta_rsp.attr_length;

            size_t max = sizeof(TITLE) - 1;
            size_t copy_len = (len < max) ? len : max;

            memcpy(TITLE, src, copy_len);
            TITLE[copy_len] = '\0';

            printf("Title: %s\n", TITLE);
        }

        /* --- ARTIST --- */
        if (param->meta_rsp.attr_id == ESP_AVRC_MD_ATTR_ARTIST) {

            const uint8_t *src = param->meta_rsp.attr_text;
            size_t len = param->meta_rsp.attr_length;

            size_t max = sizeof(ARTIST) - 1;
            size_t copy_len = (len < max) ? len : max;

            memcpy(ARTIST, src, copy_len);
            ARTIST[copy_len] = '\0';

            printf("Artist: %s\n", ARTIST);
        }
    }
}

void lcd(void *pvParameters){
    static hd44780_t lcd =
    {
        .write_cb = NULL,
        .font = HD44780_FONT_5X8,
        .lines = 2,
        .pins = {
            .rs = GPIO_NUM_19,
            .e  = GPIO_NUM_18,
            .d4 = GPIO_NUM_5,
            .d5 = GPIO_NUM_17,
            .d6 = GPIO_NUM_16,
            .d7 = GPIO_NUM_4,
            .bl = HD44780_NOT_USED
        }
    };

    vTaskDelay(50/portTICK_PERIOD_MS);
    //ESP_ERROR_CHECK(hd44780_init(&lcd));
    hd44780_init(&lcd);
   
    hd44780_upload_character(&lcd, 3, char_data);
    hd44780_upload_character(&lcd, 4, char_data + 8);

    hd44780_clear(&lcd);

    while (1)
    {
        static uint8_t pos1;
        static uint8_t pos2;
        hd44780_gotoxy(&lcd, 0, 0);
        hd44780_putc(&lcd, 3);
        hd44780_gotoxy(&lcd, 2, 0);
        for (uint8_t i = 0; i < 14; i++) {
            char c = mesg1[(pos1 + i) % (sizeof(mesg1) - 1)];
            hd44780_putc(&lcd, c);
        }
        pos1 = (pos1 + 1) % (sizeof(mesg1) - 1);
           
        hd44780_gotoxy(&lcd, 0, 1);
        hd44780_putc(&lcd, 4);
        hd44780_gotoxy(&lcd, 2, 1);
        for (uint8_t i = 0; i < 14; i++) {
            char c = mesg2[(pos2 + i) % (sizeof(mesg2) - 1)];
            hd44780_putc(&lcd, c);
        }
        pos2 = (pos2 + 1) % (sizeof(mesg2) - 1);
        vTaskDelay(pdMS_TO_TICKS(400));
    }
}



void buttonHandler(){
    State_t state = INIT;
    songPlaying = true;
    //bool playPressed = false;
    //bool nextPressed = false;
    for(;;){
        vTaskDelay(10/portTICK_PERIOD_MS);
        switch(state){
        case INIT:
            if (gpio_get_level(play) == 1){
                //playPressed = true;
                state = PL_WAIT;
            }
            else if (gpio_get_level(next) == 1){
                //nextPressed = true;
                state = NX_WAIT;
            }
            else {
                state = INIT;
            }
            break;
        case PL_WAIT:
            if (gpio_get_level(play) == 0){
                //playPressed = false;
                state = PLAY;
            }
            break;
        case NX_WAIT:
            if (gpio_get_level(next) == 0){
                //nextPressed = false;
                state = NEXT;
            }
            break; 
        case PLAY:
            if (songPlaying){
                esp_avrc_ct_send_passthrough_cmd(0, ESP_AVRC_PT_CMD_PAUSE, ESP_AVRC_PT_CMD_STATE_PRESSED);
            }
            else {
                esp_avrc_ct_send_passthrough_cmd(0, ESP_AVRC_PT_CMD_PLAY, ESP_AVRC_PT_CMD_STATE_PRESSED);
            }
            songPlaying = !songPlaying;
            state = INIT;
            break;
        case NEXT:
            esp_avrc_ct_send_passthrough_cmd(0, ESP_AVRC_PT_CMD_FORWARD, ESP_AVRC_PT_CMD_STATE_PRESSED);
            state = INIT;
            break;
        }

    }
}

void config(){
    gpio_reset_pin(play);
    gpio_set_direction(play, GPIO_MODE_INPUT);
    gpio_pulldown_en(play);

    gpio_reset_pin(next);
    gpio_set_direction(next, GPIO_MODE_INPUT);
    gpio_pulldown_en(next);

    gpio_pulldown_en(CONFIG_EXAMPLE_I2S_DATA_PIN);
}

static char *bda2str(uint8_t * bda, char *str, size_t size)
{
    if (bda == NULL || str == NULL || size < 18) {
        return NULL;
    }

    uint8_t *p = bda;
    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
            p[0], p[1], p[2], p[3], p[4], p[5]);
    return str;
}

static void bt_app_dev_cb(esp_bt_dev_cb_event_t event, esp_bt_dev_cb_param_t *param)
{
    switch (event) {
    case ESP_BT_DEV_NAME_RES_EVT: {
        if (param->name_res.status == ESP_BT_STATUS_SUCCESS) {
            ESP_LOGI(BT_AV_TAG, "Get local device name success: %s", param->name_res.name);
        } else {
            ESP_LOGE(BT_AV_TAG, "Get local device name failed, status: %d", param->name_res.status);
        }
        break;
    }
    default: {
        ESP_LOGI(BT_AV_TAG, "event: %d", event);
        break;
    }
    }
}

static void bt_app_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    uint8_t *bda = NULL;

    switch (event) {
    /* when authentication completed, this event comes */
    case ESP_BT_GAP_AUTH_CMPL_EVT: {
        if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
            ESP_LOGI(BT_AV_TAG, "authentication success: %s", param->auth_cmpl.device_name);
            ESP_LOG_BUFFER_HEX(BT_AV_TAG, param->auth_cmpl.bda, ESP_BD_ADDR_LEN);
        } else {
            ESP_LOGE(BT_AV_TAG, "authentication failed, status: %d", param->auth_cmpl.stat);
        }
        ESP_LOGI(BT_AV_TAG, "link key type of current link is: %d", param->auth_cmpl.lk_type);
        break;
    }
    case ESP_BT_GAP_ENC_CHG_EVT: {
        char *str_enc[3] = {"OFF", "E0", "AES"};
        bda = (uint8_t *)param->enc_chg.bda;
        ESP_LOGI(BT_AV_TAG, "Encryption mode to [%02x:%02x:%02x:%02x:%02x:%02x] changed to %s",
                 bda[0], bda[1], bda[2], bda[3], bda[4], bda[5], str_enc[param->enc_chg.enc_mode]);
        break;
    }

#if (CONFIG_EXAMPLE_A2DP_SINK_SSP_ENABLED == true)
    /* when Security Simple Pairing user confirmation requested, this event comes */
    case ESP_BT_GAP_CFM_REQ_EVT:
        ESP_LOGI(BT_AV_TAG, "ESP_BT_GAP_CFM_REQ_EVT Please compare the numeric value: %06"PRIu32, param->cfm_req.num_val);
        esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
        break;
    /* when Security Simple Pairing passkey notified, this event comes */
    case ESP_BT_GAP_KEY_NOTIF_EVT:
        ESP_LOGI(BT_AV_TAG, "ESP_BT_GAP_KEY_NOTIF_EVT passkey: %06"PRIu32, param->key_notif.passkey);
        break;
    /* when Security Simple Pairing passkey requested, this event comes */
    case ESP_BT_GAP_KEY_REQ_EVT:
        ESP_LOGI(BT_AV_TAG, "ESP_BT_GAP_KEY_REQ_EVT Please enter passkey!");
        break;
#endif

    /* when GAP mode changed, this event comes */
    case ESP_BT_GAP_MODE_CHG_EVT:
        ESP_LOGI(BT_AV_TAG, "ESP_BT_GAP_MODE_CHG_EVT mode: %d, interval: %.2f ms",
                 param->mode_chg.mode, param->mode_chg.interval * 0.625);
        break;
    /* when ACL connection completed, this event comes */
    case ESP_BT_GAP_ACL_CONN_CMPL_STAT_EVT:
        bda = (uint8_t *)param->acl_conn_cmpl_stat.bda;
        ESP_LOGI(BT_AV_TAG, "ESP_BT_GAP_ACL_CONN_CMPL_STAT_EVT Connected to [%02x:%02x:%02x:%02x:%02x:%02x], status: 0x%x",
                 bda[0], bda[1], bda[2], bda[3], bda[4], bda[5], param->acl_conn_cmpl_stat.stat);
        break;
    /* when ACL disconnection completed, this event comes */
    case ESP_BT_GAP_ACL_DISCONN_CMPL_STAT_EVT:
        bda = (uint8_t *)param->acl_disconn_cmpl_stat.bda;
        ESP_LOGI(BT_AV_TAG, "ESP_BT_GAP_ACL_DISC_CMPL_STAT_EVT Disconnected from [%02x:%02x:%02x:%02x:%02x:%02x], reason: 0x%x",
                 bda[0], bda[1], bda[2], bda[3], bda[4], bda[5], param->acl_disconn_cmpl_stat.reason);
        break;
    /* others */
    default: {
        ESP_LOGI(BT_AV_TAG, "event: %d", event);
        break;
    }
    }
}

static void bt_av_hdl_stack_evt(uint16_t event, void *p_param)
{
    ESP_LOGD(BT_AV_TAG, "%s event: %d", __func__, event);

    switch (event) {
    /* when do the stack up, this event comes */
    case BT_APP_EVT_STACK_UP: {
        esp_bt_gap_set_device_name(local_device_name);
        esp_bt_dev_register_callback(bt_app_dev_cb);
        esp_bt_gap_register_callback(bt_app_gap_cb);

        esp_avrc_ct_register_callback(bt_app_rc_ct_cb);
        assert(esp_avrc_ct_init() == ESP_OK);
        esp_avrc_tg_register_callback(bt_app_rc_tg_cb);
        assert(esp_avrc_tg_init() == ESP_OK);

        esp_avrc_rn_evt_cap_mask_t evt_set = {0};
        esp_avrc_rn_evt_bit_mask_operation(ESP_AVRC_BIT_MASK_OP_SET, &evt_set, ESP_AVRC_RN_VOLUME_CHANGE);
        assert(esp_avrc_tg_set_rn_evt_cap(&evt_set) == ESP_OK);

        esp_a2d_register_callback(&bt_app_a2d_cb);
        assert(esp_a2d_sink_init() == ESP_OK);

#if CONFIG_EXAMPLE_A2DP_SINK_USE_EXTERNAL_CODEC == FALSE
        esp_a2d_sink_register_data_callback(bt_app_a2d_data_cb);
#else
        esp_a2d_mcc_t mcc = {0};
        mcc.type = ESP_A2D_MCT_SBC;
        mcc.cie.sbc_info.samp_freq = 0xf;
        mcc.cie.sbc_info.ch_mode = 0xf;
        mcc.cie.sbc_info.block_len = 0xf;
        mcc.cie.sbc_info.num_subbands = 0x3;
        mcc.cie.sbc_info.alloc_mthd = 0x3;
        mcc.cie.sbc_info.max_bitpool = 250;
        mcc.cie.sbc_info.min_bitpool = 2;
        /* register stream end point, only support mSBC currently */
        esp_a2d_sink_register_stream_endpoint(0, &mcc);
        esp_a2d_sink_register_audio_data_callback(bt_app_a2d_audio_data_cb);
#endif
        /* Get the default value of the delay value */
        esp_a2d_sink_get_delay_value();
        /* Get local device name */
        esp_bt_gap_get_device_name();

        /* set discoverable and connectable mode, wait to be connected */
        esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
        break;
    }
    /* others */
    default:
        ESP_LOGE(BT_AV_TAG, "%s unhandled event: %d", __func__, event);
        break;
    }
}

/*******************************
 * MAIN ENTRY POINT
 ******************************/

void app_main(void)
{
    config();
    xTaskCreate(lcd, "LCDmessages", configMINIMAL_STACK_SIZE * 3, NULL, 3, NULL);
    xTaskCreate(buttonHandler, "ButtonHandler", configMINIMAL_STACK_SIZE * 3, NULL, 4, NULL);

    char bda_str[18] = {0};
    /* initialize NVS — it is used to store PHY calibration data */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    /*
     * This example only uses the functions of Classical Bluetooth.
     * So release the controller memory for Bluetooth Low Energy.
     */
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((err = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s initialize controller failed: %s", __func__, esp_err_to_name(err));
        return;
    }
    if ((err = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(err));
        return;
    }

    esp_bluedroid_config_t bluedroid_cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
#if (CONFIG_EXAMPLE_A2DP_SINK_SSP_ENABLED == false)
    bluedroid_cfg.ssp_en = false;
#endif
    if ((err = esp_bluedroid_init_with_cfg(&bluedroid_cfg)) != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s initialize bluedroid failed: %s", __func__, esp_err_to_name(err));
        return;
    }

    if ((err = esp_bluedroid_enable()) != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s enable bluedroid failed: %s", __func__, esp_err_to_name(err));
        return;
    }

#if (CONFIG_EXAMPLE_A2DP_SINK_SSP_ENABLED == true)
    /* set default parameters for Secure Simple Pairing */
    esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
    esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_IO;
    esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));
#endif

    /* set default parameters for Legacy Pairing (use fixed pin code 1234) */
    esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_FIXED;
    esp_bt_pin_code_t pin_code;
    pin_code[0] = '1';
    pin_code[1] = '2';
    pin_code[2] = '3';
    pin_code[3] = '4';
    esp_bt_gap_set_pin(pin_type, 4, pin_code);

    ESP_LOGI(BT_AV_TAG, "Own address:[%s]", bda2str((uint8_t *)esp_bt_dev_get_address(), bda_str, sizeof(bda_str)));
    bt_app_task_start_up();
    /* bluetooth device name, connection mode and profile set up */
    bt_app_work_dispatch(bt_av_hdl_stack_evt, BT_APP_EVT_STACK_UP, NULL, 0, NULL);
}
