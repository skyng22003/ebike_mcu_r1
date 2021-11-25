
#include"CAN.h"




// define your CAN messages here, OR you can define them locally...
// standard 11-bit frame ID = 0
// extended 29-bit frame ID = 1
// format:   can_message_t (name of your message) = {std/ext frame, message ID, message DLC, {data bytes here}};

can_message_t myMessageToSend = { 0, 0x123, 8, {0x01, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x99} };

uint32_t previousMillis;
const uint32_t interval = 1000;

//=================================================



void CANinit()
{
    can_general_config_t general_config = {
           .mode = CAN_MODE_NORMAL,
           .tx_io = (gpio_num_t)GPIO_NUM_5,
           .rx_io = (gpio_num_t)GPIO_NUM_4,
           .clkout_io = (gpio_num_t)CAN_IO_UNUSED,
           .bus_off_io = (gpio_num_t)CAN_IO_UNUSED,
           .tx_queue_len = 100,
           .rx_queue_len = 65,
           .alerts_enabled = CAN_ALERT_NONE,
           .clkout_divider = 0 };
    can_timing_config_t timing_config = CAN_TIMING_CONFIG_500KBITS();
    can_filter_config_t filter_config = CAN_FILTER_CONFIG_ACCEPT_ALL();
    esp_err_t error;

    error = can_driver_install(&general_config, &timing_config, &filter_config);
    if (error == ESP_OK)
    {
        Serial.println("CAN Driver installation success...");
    }
    else
    {
        Serial.println("CAN Driver installation fail...");
        return;
    }

    // start CAN driver
    error = can_start();
    if (error == ESP_OK)
    {
        Serial.println("CAN Driver start success...");
    }
    else
    {
        Serial.println("CAN Driver start FAILED...");
        return;
    }
}

//=======================================================================

void CANloop()
{
    canTx_buf[8];
    can_message_t rx_frame;
    if (can_receive(&rx_frame, pdMS_TO_TICKS(1000)) == ESP_OK)
    {
        Serial.println("CAN received message");
        
        //do whatever you need with your received CAN messages here
        // follow the can_message_t struct to learn how to decode/process the received frame.
    }
/* 
//Battery State   
canTx_buf[0]= rt_vars.ui16_adc_battery_voltage;
canTx_buf[1]= rt_vars.ui16_adc_battery_voltage;
canTx_buf[2]=rt_vars.ui8_battery_current_x5;


canTx_buf[3]= rt_vars.ui16_wheel_speed_x10;
canTx_buf[4]= rt_vars.ui16_wheel_speed_x10;
canTx_buf[5]= rt_vars.ui8_pedal_cadence
ui8_pedal_cadence
ui8_pedal_weight
ui8_pedal_weight_with_offset
ui16_adc_pedal_torque_sensor

//
canTx_buf[2] =ui_vars.ui8_assist_level
ui8_walk_assist
current_gear


//Motor State
g_motor_init_state
ui8_foc_angle
ui8_motor_temperature
ui8_duty_cycle
rt_vars.ui16_motor_speed_erps
canTx_buf[6]=rt_vars.ui8_error_states 
canTx_buf[7]=rt_vars.ui8_motor_current_x5

    
*/
        can_transmit(&myMessageToSend, pdMS_TO_TICKS(1000));
        Serial.println("Sent CAN Msg");
}

void CAN_Rxpacket(){








}


void CAN_Txpacket(){








}