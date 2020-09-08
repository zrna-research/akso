/**
 ******************************************************************************
 * @file        main.c
 * @author      ON Semiconductor USB-PD Firmware Team
 * Software License Agreement:
 *
 * The software supplied herewith by ON Semiconductor (the Company)
 * is supplied to you, the Company's customer, for exclusive use with its
 * USB Type C / USB PD products.  The software is owned by the Company and/or
 * its supplier, and is protected under applicable copyright laws.
 * All rights are reserved. Any use in violation of the foregoing restrictions
 * may subject the user to criminal sanctions under applicable laws, as well
 * as to civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN AS IS CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *******************************************************************************
 */

#include "core.h"
#include "Port.h"
#include <stdio.h>

static DevicePolicyPtr_t dpm;           /* Device policy manager pointer */
static Port_t ports[NUM_PORTS];         /* Initalize port objects for each port */

static void InitializeSystem(void)
{
    /* Initialize the system */
    /* Timers, GPIO, Type-C/PD configuration, etc... */

    core_initialize(&ports[0], 0x11);
    core_enable_typec(&ports[0], TRUE);
}

/**
 * @brief Program entry point and container of main loop
 * @param None
 * @return None
 */
int main(void)
{
    ports[0].PortID = 0;

    InitializeSystem();
    DPM_Init(&dpm);
    /* Tell the dpm which port to manage */
    DPM_AddPort(dpm, &ports[0]);
    /* Tell port the dpm it must use */
    ports[0].dpm = dpm;

    printf("Running core machine\n");

    while (1)
    {
        /* Run the Type-C state machine */
        core_state_machine(&ports[0]);
    }
}


