#include "ch.h"
#include "hal.h"
#include "platform.h"
#include "fusb.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_i2c.h"

#include "axoloti_board.h"
#define I2C1_TIMEOUT_MAX 0xFFFF

void platform_set_vbus_lvl_enable(FSC_U8 port,
                                  VBUS_LVL level,
                                  FSC_BOOL enable,
                                  FSC_BOOL disableOthers) {
    if(enable) {
        palWritePad(GPIOH, 14, PAL_LOW);
    } else {
        palWritePad(GPIOH, 14, PAL_HIGH);
    }
}

FSC_BOOL platform_get_vbus_lvl_enable(FSC_U8 port, VBUS_LVL level) {
    return palReadPad(GPIOH, 14) == PAL_LOW;
}

void platform_set_vbus_discharge(FSC_U8 port, FSC_BOOL enable) {
}

FSC_BOOL platform_get_device_irq_state(FSC_U8 port) {
    volatile bool foo = palReadPad(GPIOH, 15) == PAL_LOW;
    return foo;
}

FSC_BOOL platform_i2c_write(FSC_U8 SlaveAddress,
                            FSC_U8 RegAddrLength,
                            FSC_U8 DataLength,
                            FSC_U8 PacketSize,
                            FSC_U8 IncSize,
                            FSC_U32 RegisterAddress,
                            FSC_U8* Data)
{
    HAL_StatusTypeDef result = HAL_OK;

    result = HAL_I2C_Mem_Write(&fusb_i2c_handle, SlaveAddress, RegisterAddress,
                               RegAddrLength, Data, DataLength,
                               I2C1_TIMEOUT_MAX);

    return ((result == HAL_OK) ? true : false);
}

FSC_BOOL platform_i2c_read( FSC_U8 SlaveAddress,
                            FSC_U8 RegAddrLength,
                            FSC_U8 DataLength,
                            FSC_U8 PacketSize,
                            FSC_U8 IncSize,
                            FSC_U32 RegisterAddress,
                            FSC_U8* Data)
{


    HAL_StatusTypeDef result = HAL_OK;

    result = HAL_I2C_Mem_Read(&fusb_i2c_handle, SlaveAddress, RegisterAddress,
                              RegAddrLength, Data, DataLength,
                              I2C1_TIMEOUT_MAX);

    return ((result == HAL_OK) ? true : false);
}

/*****************************************************************************
* Function:        platform_delay_10us
* Input:           delayCount - Number of 10us delays to wait
* Return:          None
* Description:     Perform a software delay in intervals of 10us.
******************************************************************************/
void platform_delay_10us(FSC_U32 delayCount) {
    chThdSleepMicroseconds(10 * delayCount);
}

void platform_set_pps_voltage(FSC_U8 port, FSC_U32 mv) {
}

FSC_U16 platform_get_pps_voltage(FSC_U8 port) {
    return 0;
}

void platform_set_pps_current(FSC_U8 port, FSC_U32 ma) {
}

FSC_U16 platform_get_pps_current(FSC_U8 port) {
    return 0;
}

FSC_U32 platform_get_system_time(void) {
    return chVTGetSystemTime();
}
