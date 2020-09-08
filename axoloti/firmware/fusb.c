#include "fusb.h"
#include "ch.h"
#include "hal.h"

#include "core.h"
#include "Port.h"

#if BOARD_AKSO

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_i2c.h"

#include "axoloti_board.h"

static uint8_t i2ctxbuf[8] = {0, 0, 0, 0, 0, 0, 0, 0};

static DevicePolicyPtr_t dpm;           /* Device policy manager pointer */
static Port_t ports[NUM_PORTS];         /* Initalize port objects for each port */

#define FUSB_I2C_ADDR (0b1000100 >> 1)

I2C_HandleTypeDef fusb_i2c_handle;

static void fusb_i2c_init(void) {
    if (HAL_I2C_GetState(&fusb_i2c_handle) == HAL_I2C_STATE_RESET) {
        fusb_i2c_handle.Init.Timing = 0x307075B1;

        fusb_i2c_handle.Init.OwnAddress1 = 0x33;
        fusb_i2c_handle.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
        fusb_i2c_handle.Instance = I2C4;

        fusb_i2c_handle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
        fusb_i2c_handle.Init.OwnAddress2 = 0;
        fusb_i2c_handle.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
        fusb_i2c_handle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
        fusb_i2c_handle.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

        /* Init the I2C */
        palSetPadMode(
            GPIOH,
            11,
            PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN
            | PAL_MODE_INPUT_PULLUP);
        palSetPadMode(
            GPIOH,
            12,
            PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN
            | PAL_MODE_INPUT_PULLUP);

        rccEnableI2C4(FALSE);
        nvicEnableVector(I2C4_EV_IRQn, STM32_I2C_I2C4_IRQ_PRIORITY);
        nvicEnableVector(I2C4_ER_IRQn, STM32_I2C_I2C4_IRQ_PRIORITY);

        HAL_I2C_Init(&fusb_i2c_handle);
    }
}


OSAL_IRQ_HANDLER(STM32_I2C4_EVENT_HANDLER) {
    OSAL_IRQ_PROLOGUE();
    HAL_I2C_EV_IRQHandler(&fusb_i2c_handle);
    OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(STM32_I2C4_ERROR_HANDLER) {
    OSAL_IRQ_PROLOGUE();
    HAL_I2C_ER_IRQHandler(&fusb_i2c_handle);
    OSAL_IRQ_EPILOGUE();
}

static void fusb_gpio_init(void) {
    palSetPadMode(
        GPIOH,
        15,
        PAL_MODE_INPUT);
}

bool fusb_WriteRegister_on(uint16_t RegisterAddr, uint8_t RegisterValue) {
    i2ctxbuf[0] = RegisterAddr;
    i2ctxbuf[1] = RegisterValue;
    HAL_StatusTypeDef r = HAL_I2C_Master_Transmit(&fusb_i2c_handle, FUSB_I2C_ADDR << 1, i2ctxbuf, 2, 0xffffffff);
    return r == HAL_OK;
}

bool fusb_ReadRegister_on(uint16_t RegisterAddr, uint8_t* data) {
    i2ctxbuf[0] = RegisterAddr;
    HAL_StatusTypeDef r = HAL_I2C_Master_Transmit(&fusb_i2c_handle, FUSB_I2C_ADDR << 1, i2ctxbuf, 1, 0xffffffff);
    if (r != HAL_OK) {
        return false;
    }


    r = HAL_I2C_Master_Receive(&fusb_i2c_handle, (FUSB_I2C_ADDR << 1 ) + 1, data, 1, 0xffffffff);
    return r == HAL_OK;
}

static thread_t * thd_fusb_monitor;

static void InitializeSystem(void)
{
    core_initialize(&ports[0], 0b100010 << 1);
//    core_enable_typec(&ports[0], TRUE);
//    core_enable_pd(&ports[0], false);
}

static THD_WORKING_AREA(wa_fusb_monitor, 256);
static THD_FUNCTION(fusb_monitor, arg) {
    (void)arg;
    chRegSetThreadName("fusb_monitor");

    chThdSleepMilliseconds(1000);

    ports[0].PortID = 0;

    InitializeSystem();
    DPM_Init(&dpm);
    /* Tell the dpm which port to manage */
    DPM_AddPort(dpm, &ports[0]);
    /* Tell port the dpm it must use */
    ports[0].dpm = dpm;

    while (true) {
        /* Run the Type-C state machine */
        core_state_machine(&ports[0]);
        chThdSleepMilliseconds(100);
    }

#if 0
    fusb_WriteRegister(0x0c, 0b1); // soft reset fusb registers

    fusb_WriteRegister(MASK_REG, 0b01011110);
    fusb_WriteRegister(MASKA_REG, 0xbf);
    fusb_WriteRegister(MASKB_REG, 0x01);
    fusb_WriteRegister(0x0b, 0x7);

    // global interrupt unmask
    fusb_WriteRegister(CONTROL0_REG, 0b00000100);

    fusb_ReadRegister(INTERRUPT_REG);
    fusb_ReadRegister(INTERRUPTA_REG);
    fusb_ReadRegister(INTERRUPTB_REG);

    fusb_WriteRegister(CONTROL2_REG, 0b00000111);

    while(true) {
        while(palReadPad(GPIOH, 15) == PAL_HIGH) {
            chThdSleepMilliseconds(100);
        }

        palWritePad(GPIOH, 14, PAL_LOW);
    }
#endif
}

void fusb_init(void) {
    fusb_i2c_init();
    fusb_gpio_init();
    thd_fusb_monitor = chThdCreateStatic(wa_fusb_monitor, sizeof(wa_fusb_monitor), NORMALPRIO, fusb_monitor, NULL);
}
#endif
