/*******************************************************************************
 * @file     TypeC.c
 * @author   USB PD Firmware Team
 *
 * Copyright 2018 ON Semiconductor. All rights reserved.
 *
 * This software and/or documentation is licensed by ON Semiconductor under
 * limited terms and conditions. The terms and conditions pertaining to the
 * software and/or documentation are available at
 * http://www.onsemi.com/site/pdf/ONSEMI_T&C.pdf
 * ("ON Semiconductor Standard Terms and Conditions of Sale, Section 8 Software").
 *
 * DO NOT USE THIS SOFTWARE AND/OR DOCUMENTATION UNLESS YOU HAVE CAREFULLY
 * READ AND YOU AGREE TO THE LIMITED TERMS AND CONDITIONS. BY USING THIS
 * SOFTWARE AND/OR DOCUMENTATION, YOU AGREE TO THE LIMITED TERMS AND CONDITIONS.
 ******************************************************************************/
#include "vendor_info.h"
#include "fusb30X.h"
#include "TypeC_Types.h"
#include "TypeC.h"
#include "PDPolicy.h"
#include "PDProtocol.h"

#ifdef PLATFORM_PIC32_ADC
#include "AnalogInput.h"
#endif /* PLATFORM_PIC32_ADC */

#ifdef FSC_DEBUG
#include "Log.h"
#endif /* FSC_DEBUG */

void StateMachineTypeC(Port_t *port)
{
    do
    {

    if (!port->SMEnabled)
    {
        return;
    }

    port->TCIdle = false;

    if (platform_get_device_irq_state(port->PortID))
    {
        /* Read the interrupta, interruptb, status0, status1 and
         * interrupt registers.
         */
        DeviceRead(port->I2cAddr, regInterrupta, 5,
                   &port->Registers.Status.byte[2]);
    }

    if (port->USBPDActive)
    {
        port->PEIdle = false;

        /* Protocol operations */
        USBPDProtocol(port);

        /* Policy Engine operations */
        USBPDPolicyEngine(port);

#ifdef FSC_HAVE_EXT_MSG
        /* Extended messaging may require additional chunk handling
         * before idling.
         */
        if (port->ExtTxOrRx != NoXfer)
        {
            /* Don't allow system to idle */
            port->PEIdle = false;
        }
#endif /* FSC_HAVE_EXT_MSG */
    }

    switch (port->ConnState)
    {
    case Disabled:
        StateMachineDisabled(port);
        break;
    case ErrorRecovery:
        StateMachineErrorRecovery(port);
        break;
    case Unattached:
        StateMachineUnattached(port);
        break;
#ifdef FSC_HAVE_SNK
    case AttachWaitSink:
        StateMachineAttachWaitSink(port);
        break;
    case AttachedSink:
        StateMachineAttachedSink(port);
        break;
#ifdef FSC_HAVE_DRP
    case TryWaitSink:
        StateMachineTryWaitSink(port);
        break;
#endif /* FSC_HAVE_DRP */
#endif /* FSC_HAVE_SNK */
#if (defined(FSC_HAVE_DRP) || \
     (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACCMODE)))
    case TrySink:
        StateMachineTrySink(port);
        break;
#endif /* FSC_HAVE_DRP || (FSC_HAVE_SNK && FSC_HAVE_ACCMODE)) */
#ifdef FSC_HAVE_SRC
    case AttachWaitSource:
        StateMachineAttachWaitSource(port);
        break;
    case AttachedSource:
        StateMachineAttachedSource(port);
        break;
#ifdef FSC_HAVE_DRP
    case TryWaitSource:
        StateMachineTryWaitSource(port);
        break;
    case TrySource:
        StateMachineTrySource(port);
        break;
    case UnattachedSource:
        StateMachineUnattachedSource(port);
        break;
#endif /* FSC_HAVE_DRP */
    case DebugAccessorySource:
        StateMachineDebugAccessorySource(port);
        break;
#endif /* FSC_HAVE_SRC */
#ifdef FSC_HAVE_ACCMODE
    case AudioAccessory:
        StateMachineAudioAccessory(port);
        break;
#ifdef FSC_HAVE_SNK
    case DebugAccessorySink:
        StateMachineDebugAccessorySink(port);
        break;
#endif /* FSC_HAVE_SNK */
#endif /* FSC_HAVE_ACCMODE */
#if (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACCMODE))
    case AttachWaitAccessory:
        StateMachineAttachWaitAccessory(port);
        break;
    case UnsupportedAccessory:
        StateMachineUnsupportedAccessory(port);
        break;
    case PoweredAccessory:
        StateMachinePoweredAccessory(port);
        break;
#endif /* FSC_HAVE_SNK && FSC_HAVE_ACCMODE */
    case IllegalCable:
        StateMachineIllegalCable(port);
        break;
    default:
        SetStateUnattached(port);
        break;
    }

    /* Clear the interrupt registers after processing the state machines */
    port->Registers.Status.Interrupt1 = 0;
    port->Registers.Status.InterruptAdv = 0;

    } while (port->TCIdle == false || port->PEIdle == false);
}

void StateMachineDisabled(Port_t *port)
{
    /* Do nothing until directed to go to some other state... */
}

void StateMachineErrorRecovery(Port_t *port)
{
    if (TimerExpired(&port->StateTimer))
    {
        SetStateUnattached(port);
    }
}

void StateMachineUnattached(Port_t *port)
{
    port->TCIdle = true;

    if (TimerExpired(&port->LoopCountTimer))
    {
        /* Detached for ~100ms - safe to clear the loop counter */
        TimerDisable(&port->LoopCountTimer);
        port->loopCounter = 0;
    }

    if (port->Registers.Status.I_TOGDONE)
    {
        TimerDisable(&port->LoopCountTimer);

        DeviceRead(port->I2cAddr, regStatus1a, 1,
                   &port->Registers.Status.byte[1]);

        switch (port->Registers.Status.TOGSS)
        {
#ifdef FSC_HAVE_SNK
        case 0x5: /* Rp detected on CC1 */
            port->CCPin = CC1;
            SetStateAttachWaitSink(port);
            break;
        case 0x6: /* Rp detected on CC2 */
            port->CCPin = CC2;
            SetStateAttachWaitSink(port);
            break;
#endif /* FSC_HAVE_SNK */
#if (defined(FSC_HAVE_SRC) || \
     (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACCMODE)))
        case 0x1: /* Rd detected on CC1 */
            port->CCPin = CCNone; /* Wait to re-check orientation */
#if (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACCMODE))
            if ((port->PortConfig.PortType == USBTypeC_Sink) &&
                ((port->PortConfig.audioAccSupport) ||
                 (port->PortConfig.poweredAccSupport)))
            {
                SetStateAttachWaitAccessory(port);
            }
#endif /* FSC_HAVE_SNK && FSC_HAVE_ACCMODE */
#if defined(FSC_HAVE_SRC) && defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACCMODE)
            else
#endif /* FSC_HAVE_SRC && FSC_HAVE_SNK && FSC_HAVE_ACCMODE */
#ifdef FSC_HAVE_SRC
            {
                SetStateAttachWaitSource(port);
            }
#endif /* FSC_HAVE_SRC */
            break;
        case 0x2: /* Rd detected on CC2 */
            port->CCPin = CCNone; /* Wait to re-check orientation */
#if (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACCMODE))
            if ((port->PortConfig.PortType == USBTypeC_Sink) &&
                ((port->PortConfig.audioAccSupport) ||
                 (port->PortConfig.poweredAccSupport)))
            {
                SetStateAttachWaitAccessory(port);
            }
#endif /* FSC_HAVE_SNK && FSC_HAVE_ACCMODE */
#if defined(FSC_HAVE_SRC) && defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACCMODE)
            else
#endif /* FSC_HAVE_SRC && FSC_HAVE_SNK && FSC_HAVE_ACCMODE */
#ifdef FSC_HAVE_SRC
            {
                SetStateAttachWaitSource(port);
            }
#endif /* FSC_HAVE_SRC */
            break;
        case 0x7: /* Ra detected on both CC1 and CC2 */
            port->CCPin = CCNone;
#if (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACCMODE))
            if ((port->PortConfig.PortType == USBTypeC_Sink) &&
                ((port->PortConfig.audioAccSupport) ||
                 (port->PortConfig.poweredAccSupport)))
            {
                    SetStateAttachWaitAccessory(port);
            }
#endif /* defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACCMODE) */
#if defined(FSC_HAVE_SRC) && defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACCMODE)
            else
#endif /* FSC_HAVE_SRC && FSC_HAVE_SNK && FSC_HAVE_ACCMODE */
#if defined(FSC_HAVE_SRC)
            if (port->PortConfig.PortType != USBTypeC_Sink)
            {
                    SetStateAttachWaitSource(port);
            }
#endif /* FSC_HAVE_SRC */
            break;
#endif /* FSC_HAVE_SRC || (FSC_HAVE_SNK && FSC_HAVE_ACCMODE) */
        default:
            /* Shouldn't get here, but just in case reset everything... */
            port->Registers.Control.TOGGLE = 0;
            DeviceWrite(port->I2cAddr, regControl2, 1,
                        &port->Registers.Control.byte[2]);

            platform_delay_10us(1);

            /* Re-enable the toggle state machine... (allows us to get
             * another I_TOGDONE interrupt)
             */
            port->Registers.Control.TOGGLE = 1;
            DeviceWrite(port->I2cAddr, regControl2, 1,
                        &port->Registers.Control.byte[2]);
            break;
        }
    }
}

#ifdef FSC_HAVE_SNK
void StateMachineAttachWaitSink(Port_t *port)
{
    port->TCIdle = true;

    debounceCC(port);

    if(port->Registers.Status.ACTIVITY == 1)
    {
        /* PD Traffic will prevent correct use of BC_LVL during debouncing */
        return;
    }

    /* Look for an open termination for > tPDDebounce. */
    if(port->CCTermPDDebounce == CCTypeOpen)
    {
        /* PDDebounce Expired means the selected pin is open. Check other CC. */
        ToggleMeasure(port);
        port->CCPin = (port->CCPin == CC1) ? CC2 : CC1;

        port->CCTerm = DecodeCCTerminationSink(port);

        if(port->CCTerm == CCTypeOpen)
        {
            /* Other pin is open as well - detach. */
#ifdef FSC_HAVE_DRP
            if (port->PortConfig.PortType == USBTypeC_DRP)
            {
                SetStateUnattachedSource(port);
                return;
            }
            else
#endif // FSC_HAVE_DRP
            {
                SetStateUnattached(port);
                return;
            }
        }
        else
        {
            /* Other pin is attached.  Continue debouncing other pin. */
            TimerDisable(&port->PDDebounceTimer);
            port->CCTermPDDebounce = CCTypeUndefined;
        }
    }

    /* CC Debounce the selected CC pin. */
    if (port->CCTermCCDebounce == CCTypeRdUSB)
    {
        updateVCONNSink(port);

        if (isVSafe5V(port))
        {
            if((port->VCONNTerm >= CCTypeRdUSB) &&
               (port->VCONNTerm < CCTypeUndefined))
            {
                /* Rp-Rp */
                if(Type_C_Is_Debug_Target_SNK)
                {
                    SetStateDebugAccessorySink(port);
                }
            }
            else if(port->VCONNTerm == CCTypeOpen)
            {
                /* Rp-Open */
#ifdef FSC_HAVE_DRP
                if ((port->PortConfig.PortType == USBTypeC_DRP) &&
                     port->PortConfig.SrcPreferred)
                {
                    SetStateTrySource(port);
                }
                else
#endif // FSC_HAVE_DRP
                {
                    SetStateAttachedSink(port);
                }
            }
        }
        else
        {
            TimerStart(&port->VBusPollTimer, tVBusPollShort);
        }
    }
}
#endif /* FSC_HAVE_SNK */

#ifdef FSC_HAVE_SRC
void StateMachineAttachWaitSource(Port_t *port)
{
    FSC_BOOL ccPinIsRa = false;
    port->TCIdle = true;

    /* Update source current - can only toggle with Default and may be using
     * 3A advertisement to prevent non-compliant cable looping.
     */
    if (port->Registers.Control.HOST_CUR != port->SourceCurrent &&
        TimerExpired(&port->StateTimer))
    {
        TimerDisable(&port->StateTimer);
        updateSourceCurrent(port);
    }

    updateVCONNSource(port);
    ccPinIsRa = IsCCPinRa(port);

    /* Checking pins can cause extra COMP interrupts. */
    platform_delay_10us(12);
    DeviceRead(port->I2cAddr, regInterrupt, 1,
               &port->Registers.Status.Interrupt1);

    debounceCC(port);

    if (port->CCTerm == CCTypeOpen)
    {
        if (port->VCONNTerm == CCTypeOpen || port->VCONNTerm == CCTypeRa)
        {
            SetStateUnattached(port);
            return;
        }
        else
        {
            /* CC pin may have switched (compliance test) - swap here */
            port->CCPin = (port->CCPin == CC1) ? CC2 : CC1;
            setStateSource(port, false);
            return;
        }
    }

    if (ccPinIsRa)
    {
        if (port->VCONNTerm >= CCTypeRdUSB && port->VCONNTerm < CCTypeUndefined)
        {
            /* The toggle state machine may have stopped on an Ra - swap here */
            port->CCPin = (port->CCPin == CC1) ? CC2 : CC1;
            setStateSource(port, false);
            return;
        }
#ifdef FSC_HAVE_DRP
        else if (port->VCONNTerm == CCTypeOpen &&
                 port->PortConfig.PortType == USBTypeC_DRP)
        {
            /* Dangling Ra cable - could still attach to Snk or Src Device.
             * Disconnect and keep looking for full connection.
             * Reset loopCounter to prevent landing in IllegalCable state.
             */
            port->loopCounter = 1;
            SetStateUnattached(port);
            return;
        }
#endif /* FSC_HAVE_DRP */
    }

    /* Wait on CC Debounce for connection */
    if (port->CCTermCCDebounce != CCTypeUndefined)
    {
        if (ccPinIsRa)
        {
#ifdef FSC_HAVE_ACCMODE
            if (port->PortConfig.audioAccSupport &&
                (port->VCONNTerm == CCTypeRa))
            {
                SetStateAudioAccessory(port);
            }
#endif /* FSC_HAVE_ACCMODE */
        }

        if ((port->CCTermCCDebounce >= CCTypeRdUSB) &&
                 (port->CCTermCCDebounce < CCTypeUndefined) &&
                 (port->VCONNTerm >= CCTypeRdUSB) &&
                 (port->VCONNTerm < CCTypeUndefined))
        {
            /* Both pins Rd and Debug (DTS) mode supported */
            if (VbusVSafe0V(port))
            {
                if(Type_C_Is_Debug_Target_SRC)
                {
                    SetStateDebugAccessorySource(port);
                }
            }
            else
            {
                TimerStart(&port->VBusPollTimer, tVBusPollShort);
                TimerDisable(&port->StateTimer);
            }
        }
        else if ((port->CCTermCCDebounce >= CCTypeRdUSB) &&
                 (port->CCTermCCDebounce < CCTypeUndefined) &&
                 ((port->VCONNTerm == CCTypeOpen) ||
                  (port->VCONNTerm == CCTypeRa)))
        {
            /* One pin Rd */
            if (VbusVSafe0V(port))
            {
#ifdef FSC_HAVE_DRP
                if (port->PortConfig.SnkPreferred)
                {
                    SetStateTrySink(port);
                }
                else
#endif /* FSC_HAVE_DRP */
                {
                    SetStateAttachedSource(port);
                }
            }
            else
            {
                TimerStart(&port->VBusPollTimer, tVBusPollShort);
            }
        }
        else
        {
            /* In the current configuration, we may be here with Ra-Open
             * (cable with nothing attached) so periodically poll for attach
             * or open or VBus present.
             */
            if (TimerDisabled(&port->StateTimer) ||
                TimerExpired(&port->StateTimer))
            {
                TimerStart(&port->StateTimer, tAttachWaitPoll);
            }
        }
    }
}
#endif /* FSC_HAVE_SRC */

#ifdef FSC_HAVE_SNK
void StateMachineAttachedSink(Port_t *port)
{
    port->TCIdle = true;

    /* Monitor for VBus drop to detach
     * Round up detach threshold to help check slow discharge or noise on VBus
     */
    if (port->Registers.Status.I_COMP_CHNG == 1)
    {
        if (!port->IsPRSwap && !port->IsHardReset &&
            !isVBUSOverVoltage(port, port->DetachThreshold + MDAC_MV_LSB))
        {
            SetStateUnattached(port);
            return;
        }
    }

    if (!port->IsPRSwap)
    {
        debounceCC(port);
    }

    /* If using PD, sink can monitor CC as well as VBUS to allow detach
     * during a hard reset
     */
    if (port->USBPDActive && !port->IsPRSwap &&
        port->CCTermPDDebounce == CCTypeOpen)
    {
        SetStateUnattached(port);
        return;
    }

    /* Update the advertised current */
    if (port->CCTermPDDebounce != CCTypeUndefined)
    {
        UpdateSinkCurrent(port, port->CCTermPDDebounce);
    }
}
#endif /* FSC_HAVE_SNK */

#ifdef FSC_HAVE_SRC
void StateMachineAttachedSource(Port_t *port)
{
    port->TCIdle = true;

    switch (port->TypeCSubState)
    {
    case 0:
        if ((port->loopCounter != 0) ||
            (port->Registers.Status.I_COMP_CHNG == 1))
        {
            port->CCTerm = DecodeCCTerminationSource(port);
        }

        if ((port->CCTerm == CCTypeOpen) && (!port->IsPRSwap))
        {
            platform_set_vbus_lvl_enable(port->PortID, VBUS_LVL_ALL, false,
                                         false);
            platform_set_vbus_discharge(port->PortID, true);

            notify_observers(CC_NO_ORIENT, port->I2cAddr, 0);

            USBPDDisable(port, true);

            /* VConn off and Pulldowns while detatching */
#if 0
            port->Registers.Switches.byte[0] = 0x03;
            DeviceWrite(port->I2cAddr, regSwitches0, 1,
                        &port->Registers.Switches.byte[0]);
#endif

            port->TypeCSubState++;

            TimerStart(&port->StateTimer, tSafe0V);
        }
        else if ((port->loopCounter != 0) &&
                 (TimerExpired(&port->StateTimer) || port->PolicyHasContract))
        {
            /* Valid attach, so reset loop counter and go Idle */
            TimerDisable(&port->StateTimer);
            port->loopCounter = 0;
            port->Registers.Mask.M_COMP_CHNG = 0;
            DeviceWrite(port->I2cAddr, regMask, 1, &port->Registers.Mask.byte);
        }
        break;
    case 1:
#ifdef FSC_HAVE_DRP
        if ((port->PortConfig.PortType == USBTypeC_DRP) &&
             port->PortConfig.SrcPreferred)
        {
            SetStateTryWaitSink(port);

            /* Disable VBus discharge after 10ms in TW.Snk */
            TimerStart(&port->StateTimer, tVBusPollShort);

            return;
        }
#endif /* FSC_HAVE_DRP */

        if (VbusVSafe0V(port) || TimerExpired(&port->StateTimer))
        {
            platform_set_vbus_discharge(port->PortID, false);
            SetStateUnattached(port);
        }
        else
        {
            TimerStart(&port->VBusPollTimer, tVBusPollShort);
        }
        break;
    default:
        SetStateErrorRecovery(port);
        break;
    }
}
#endif /* FSC_HAVE_SRC */

#ifdef FSC_HAVE_DRP
void StateMachineTryWaitSink(Port_t *port)
{
    port->TCIdle = true;

    debounceCC(port);

    if (port->CCTermPDDebounce == CCTypeOpen)
    {
        SetStateUnattached(port);
        return;
    }

    if (TimerExpired(&port->StateTimer))
    {
        TimerDisable(&port->StateTimer);

        /* Turn off discharge that was enabled before exiting AttachedSource */
        platform_set_vbus_discharge(port->PortID, false);
    }

    if (isVSafe5V(port))
    {
        if ((port->CCTermCCDebounce > CCTypeOpen) &&
            (port->CCTermCCDebounce < CCTypeUndefined))
        {
            platform_set_vbus_discharge(port->PortID, false);
            SetStateAttachedSink(port);
        }
    }
    else
    {
        TimerStart(&port->VBusPollTimer, tVBusPollShort);
    }
}
#endif /* FSC_HAVE_DRP */

#ifdef FSC_HAVE_DRP
void StateMachineTrySource(Port_t *port)
{
    port->TCIdle = true;

    debounceCC(port);

    if ((port->CCTermPDDebounce > CCTypeRa) &&
        (port->CCTermPDDebounce < CCTypeUndefined) &&
       ((port->VCONNTerm == CCTypeOpen) ||
        (port->VCONNTerm == CCTypeRa)))
    {
        /* If the CC pin is Rd for at least tPDDebounce */
        SetStateAttachedSource(port);
    }
    else if (TimerExpired(&port->StateTimer))
    {
        TimerDisable(&port->StateTimer);

        /* Move onto the TryWait.Snk state to not get stuck in here */
        SetStateTryWaitSink(port);
    }
}
#endif /* FSC_HAVE_DRP */

#ifdef FSC_HAVE_SRC
void StateMachineDebugAccessorySource(Port_t *port)
{
    port->TCIdle = true;

    updateVCONNSource(port);

    /* Checking pins can cause extra COMP interrupts. */
    platform_delay_10us(12);
    DeviceRead(port->I2cAddr, regInterrupt, 1,
               &port->Registers.Status.Interrupt1);

    debounceCC(port);

    if (port->CCTerm == CCTypeOpen)
    {
        SetStateUnattached(port);
        return;
    }
    else if ((port->CCTermPDDebounce >= CCTypeRdUSB)
            && (port->CCTermPDDebounce < CCTypeUndefined)
            && (port->VCONNTerm >= CCTypeRdUSB)
            && (port->VCONNTerm < CCTypeUndefined)
            && port->USBPDActive == false)
    {
        if (port->CCTermPDDebounce > port->VCONNTerm)
        {
            port->CCPin = port->Registers.Switches.MEAS_CC1 ? CC1 :
                         (port->Registers.Switches.MEAS_CC2 ? CC2 : CCNone);

            USBPDEnable(port, true, SOURCE);
        }
        else if (port->VCONNTerm > port->CCTermPDDebounce)
        {
            ToggleMeasure(port);
            port->CCPin = port->Registers.Switches.MEAS_CC1 ? CC1 :
                         (port->Registers.Switches.MEAS_CC2 ? CC2 : CCNone);

            USBPDEnable(port, true, SOURCE);
        }
    }
}
#endif /* FSC_HAVE_SRC */

#ifdef FSC_HAVE_ACCMODE
void StateMachineAudioAccessory(Port_t *port)
{
    port->TCIdle = true;

    debounceCC(port);

    /* Wait for a detach */
    if (port->CCTermCCDebounce == CCTypeOpen)
    {
        SetStateUnattached(port);
    }
}
#endif /* FSC_HAVE_ACCMODE */

#if (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACCMODE))
void StateMachineAttachWaitAccessory(Port_t *port)
{
    FSC_BOOL ccIsRa = false;

    port->TCIdle = true;

    updateVCONNSource(port);
    ccIsRa = IsCCPinRa(port);

    /* Checking pins can cause extra COMP interrupts. */
    platform_delay_10us(12);
    DeviceRead(port->I2cAddr, regInterrupt, 1,
               &port->Registers.Status.Interrupt1);

    debounceCC(port);

    if (ccIsRa &&
        (port->VCONNTerm >= CCTypeRdUSB && port->VCONNTerm < CCTypeUndefined))
    {
            /* The toggle state machine may have stopped on an Ra - swap here */
            port->CCPin = (port->CCPin == CC1) ? CC2 : CC1;
            setStateSource(port, false);
            port->TCIdle = false;
            return;
    }
    else if ((port->CCTerm == CCTypeOpen && port->VCONNTerm == CCTypeRa) ||
             (ccIsRa && port->VCONNTerm == CCTypeOpen))
    {
        /* Dangling Ra cable - could still attach to Snk or Src Device.
         * Disconnect and keep looking for full connection.
         * Reset loopCounter to prevent landing in IllegalCable state.
         */
        port->loopCounter = 1;
        SetStateUnattached(port);
        return;
    }
    else if (((port->CCTerm >= CCTypeRdUSB) &&
              (port->CCTerm < CCTypeUndefined) &&
              (port->VCONNTerm == CCTypeOpen)) ||
             ((port->VCONNTerm >= CCTypeRdUSB) &&
              (port->VCONNTerm < CCTypeUndefined) &&
              (port->CCTerm == CCTypeOpen)))
    {
        /* Rd-Open or Open-Rd shouldn't have generated an attach but
         * the 302 reports it anyway.
         */
        SetStateUnattached(port);
        return;
    }

    if (ccIsRa
            && (port->CCTermCCDebounce >= CCTypeRdUSB)
            && (port->CCTermCCDebounce < CCTypeUndefined))
    {
        port->CCTermCCDebounce = CCTypeRa;
    }

    if (port->PortConfig.audioAccSupport
            && port->CCTermCCDebounce == CCTypeRa
            && port->VCONNTerm == CCTypeRa
            && port->PortConfig.audioAccSupport)
    {
        SetStateAudioAccessory(port);
    }
    else if (port->CCTermCCDebounce == CCTypeOpen)
    {
        SetStateUnattached(port);
    }
    else if (port->PortConfig.poweredAccSupport
            && (port->CCTermCCDebounce >= CCTypeRdUSB)
            && (port->CCTermCCDebounce < CCTypeUndefined)
            && (port->VCONNTerm == CCTypeRa))
    {
        SetStatePoweredAccessory(port);
    }
    else if ((port->CCTermCCDebounce >= CCTypeRdUSB)
            && (port->CCTermCCDebounce < CCTypeUndefined)
            && (port->VCONNTerm == CCTypeOpen))
    {
        SetStateTrySink(port);
    }
}

void StateMachinePoweredAccessory(Port_t *port)
{
    port->TCIdle = true;

    debounceCC(port);

    if (port->CCTerm == CCTypeOpen)
    {
        SetStateUnattached(port);
    }
#ifdef FSC_HAVE_VDM
    else if (port->mode_entered == true)
    {
        /* Disable tAMETimeout if we enter a mode */
        TimerDisable(&port->StateTimer);

        port->loopCounter = 0;

        if(port->PolicyState == peSourceReady)
        {
            /* Unmask COMP register to detect detach */
            port->Registers.Mask.M_COMP_CHNG = 0;
            DeviceWrite(port->I2cAddr, regMask, 1, &port->Registers.Mask.byte);
        }
    }
#endif /* FSC_HAVE_VDM */
    else if (TimerExpired(&port->StateTimer))
    {
        /* If we have timed out and haven't entered an alternate mode... */
        if (port->PolicyHasContract)
        {
            SetStateUnsupportedAccessory(port);
        }
        else
        {
            SetStateTrySink(port);
        }
    }
}

void StateMachineUnsupportedAccessory(Port_t *port)
{
    port->TCIdle = true;

    debounceCC(port);

    if(port->CCTerm == CCTypeOpen)
    {
        SetStateUnattached(port);
    }
}
#endif /* (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACCMODE) */

#if (defined(FSC_HAVE_DRP) || \
     (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACCMODE)))
void StateMachineTrySink(Port_t *port)
{
    port->TCIdle = true;

    switch (port->TypeCSubState)
    {
    case 0:
        if (TimerExpired(&port->StateTimer))
        {
            TimerStart(&port->StateTimer, tDRPTryWait);
            port->TypeCSubState++;
        }
        break;
    case 1:
        debounceCC(port);

        if(port->Registers.Status.ACTIVITY == 1)
        {
            /* PD Traffic will prevent correct use of BC_LVL during debounce */
            return;
        }

        if (isVSafe5V(port))
        {
            if (port->CCTermPDDebounce == CCTypeRdUSB)
            {
                SetStateAttachedSink(port);
            }
        }

#ifdef FSC_HAVE_ACCMODE
        else if ((port->PortConfig.PortType == USBTypeC_Sink) &&
                 (TimerExpired(&port->StateTimer) ||
                  TimerDisabled(&port->StateTimer)) &&
                 (port->CCTermPDDebounce == CCTypeOpen))
        {
            SetStateUnsupportedAccessory(port);
        }
#endif /* FSC_HAVE_ACCMODE */

#ifdef FSC_HAVE_DRP
        else if ((port->PortConfig.PortType == USBTypeC_DRP) &&
                 (port->CCTermPDDebounce == CCTypeOpen))
        {
            SetStateTryWaitSource(port);
        }
#endif /* FSC_HAVE_DRP */
        else
        {
            TimerStart(&port->VBusPollTimer, tVBusPollShort);
        }
        break;
    default:
        break;
    }

}
#endif /* FSC_HAVE_DRP || (FSC_HAVE_SNK && FSC_HAVE_ACCMODE) */

#ifdef FSC_HAVE_DRP
void StateMachineTryWaitSource(Port_t *port)
{
    port->TCIdle = true;

    debounceCC(port);

    if (VbusVSafe0V(port))
    {
        if (((port->CCTermPDDebounce >= CCTypeRdUSB) &&
             (port->CCTermPDDebounce < CCTypeUndefined)) &&
            ((port->VCONNTerm == CCTypeRa) ||
             (port->VCONNTerm == CCTypeOpen)))
        {
            SetStateAttachedSource(port);
        }
    }

    /* After tDRPTry transition to Unattached.SNK if Rd is not
     * detected on exactly one CC pin
     */
    else if (TimerExpired(&port->StateTimer) ||
             TimerDisabled(&port->StateTimer))
    {
        if (!((port->CCTerm >= CCTypeRdUSB) &&
              (port->CCTerm < CCTypeUndefined)) &&
             ((port->VCONNTerm == CCTypeRa) ||
              (port->VCONNTerm == CCTypeOpen)))
        {
            SetStateUnattached(port);
        }
    }
    else
    {
        TimerStart(&port->VBusPollTimer, tVBusPollShort);
    }
}
#endif /* FSC_HAVE_DRP */

#ifdef FSC_HAVE_DRP
void StateMachineUnattachedSource(Port_t *port)
{
    port->TCIdle = true;

    debounceCC(port);
    updateVCONNSource(port);

    if ((port->CCTerm == CCTypeRa) && (port->VCONNTerm == CCTypeRa))
    {
        SetStateAttachWaitSource(port);
    }
    else if ((port->CCTerm >= CCTypeRdUSB)
            && (port->CCTerm < CCTypeUndefined)
            && ((port->VCONNTerm == CCTypeRa) ||
                (port->VCONNTerm == CCTypeOpen)))
    {
        port->CCPin = CC1;
        SetStateAttachWaitSource(port);
    }
    else if ((port->VCONNTerm >= CCTypeRdUSB)
            && (port->VCONNTerm < CCTypeUndefined)
            && ((port->CCTerm == CCTypeRa) ||
                (port->CCTerm == CCTypeOpen)))
    {
        port->CCPin = CC2;
        SetStateAttachWaitSource(port);
    }
    else if (TimerExpired(&port->StateTimer))
    {
        SetStateUnattached(port);
    }
}
#endif /* FSC_HAVE_DRP */

#ifdef FSC_HAVE_SNK
void StateMachineDebugAccessorySink(Port_t *port)
{
    port->TCIdle = true;

    debounceCC(port);
    updateVCONNSink(port);

    if (!isVBUSOverVoltage(port, port->DetachThreshold))
    {
        SetStateUnattached(port);
    }
    else if ((port->CCTermPDDebounce >= CCTypeRdUSB)
            && (port->CCTermPDDebounce < CCTypeUndefined)
            && (port->VCONNTerm >= CCTypeRdUSB)
            && (port->VCONNTerm < CCTypeUndefined)
            && port->USBPDActive == false)
    {
        if (port->CCTermPDDebounce > port->VCONNTerm)
        {
            port->CCPin = port->Registers.Switches.MEAS_CC1 ? CC1 :
                         (port->Registers.Switches.MEAS_CC2 ? CC2 : CCNone);

            USBPDEnable(port, true, SINK);
        }
        else if (port->VCONNTerm > port->CCTermPDDebounce)
        {
            ToggleMeasure(port);
            port->CCPin = port->Registers.Switches.MEAS_CC1 ? CC1 :
                         (port->Registers.Switches.MEAS_CC2 ? CC2 : CCNone);

            USBPDEnable(port, true, SINK);
        }
    }
}
#endif /* FSC_HAVE_SNK */

/* State Machine Configuration */
void SetStateDisabled(Port_t *port)
{
    port->TCIdle = true;

    SetTypeCState(port, Disabled);
    TimerDisable(&port->StateTimer);

    clearState(port);
}

void SetStateErrorRecovery(Port_t *port)
{
    port->TCIdle = true;

    SetTypeCState(port, ErrorRecovery);
    TimerStart(&port->StateTimer, tErrorRecovery);

    clearState(port);
}

void SetStateUnattached(Port_t *port)
{
    port->TCIdle = true;

    SetTypeCState(port, Unattached);

    clearState(port);

    port->Registers.Control.TOGGLE = 0;
    DeviceWrite(port->I2cAddr, regControl2, 1,&port->Registers.Control.byte[2]);

    port->Registers.Measure.MEAS_VBUS = 0;
    DeviceWrite(port->I2cAddr, regMeasure, 1, &port->Registers.Measure.byte);

    port->Registers.MaskAdv.M_TOGDONE = 0;
    DeviceWrite(port->I2cAddr, regMaska, 1, &port->Registers.MaskAdv.byte[0]);

    /* Host current must be set to default for Toggle Functionality */
    if (port->Registers.Control.HOST_CUR != 0x1)
    {
        port->Registers.Control.HOST_CUR = 0x1;
        DeviceWrite(port->I2cAddr, regControl0, 1,
                    &port->Registers.Control.byte[0]);
    }

    if (port->PortConfig.PortType == USBTypeC_DRP)
    {
        /* DRP - Configure Rp/Rd toggling */
        port->Registers.Control.MODE = 0x1;
    }
#ifdef FSC_HAVE_ACCMODE
    else if((port->PortConfig.PortType == USBTypeC_Sink) &&
            ((port->PortConfig.audioAccSupport) ||
             (port->PortConfig.poweredAccSupport)))
    {
        /* Sink + Acc - Configure Rp/Rd toggling */
        port->Registers.Control.MODE = 0x1;
    }
#endif /* FSC_HAVE_ACCMODE */
    else if (port->PortConfig.PortType == USBTypeC_Source)
    {
        /* Source - Look for Rd */
        port->Registers.Control.MODE = 0x3;
    }
    else
    {
        /* Sink - Look for Rp */
        port->Registers.Control.MODE = 0x2;
    }

    /* Delay before re-enabling toggle */
    platform_delay_10us(25);
    port->Registers.Control.TOGGLE = 1;
    DeviceWrite(port->I2cAddr, regControl0, 3,&port->Registers.Control.byte[0]);

    TimerDisable(&port->StateTimer);

    /* Wait to clear the connect loop counter till detached for > ~100ms. */
    TimerStart(&port->LoopCountTimer, tLoopReset);
}

#ifdef FSC_HAVE_SNK
void SetStateAttachWaitSink(Port_t *port)
{
    port->TCIdle = false;

    SetTypeCState(port, AttachWaitSink);

    /* Swap toggle state machine current if looping */
    if (port->loopCounter++ > MAX_CABLE_LOOP)
    {
        SetStateIllegalCable(port);
        return;
    }

    setStateSink(port);

    /* Disable the Toggle functionality */
    port->Registers.Control.TOGGLE = 0;
    DeviceWrite(port->I2cAddr, regControl2, 1,&port->Registers.Control.byte[2]);

    /* Enable interrupts */
    port->Registers.Mask.M_ACTIVITY = 0;
    DeviceWrite(port->I2cAddr, regMask, 1, &port->Registers.Mask.byte);

    /* Check for a possible C-to-A cable situation */
    if (isVSafe5V(port))
    {
        port->C2ACable = true;
    }

    /* Check for detach before continuing - FUSB302-210*/
    DeviceRead(port->I2cAddr, regStatus0, 1, &port->Registers.Status.byte[4]);
    if (port->Registers.Status.BC_LVL == 0)
    {
        SetStateUnattached(port);
    }
}

void SetStateDebugAccessorySink(Port_t *port)
{
    port->TCIdle = true;
    port->loopCounter = 0;

    SetTypeCState(port, DebugAccessorySink);
    setStateSink(port);

    port->Registers.Measure.MEAS_VBUS = 1;
    port->Registers.Measure.MDAC = (port->DetachThreshold / MDAC_MV_LSB) - 1;
    DeviceWrite(port->I2cAddr, regMeasure, 1, &port->Registers.Measure.byte);

    /* TODO RICK platform_double_56k_cable()? */

    TimerStart(&port->StateTimer, tOrientedDebug);
}
#endif /* FSC_HAVE_SNK */

#ifdef FSC_HAVE_SRC
void SetStateAttachWaitSource(Port_t *port)
{
    port->TCIdle = true;

    SetTypeCState(port, AttachWaitSource);

    /* Swap toggle state machine current if looping */
    if (port->loopCounter++ > MAX_CABLE_LOOP)
    {
        SetStateIllegalCable(port);
        return;
    }

    platform_set_vbus_lvl_enable(port->PortID, VBUS_LVL_ALL, false, false);

    /* Disabling the toggle bit here may cause brief pulldowns (they are the
     * default), so call setStateSource first to set pullups,
     * then disable the toggle bit, then re-run DetectCCPinSource to make sure
     * we have the correct pin selected.
     */
    setStateSource(port, false);

    /* To help prevent detection of a non-compliant cable, briefly set the
     * advertised current to 3A here.  It will be reset after tAttachWaitAdv
     */
    if (port->Registers.Control.HOST_CUR != 0x3)
    {
        port->Registers.Control.HOST_CUR = 0x3;
        DeviceWrite(port->I2cAddr, regControl0, 1,
                    &port->Registers.Control.byte[0]);
    }
    updateSourceMDACHigh(port);

    /* Disable toggle */
    port->Registers.Control.TOGGLE = 0;
    DeviceWrite(port->I2cAddr, regControl2, 1,&port->Registers.Control.byte[2]);

    /* Recheck for termination / orientation */
    DetectCCPinSource(port);
    setStateSource(port, false);

    /* Enable interrupts */
    port->Registers.Mask.M_COMP_CHNG = 0;
    DeviceWrite(port->I2cAddr, regMask, 1, &port->Registers.Mask.byte);

    /* After a delay, switch to the appropriate advertisement pullup */
    TimerStart(&port->StateTimer, tAttachWaitAdv);
}
#endif /* FSC_HAVE_SRC */

#ifdef FSC_HAVE_ACCMODE
void SetStateAttachWaitAccessory(Port_t *port)
{
    port->TCIdle = false;

    platform_set_vbus_lvl_enable(port->PortID, VBUS_LVL_ALL, false, false);
    SetTypeCState(port, AttachWaitAccessory);

    setStateSource(port, false);

    port->Registers.Control.TOGGLE = 0;
    DeviceWrite(port->I2cAddr, regControl2, 1,&port->Registers.Control.byte[2]);

    /* Enable interrupts */
    port->Registers.Mask.M_COMP_CHNG = 0;
    DeviceWrite(port->I2cAddr, regMask, 1, &port->Registers.Mask.byte);

    TimerDisable(&port->StateTimer);
}
#endif /* FSC_HAVE_ACCMODE */

#ifdef FSC_HAVE_SRC
void SetStateAttachedSource(Port_t *port)
{
    port->TCIdle = true;

    SetTypeCState(port, AttachedSource);

    setStateSource(port, true);

    /* Enable 5V VBus */
    platform_set_vbus_lvl_enable(port->PortID, VBUS_LVL_5V, true, true);

    notify_observers((port->CCPin == CC1) ? CC1_ORIENT : CC2_ORIENT,
                     port->I2cAddr, 0);

    USBPDEnable(port, true, SOURCE);

    /* Start delay to check for illegal cable looping */
    TimerStart(&port->StateTimer, tIllegalCable);
}
#endif /* FSC_HAVE_SRC */

#ifdef FSC_HAVE_SNK
void SetStateAttachedSink(Port_t *port)
{
    port->TCIdle = true;

    /* Default to 5V detach threshold */
    port->DetachThreshold = VBUS_MV_VSAFE5V_DISC;

    port->loopCounter = 0;

    SetTypeCState(port, AttachedSink);

    setStateSink(port);

    notify_observers((port->CCPin == CC1) ? CC1_ORIENT : CC2_ORIENT,
                     port->I2cAddr, 0);

    port->CCTerm = DecodeCCTerminationSink(port);
    UpdateSinkCurrent(port, port->CCTerm);

    USBPDEnable(port, true, SINK);
    TimerDisable(&port->StateTimer);
}
#endif /* FSC_HAVE_SNK */

#ifdef FSC_HAVE_DRP
void RoleSwapToAttachedSink(Port_t *port)
{
    SetTypeCState(port, AttachedSink);
    port->sourceOrSink = SINK;

    /* Watch VBUS for sink disconnect */
    port->Registers.Measure.MEAS_VBUS = 1;
    port->Registers.Measure.MDAC = (port->DetachThreshold / MDAC_MV_LSB) - 1;
    port->Registers.Mask.M_COMP_CHNG = 0;
    port->Registers.Mask.M_VBUSOK = 1;
    DeviceWrite(port->I2cAddr, regMeasure, 1, &port->Registers.Measure.byte);
    DeviceWrite(port->I2cAddr, regMask, 1, &port->Registers.Mask.byte);

    /* Swap Pullup/Pulldown */
    if (port->CCPin == CC1)
    {
        /* Maintain VCONN */
        port->Registers.Switches.PU_EN1 = 0;
        port->Registers.Switches.PDWN1 = 1;
    }
    else
    {
        /* Maintain VCONN */
        port->Registers.Switches.PU_EN2 = 0;
        port->Registers.Switches.PDWN2 = 1;
    }
//    DeviceWrite(port->I2cAddr, regSwitches0, 1,
//                &port->Registers.Switches.byte[0]);

    port->SinkCurrent = utccNone;
    resetDebounceVariables(port);
    TimerDisable(&port->StateTimer);
    TimerDisable(&port->PDDebounceTimer);
    TimerStart(&port->CCDebounceTimer, tCCDebounce);
}
#endif /* FSC_HAVE_DRP */

#ifdef FSC_HAVE_DRP
void RoleSwapToAttachedSource(Port_t *port)
{
    port->Registers.Measure.MEAS_VBUS = 0;
    DeviceWrite(port->I2cAddr, regMeasure, 1, &port->Registers.Measure.byte);

    platform_set_vbus_lvl_enable(port->PortID, VBUS_LVL_5V, true, true);
    SetTypeCState(port, AttachedSource);
    port->sourceOrSink = SOURCE;
    resetDebounceVariables(port);
    updateSourceMDACHigh(port);

    /* Swap Pullup/Pulldown */
    if (port->CCPin == CC1)
    {
        port->Registers.Switches.PU_EN1 = 1;
        port->Registers.Switches.PDWN1 = 0;
    }
    else
    {
        port->Registers.Switches.PU_EN2 = 1;
        port->Registers.Switches.PDWN2 = 0;
    }
    DeviceWrite(port->I2cAddr, regSwitches0, 1,
                &port->Registers.Switches.byte[0]);

    /* Enable comp change interrupt */
    port->Registers.Mask.M_COMP_CHNG = 0;
    DeviceWrite(port->I2cAddr, regMask, 1, &port->Registers.Mask.byte);

    port->SinkCurrent = utccNone;
    TimerDisable(&port->StateTimer);
    TimerDisable(&port->PDDebounceTimer);
    TimerStart(&port->CCDebounceTimer, tCCDebounce);
}
#endif /* FSC_HAVE_DRP */

#ifdef FSC_HAVE_DRP
void SetStateTryWaitSink(Port_t *port)
{
    port->TCIdle = false;

    /* Mask all */
    port->Registers.Mask.byte = 0xFF;
    DeviceWrite(port->I2cAddr, regMask, 1, &port->Registers.Mask.byte);
    port->Registers.MaskAdv.byte[0] = 0xFF;
    DeviceWrite(port->I2cAddr, regMaska, 1, &port->Registers.MaskAdv.byte[0]);
    port->Registers.MaskAdv.M_GCRCSENT = 1;
    DeviceWrite(port->I2cAddr, regMaskb, 1, &port->Registers.MaskAdv.byte[1]);

    USBPDDisable(port, true);

    SetTypeCState(port, TryWaitSink);

    setStateSink(port);

    TimerDisable(&port->StateTimer);
}
#endif /* FSC_HAVE_DRP */

#ifdef FSC_HAVE_DRP
void SetStateTrySource(Port_t *port)
{
    port->TCIdle = false;

    platform_set_vbus_lvl_enable(port->PortID, VBUS_LVL_ALL, false, false);
    SetTypeCState(port, TrySource);

    setStateSource(port, false);

    TimerStart(&port->StateTimer, tDRPTry);
}
#endif /* FSC_HAVE_DRP */

#if (defined(FSC_HAVE_DRP) || \
     (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACCMODE)))
void SetStateTrySink(Port_t *port)
{
    port->TCIdle = false;

    SetTypeCState(port, TrySink);
    USBPDDisable(port, true);
    setStateSink(port);

    /* Enable interrupts */
    port->Registers.Mask.M_ACTIVITY = 0;
    DeviceWrite(port->I2cAddr, regMask, 1, &port->Registers.Mask.byte);

    TimerStart(&port->StateTimer, tDRPTry);
}
#endif /* FSC_HAVE_DRP || (FSC_HAVE_SNK && FSC_HAVE_ACCMODE) */

#ifdef FSC_HAVE_DRP
void SetStateTryWaitSource(Port_t *port)
{
    port->TCIdle = false;

    port->Registers.Mask.M_COMP_CHNG = 0;
    DeviceWrite(port->I2cAddr, regMask, 1, &port->Registers.Mask.byte);
    platform_set_vbus_lvl_enable(port->PortID, VBUS_LVL_ALL, false, false);

    SetTypeCState(port, TryWaitSource);

    setStateSource(port, false);

    TimerStart(&port->StateTimer, tDRPTry);
}
#endif /* FSC_HAVE_DRP */

#ifdef FSC_HAVE_SRC
void SetStateDebugAccessorySource(Port_t *port)
{
    port->Registers.Mask.M_COMP_CHNG = 0;
    DeviceWrite(port->I2cAddr, regMask, 1, &port->Registers.Mask.byte);
    port->TCIdle = false;
    port->loopCounter = 0;

    platform_set_vbus_lvl_enable(port->PortID, VBUS_LVL_5V, true, true);
    SetTypeCState(port, DebugAccessorySource);

    setStateSource(port, false);

    TimerStart(&port->StateTimer, tOrientedDebug);
}
#endif /* FSC_HAVE_SRC */

#ifdef FSC_HAVE_ACCMODE
void SetStateAudioAccessory(Port_t *port)
{
    port->TCIdle = false;

    port->loopCounter = 0;

    platform_set_vbus_lvl_enable(port->PortID, VBUS_LVL_ALL, false, false);
    SetTypeCState(port, AudioAccessory);

    setStateSource(port, false);

    port->Registers.Mask.M_COMP_CHNG = 0;
    DeviceWrite(port->I2cAddr, regMask, 1, &port->Registers.Mask.byte);

    TimerDisable(&port->StateTimer);
}
#endif /* FSC_HAVE_ACCMODE */

#if (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACCMODE))
void SetStatePoweredAccessory(Port_t *port)
{
    port->TCIdle = false;

    platform_set_vbus_lvl_enable(port->PortID, VBUS_LVL_ALL, false, false);

    /* NOTE: Leave commented - Enable the 5V output for debugging */
    /*platform_set_vbus_lvl_enable(port->PortID, VBUS_LVL_5V, true, true);*/

    SetTypeCState(port, PoweredAccessory);
    setStateSource(port, true);

    /* If the current is default set it to 1.5A advert (Must be 1.5 or 3.0) */
    if (port->Registers.Control.HOST_CUR != 0x2)
    {
        port->Registers.Control.HOST_CUR = 0x2;
        DeviceWrite(port->I2cAddr, regControl0, 1,
                    &port->Registers.Control.byte[0]);
    }

    notify_observers((port->CCPin == CC1) ? CC1_ORIENT : CC2_ORIENT,
                     port->I2cAddr, 0);

    USBPDEnable(port, true, SOURCE);

    TimerStart(&port->StateTimer, tAMETimeout);
}

void SetStateUnsupportedAccessory(Port_t *port)
{
    port->TCIdle = true;

    /* Mask for COMP */
    port->Registers.Mask.M_COMP_CHNG = 0;
    DeviceWrite(port->I2cAddr, regMask, 1, &port->Registers.Mask.byte);

    platform_set_vbus_lvl_enable(port->PortID, VBUS_LVL_ALL, false, false);
    SetTypeCState(port, UnsupportedAccessory);
    setStateSource(port, false);

    /* Must advertise default current */
    port->Registers.Control.HOST_CUR = 0x1;
    DeviceWrite(port->I2cAddr, regControl0, 1,
                &port->Registers.Control.byte[0]);
    USBPDDisable(port, true);

    TimerDisable(&port->StateTimer);

    notify_observers(ACC_UNSUPPORTED, port->I2cAddr, 0);
}
#endif /* (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACCMODE)) */

#ifdef FSC_HAVE_DRP
void SetStateUnattachedSource(Port_t *port)
{
    /* Currently only implemented for AttachWaitSnk to Unattached for DRP */
    port->TCIdle = false;

    platform_set_vbus_lvl_enable(port->PortID, VBUS_LVL_ALL, false, false);
    SetTypeCState(port, UnattachedSource);
    port->CCPin = CCNone;

    setStateSource(port, false);

    USBPDDisable(port, true);

    TimerStart(&port->StateTimer, tTOG2);
}
#endif /* FSC_HAVE_DRP */

/* Type C Support Routines */
void updateSourceCurrent(Port_t *port)
{
    switch (port->SourceCurrent)
    {
    case utccDefault:
        /* Set the host current to reflect the default USB power */
        port->Registers.Control.HOST_CUR = 0x1;
        break;
    case utcc1p5A:
        /* Set the host current to reflect 1.5A */
        port->Registers.Control.HOST_CUR = 0x2;
        break;
    case utcc3p0A:
        /* Set the host current to reflect 3.0A */
        port->Registers.Control.HOST_CUR = 0x3;
        break;
    default:
        /* This assumes that there is no current being advertised */
        /* Set the host current to disabled */
        port->Registers.Control.HOST_CUR = 0x0;
        break;
    }
    DeviceWrite(port->I2cAddr, regControl0, 1,
                &port->Registers.Control.byte[0]);
}

void updateSourceMDACHigh(Port_t *port)
{
    switch (port->Registers.Control.HOST_CUR)
    {
    case 0x1:
        /* Set up DAC threshold to 1.6V (default USB current advertisement) */
        port->Registers.Measure.MDAC = MDAC_1P596V;
        break;
    case 0x2:
        /* Set up DAC threshold to 1.6V */
        port->Registers.Measure.MDAC = MDAC_1P596V;
        break;
    case 0x3:
        /* Set up DAC threshold to 2.6V */
        port->Registers.Measure.MDAC = MDAC_2P604V;
        break;
    default:
        /* This assumes that there is no current being advertised */
        /* Set up DAC threshold to 1.6V (default USB current advertisement) */
        port->Registers.Measure.MDAC = MDAC_1P596V;
        break;
    }
    DeviceWrite(port->I2cAddr, regMeasure, 1, &port->Registers.Measure.byte);
}

void updateSourceMDACLow(Port_t *port)
{
    switch (port->Registers.Control.HOST_CUR)
    {
    case 0x1:
        /* Set up DAC threshold to 1.6V (default USB current advertisement) */
        port->Registers.Measure.MDAC = MDAC_0P210V;
        break;
    case 0x2:
        /* Set up DAC threshold to 1.6V */
        port->Registers.Measure.MDAC = MDAC_0P420V;
        break;
    case 0x3:
        /* Set up DAC threshold to 2.6V */
        port->Registers.Measure.MDAC = MDAC_0P798V;
        break;
    default:
        /* This assumes that there is no current being advertised */
        /* Set up DAC threshold to 1.6V (default USB current advertisement) */
        port->Registers.Measure.MDAC = MDAC_1P596V;
        break;
    }
    DeviceWrite(port->I2cAddr, regMeasure, 1, &port->Registers.Measure.byte);
}

void ToggleMeasure(Port_t *port)
{
    /* Toggle measure block between CC pins */
    if (port->Registers.Switches.MEAS_CC2 == 1)
    {
        port->Registers.Switches.MEAS_CC1 = 1;
        port->Registers.Switches.MEAS_CC2 = 0;
    }
    else if (port->Registers.Switches.MEAS_CC1 == 1)
    {
        port->Registers.Switches.MEAS_CC1 = 0;
        port->Registers.Switches.MEAS_CC2 = 1;
    }

    DeviceWrite(port->I2cAddr, regSwitches0, 1,
                &port->Registers.Switches.byte[0]);
}

CCTermType DecodeCCTermination(Port_t *port)
{
    switch (port->sourceOrSink)
    {
#if (defined(FSC_HAVE_SRC) || \
     (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACCMODE)))
    case SOURCE:
        return DecodeCCTerminationSource(port);
#endif /* FSC_HAVE_SRC || (FSC_HAVE_SNK && FSC_HAVE_ACCMODE) */
#ifdef FSC_HAVE_SNK
    case SINK:
        return DecodeCCTerminationSink(port);
#endif /* FSC_HAVE_SNK */
    default:
        return CCTypeUndefined;
    }
}

#if (defined(FSC_HAVE_SRC) || \
     (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACCMODE)))
CCTermType DecodeCCTerminationSource(Port_t *port)
{
    CCTermType Termination = CCTypeUndefined;
    regMeasure_t saved_measure = port->Registers.Measure;

    /* Make sure MEAS_VBUS is cleared */
    if (port->Registers.Measure.MEAS_VBUS != 0)
    {
        port->Registers.Measure.MEAS_VBUS = 0;
        DeviceWrite(port->I2cAddr, regMeasure, 1,&port->Registers.Measure.byte);
    }

    /* Assume we are called with MDAC high. */
    /* Delay to allow measurement to settle */
    platform_delay_10us(25);
    DeviceRead(port->I2cAddr, regStatus0, 1, &port->Registers.Status.byte[4]);

    if (port->Registers.Status.COMPARATOR == 1)
    {
        /* Open voltage */
        Termination = CCTypeOpen;
        return Termination;
    }
    else if ((port->Registers.Switches.MEAS_CC1 && (port->CCPin == CC1)) ||
             (port->Registers.Switches.MEAS_CC2 && (port->CCPin == CC2)))
    {
        /* Optimization determines whether the pin is Open or Rd.  Ra level
         * is checked elsewhere.  This prevents additional changes to the MDAC
         * level which causes a continuous cycle of additional interrupts.
         */
        switch (port->Registers.Control.HOST_CUR)
        {
        case 0x1:
            Termination = CCTypeRdUSB;
            break;
        case 0x2:
            Termination = CCTypeRd1p5;
            break;
        case 0x3:
            Termination = CCTypeRd3p0;
            break;
        case 0x0:
            break;
        }

        return Termination;
    }

    /* Lower than open voltage - Rd or Ra */
    updateSourceMDACLow(port);

    /* Delay to allow measurement to settle */
    platform_delay_10us(25);
    DeviceRead(port->I2cAddr, regStatus0, 1,
               &port->Registers.Status.byte[4]);

    if (port->Registers.Status.COMPARATOR == 0)
    {
        /* Lower than Ra threshold is Ra */
        Termination = CCTypeRa;
    }
    else
    {
        /* Higher than Ra threshold is Rd */
        switch (port->Registers.Control.HOST_CUR)
        {
        case 0x1:
            Termination = CCTypeRdUSB;
            break;
        case 0x2:
            Termination = CCTypeRd1p5;
            break;
        case 0x3:
            Termination = CCTypeRd3p0;
            break;
        case 0x0:
            break;
        }
    }

    /* Restore Measure register */
    port->Registers.Measure = saved_measure;
    DeviceWrite(port->I2cAddr, regMeasure, 1, &port->Registers.Measure.byte);

    return Termination;
}

FSC_BOOL IsCCPinRa(Port_t *port)
{
    FSC_BOOL isRa = false;
    regMeasure_t saved_measure = port->Registers.Measure;

    /* Make sure MEAS_VBUS is cleared */
    port->Registers.Measure.MEAS_VBUS = 0;
    DeviceWrite(port->I2cAddr, regMeasure, 1, &port->Registers.Measure.byte);

    /* Lower than open voltage - Rd or Ra */
    updateSourceMDACLow(port);

    /* Delay to allow measurement to settle */
    platform_delay_10us(25);
    DeviceRead(port->I2cAddr, regStatus0, 1, &port->Registers.Status.byte[4]);

    isRa = (port->Registers.Status.COMPARATOR == 0) ? true : false;

    /* Restore Measure register */
    port->Registers.Measure = saved_measure;
    DeviceWrite(port->I2cAddr, regMeasure, 1, &port->Registers.Measure.byte);

    return isRa;
}
#endif /* FSC_HAVE_SRC || (FSC_HAVE_SNK && FSC_HAVE_ACCMODE) */

#ifdef FSC_HAVE_SNK
CCTermType DecodeCCTerminationSink(Port_t *port)
{
    CCTermType Termination;

    /* Delay to allow measurement to settle */
    platform_delay_10us(25);
    DeviceRead(port->I2cAddr, regStatus0, 1, &port->Registers.Status.byte[4]);

    /* Determine which level */
    switch (port->Registers.Status.BC_LVL)
    {
    case 0x0:
        /* If BC_LVL is lowest it's open */
        Termination = CCTypeOpen;
        break;
    case 0x1:
        /* If BC_LVL is 1, it's default */
        Termination = CCTypeRdUSB;
        break;
    case 0x2:
        /* If BC_LVL is 2, it's vRd1p5 */
        Termination = CCTypeRd1p5;
        break;
    default:
        /* Otherwise it's vRd3p0 */
        Termination = CCTypeRd3p0;
        break;
    }

    return Termination;
}
#endif /* FSC_HAVE_SNK */

#ifdef FSC_HAVE_SNK
void UpdateSinkCurrent(Port_t *port, CCTermType term)
{
    switch (term)
    {
    case CCTypeRdUSB:
        /* If we detect the default... */
        port->SinkCurrent = utccDefault;
        break;
    case CCTypeRd1p5:
        /* If we detect 1.5A */
        port->SinkCurrent = utcc1p5A;
        break;
    case CCTypeRd3p0:
        /* If we detect 3.0A */
        port->SinkCurrent = utcc3p0A;
        break;
    default:
        port->SinkCurrent = utccNone;
        break;
    }
}
#endif /* FSC_HAVE_SNK */

void UpdateCurrentAdvert(Port_t *port, USBTypeCCurrent Current)
{
    /* SourceCurrent value is of type USBTypeCCurrent */
    if (Current < utccInvalid)
    {
        port->SourceCurrent = Current;
        updateSourceCurrent(port);
    }
}

FSC_BOOL VbusVSafe0V(Port_t *port)
{
    return (!isVBUSOverVoltage(port, VBUS_MV_VSAFE0V)) ? true : false;
}

FSC_BOOL isVSafe5V(Port_t *port)
{
    return isVBUSOverVoltage(port, VBUS_MV_VSAFE5V_L);
}

FSC_BOOL isVBUSOverVoltage(Port_t *port, FSC_U16 vbus_mv)
{
//    return false;
    /* PPS Implementation requires better resolution vbus measurements than
     * the MDAC can provide.  If available on the platform, use an ADC
     * channel to measure the current vbus voltage.
     */
#ifdef PLATFORM_PIC32_ADC
    FSC_U16 adc = ReadADCChannel(4);
    FSC_U16 arg = vbus_mv / 23;

    return (arg <= adc) ? true : false;
#else
    regMeasure_t measure;

    FSC_U8 val;
    FSC_BOOL ret;
    FSC_BOOL mdacUpdated = false;

    /* Setup for VBUS measurement */
    measure.byte = 0;
    measure.MEAS_VBUS = 1;
    measure.MDAC = vbus_mv / MDAC_MV_LSB;

    /* The actual value of MDAC is less by 1 */
    if (measure.MDAC > 0)
    {
        measure.MDAC -= 1;
    }

    if (port->Registers.Measure.byte != measure.byte)
    {
        /* Update only if required */
//        DeviceWrite(port->I2cAddr, regMeasure, 1, &measure.byte);
        mdacUpdated = true;
        /* Delay to allow measurement to settle */
        platform_delay_10us(35);
    }

    DeviceRead(port->I2cAddr, regStatus0, 1, &val);
    /* COMP = bit 5 of status0 (Device specific?) */
    val &= 0x20;

    /* Determine return value based on COMP */
    ret = (val) ? true : false;

    if (mdacUpdated == true)
    {
        /* Restore register values */
        DeviceWrite(port->I2cAddr, regMeasure, 1, &port->Registers.Measure.byte);
    }

    return ret;
#endif /* PLATFORM_PIC32_ADC */
}

void DetectCCPinSource(Port_t *port)
{
    CCTermType CCTerm;
    FSC_BOOL CC1IsRa = false;

    if (port->Registers.DeviceID.VERSION_ID == VERSION_302A)
    {
        /* Enable CC1 pull-up and measure */
        port->Registers.Switches.byte[0] = 0x44;
    }
    else
    {
        /* Enable CC pull-ups and CC1 measure */
        port->Registers.Switches.byte[0] = 0xC4;
    }
    DeviceWrite(port->I2cAddr, regSwitches0, 1,
                &(port->Registers.Switches.byte[0]));

    CCTerm = DecodeCCTermination(port);

    if ((CCTerm >= CCTypeRdUSB) && (CCTerm < CCTypeUndefined))
    {
        port->CCPin = CC1;
        return;
    }
    else if (CCTerm == CCTypeRa)
    {
        CC1IsRa = true;
    }

    if (port->Registers.DeviceID.VERSION_ID == VERSION_302A)
    {
        /* Enable CC2 pull-up and measure */
        port->Registers.Switches.byte[0] = 0x88;
    }
    else
    {
        /* Enable CC pull-ups and CC2 measure */
        port->Registers.Switches.byte[0] = 0xC8;
    }

    DeviceWrite(port->I2cAddr, regSwitches0, 1,
                &(port->Registers.Switches.byte[0]));

    CCTerm = DecodeCCTermination(port);

    if ((CCTerm >= CCTypeRdUSB) && (CCTerm < CCTypeUndefined))
    {

        port->CCPin = CC2;
        return;
    }

    /* Only Ra found... on CC1 or CC2?
     * This supports correct dangling Ra cable behavior.
     */
    port->CCPin = CC1IsRa ? CC1 : (CCTerm == CCTypeRa) ? CC2 : CCNone;
}

void DetectCCPinSink(Port_t *port)
{
    CCTermType CCTerm;

    port->Registers.Switches.byte[0] = 0x07;
    DeviceWrite(port->I2cAddr, regSwitches0, 1,
                &(port->Registers.Switches.byte[0]));

    CCTerm = DecodeCCTermination(port);

    if ((CCTerm > CCTypeRa) && (CCTerm < CCTypeUndefined))
    {
        port->CCPin = CC1;
        return;
    }

    port->Registers.Switches.byte[0] = 0x0B;
    DeviceWrite(port->I2cAddr, regSwitches0, 1,
                &(port->Registers.Switches.byte[0]));

    CCTerm = DecodeCCTermination(port);

    if ((CCTerm > CCTypeRa) && (CCTerm < CCTypeUndefined))
    {

        port->CCPin = CC2;
        return;
    }
}

void resetDebounceVariables(Port_t *port)
{
    port->CCTerm = CCTypeUndefined;
    port->CCTermCCDebounce = CCTypeUndefined;
    port->CCTermPDDebounce = CCTypeUndefined;
    port->CCTermPDDebouncePrevious = CCTypeUndefined;
    port->VCONNTerm = CCTypeUndefined;
}

#ifdef FSC_DEBUG
FSC_BOOL GetLocalRegisters(Port_t *port, FSC_U8 * data, FSC_S32 size)
{
    if (size != 23) return false;

    data[0] = port->Registers.DeviceID.byte;
    data[1] = port->Registers.Switches.byte[0];
    data[2] = port->Registers.Switches.byte[1];
    data[3] = port->Registers.Measure.byte;
    data[4] = port->Registers.Slice.byte;
    data[5] = port->Registers.Control.byte[0];
    data[6] = port->Registers.Control.byte[1];
    data[7] = port->Registers.Control.byte[2];
    data[8] = port->Registers.Control.byte[3];
    data[9] = port->Registers.Mask.byte;
    data[10] = port->Registers.Power.byte;
    data[11] = port->Registers.Reset.byte;
    data[12] = port->Registers.OCPreg.byte;
    data[13] = port->Registers.MaskAdv.byte[0];
    data[14] = port->Registers.MaskAdv.byte[1];
    data[15] = port->Registers.Control4.byte;
    data[16] = port->Registers.Status.byte[0];
    data[17] = port->Registers.Status.byte[1];
    data[18] = port->Registers.Status.byte[2];
    data[19] = port->Registers.Status.byte[3];
    data[20] = port->Registers.Status.byte[4];
    data[21] = port->Registers.Status.byte[5];
    data[22] = port->Registers.Status.byte[6];

    return true;
}
#endif /* FSC_DEBUG */

void debounceCC(Port_t *port)
{
    /* The functionality here should work correctly using the Idle mode.
     * Will idling, a CC change or timer interrupt will
     * generate an appropriate update to the debounce state.
     */
    /* Grab the latest CC termination value */
    CCTermType CCTermCurrent = DecodeCCTermination(port);

    /* While debouncing to connect as a Sink, only care about one value for Rp.
     * When in AttachedSink state, debounce for sink sub-state. */
    if (port->sourceOrSink == SINK && port->ConnState != AttachedSink &&
        (CCTermCurrent == CCTypeRd1p5 || CCTermCurrent == CCTypeRd3p0))
    {
        CCTermCurrent = CCTypeRdUSB;
    }

    /* Check to see if the value has changed... */
    if (port->CCTerm != CCTermCurrent)
    {
        /* If it has, update the value */
        port->CCTerm = CCTermCurrent;

        /* Restart the debounce timer (wait 10ms before detach) */
        TimerStart(&port->PDDebounceTimer, tPDDebounce);
    }

    /* Check to see if our debounce timer has expired... */
    if (TimerExpired(&port->PDDebounceTimer))
    {
        /* Update the CC debounced values */
        port->CCTermPDDebounce = port->CCTerm;
        TimerDisable(&port->PDDebounceTimer);
    }

    /* CC debounce */
    if (port->CCTermPDDebouncePrevious != port->CCTermPDDebounce)
    {
        /* If the PDDebounce values have changed */
        /* Update the previous value */
        port->CCTermPDDebouncePrevious = port->CCTermPDDebounce;

        /* Reset the tCCDebounce timers */
        TimerStart(&port->CCDebounceTimer, tCCDebounce - tPDDebounce);

        /* Set CC debounce values to undefined while it is being debounced */
        port->CCTermCCDebounce = CCTypeUndefined;
    }

    if (TimerExpired(&port->CCDebounceTimer))
    {
        /* Update the CC debounced values */
        port->CCTermCCDebounce = port->CCTermPDDebouncePrevious;
        TimerDisable(&port->CCDebounceTimer);
    }
}

#if (defined(FSC_HAVE_SRC) || \
     (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACCMODE)))
void updateVCONNSource(Port_t *port)
{
    /* Assumes PUs have been set */

    /* Save current Switches */
    FSC_U8 saveRegister = port->Registers.Switches.byte[0];

    /* Toggle measure to VCONN */
    ToggleMeasure(port);

    /* Toggle PU if 302A */
    if (port->Registers.DeviceID.VERSION_ID == VERSION_302A)
    {
        if (port->CCPin == CC1)
        {
            port->Registers.Switches.PU_EN1 = 0;
            port->Registers.Switches.PU_EN2 = 1;
        }
        else
        {
            port->Registers.Switches.PU_EN1 = 1;
            port->Registers.Switches.PU_EN2 = 0;
        }

        DeviceWrite(port->I2cAddr, regSwitches0, 1,
                    &port->Registers.Switches.byte[0]);
    }

    port->VCONNTerm = DecodeCCTermination(port);

    /* Restore Switches */
    port->Registers.Switches.byte[0] = saveRegister;
    DeviceWrite(port->I2cAddr, regSwitches0, 1,
                &port->Registers.Switches.byte[0]);
}

void setStateSource(Port_t *port, FSC_BOOL vconn)
{
    port->sourceOrSink = SOURCE;
    resetDebounceVariables(port);
    updateSourceCurrent(port);
    updateSourceMDACHigh(port);

    /* Enable everything except internal oscillator */
    port->Registers.Power.POWER = 0x7;
    DeviceWrite(port->I2cAddr, regPower, 1, &port->Registers.Power.byte);

    /* For automated testing */
    if (port->CCPin == CCNone)
    {
        DetectCCPinSource(port);
    }

    if (port->CCPin == CC1)
    {
        /* If we detected CC1 as an Rd */
        if (port->Registers.DeviceID.VERSION_ID == VERSION_302A)
        {
            /* Enable CC1 pull-up and measure */
            port->Registers.Switches.byte[0] = 0x44;
        }
        else
        {
            /* Enable CC pull-ups and CC1 measure */
            port->Registers.Switches.byte[0] = 0xC4;
        }
    }
    else
    {
        /* If we detected CC2 as an Rd */
        if (port->Registers.DeviceID.VERSION_ID == VERSION_302A)
        {
            /* Enable CC2 pull-up and measure */
            port->Registers.Switches.byte[0] = 0x88;
        }
        else
        {
            /* Enable CC pull-ups and CC2 measure */
            port->Registers.Switches.byte[0] = 0xC8;
        }
    }
    DeviceWrite(port->I2cAddr, regSwitches0, 1,
                &port->Registers.Switches.byte[0]);

    updateVCONNSource(port);

    /* Turn on VConn after checking the VConn termination */
    if (vconn && Type_C_Sources_VCONN)
    {
#if 0
        if (port->CCPin == CC1)
            port->Registers.Switches.VCONN_CC2 = 1;
        else
            port->Registers.Switches.VCONN_CC1 = 1;

        // WTF?

        DeviceWrite(port->I2cAddr, regSwitches0, 1,
                    &port->Registers.Switches.byte[0]);
#endif
    }

    port->SinkCurrent = utccNone;

    TimerDisable(&port->PDDebounceTimer);
    TimerStart(&port->CCDebounceTimer, tCCDebounce);
}
#endif /* FSC_HAVE_SRC || (FSC_HAVE_SNK && FSC_HAVE_ACCMODE)) */

#ifdef FSC_HAVE_SNK
void setStateSink(Port_t *port)
{
    /* Disable the vbus outputs */
    platform_set_vbus_lvl_enable(port->PortID, VBUS_LVL_ALL, false, false);

    port->sourceOrSink = SINK;
    resetDebounceVariables(port);

    /* Enable everything except internal oscillator */
    port->Registers.Power.POWER = 0x7;
    DeviceWrite(port->I2cAddr, regPower, 1, &port->Registers.Power.byte);

    /* For automated testing */
    if (port->CCPin == CCNone)
    {
        DetectCCPinSink(port);
    }

    if (port->CCPin == CC1)
    {
        /* If we detected CC1 as an Rp, enable PD's on CC1 */
        port->Registers.Switches.byte[0] = 0x07;
    }
    else
    {
        /* If we detected CC2 as an Rp, enable PD's on CC2 */
        port->Registers.Switches.byte[0] = 0x0B;
    }
    DeviceWrite(port->I2cAddr, regSwitches0, 1,
                &port->Registers.Switches.byte[0]);


    /* Set up VBus measure interrupt to watch for detach */
    port->Registers.Measure.MEAS_VBUS = 1;
    port->Registers.Measure.MDAC = (port->DetachThreshold / MDAC_MV_LSB) - 1;
    port->Registers.Mask.M_COMP_CHNG = 0;
    port->Registers.Mask.M_BC_LVL = 0;
    DeviceWrite(port->I2cAddr, regMeasure, 1, &port->Registers.Measure.byte);
    DeviceWrite(port->I2cAddr, regMask, 1, &port->Registers.Mask.byte);

    updateVCONNSink(port);

    TimerDisable(&port->PDDebounceTimer);
    TimerStart(&port->CCDebounceTimer, tCCDebounce);
}

void updateVCONNSink(Port_t *port)
{
    /* Assumes Rd has been set */
    ToggleMeasure(port);

    port->VCONNTerm = DecodeCCTermination(port);

    ToggleMeasure(port);
}

#endif /* FSC_HAVE_SNK */

void clearState(Port_t *port)
{
    /* Disable the vbus outputs */
    platform_set_vbus_lvl_enable(port->PortID, VBUS_LVL_ALL, false, false);

#ifdef FSC_HAVE_PPS_SOURCE
    platform_set_pps_voltage(port->PortID, 0);
#endif /* FSC_HAVE_PPS_SOURCE */

    USBPDDisable(port, true);

    /* Mask/disable interrupts */
    port->Registers.Mask.byte = 0xFF;
    DeviceWrite(port->I2cAddr, regMask, 1, &port->Registers.Mask.byte);
    port->Registers.MaskAdv.byte[0] = 0xFF;
    DeviceWrite(port->I2cAddr, regMaska, 1, &port->Registers.MaskAdv.byte[0]);
    port->Registers.MaskAdv.M_GCRCSENT = 1;
    DeviceWrite(port->I2cAddr, regMaskb, 1, &port->Registers.MaskAdv.byte[1]);

    port->Registers.Control.TOGGLE = 0;         /* Disable toggling */
    port->Registers.Control.HOST_CUR = 0x0;     /* Clear PU advertisement */
    DeviceWrite(port->I2cAddr, regControl0, 3,
                &port->Registers.Control.byte[0]);

    port->Registers.Switches.byte[0] = 0x00;    /* Disable PU, PD, etc. */
    DeviceWrite(port->I2cAddr, regSwitches0, 1,
                &port->Registers.Switches.byte[0]);

    SetConfiguredCurrent(port);
    resetDebounceVariables(port);
    port->CCPin = CCNone;

    TimerDisable(&port->PDDebounceTimer);
    TimerDisable(&port->CCDebounceTimer);

    notify_observers(CC_NO_ORIENT, port->I2cAddr, 0);

    port->Registers.Power.POWER = 0x1;
    DeviceWrite(port->I2cAddr, regPower, 1, &port->Registers.Power.byte);
}

void SetStateIllegalCable(Port_t *port)
{
    port->TCIdle = true;

    port->loopCounter = 0;

    platform_set_vbus_lvl_enable(port->PortID, VBUS_LVL_ALL, false, false);
    platform_set_vbus_discharge(port->PortID, true);

    /* Disable toggle */
    port->Registers.Control.TOGGLE = 0;
    DeviceWrite(port->I2cAddr, regControl2, 1,&port->Registers.Control.byte[2]);

    SetTypeCState(port, IllegalCable);

    UpdateCurrentAdvert(port, utcc3p0A);

    /* This level (MDAC == 0x24) seems to be appropriate for 3.0A PU's */
    port->Registers.Measure.MDAC = MDAC_1P596V - 1;
    port->Registers.Measure.MEAS_VBUS = 0;
    DeviceWrite(port->I2cAddr, regMeasure, 1, &port->Registers.Measure.byte);

    port->sourceOrSink = SOURCE;

    /* Enable everything except internal oscillator */
    port->Registers.Power.POWER = 0x7;
    DeviceWrite(port->I2cAddr, regPower, 1, &port->Registers.Power.byte);

    /* Determine Orientation.
     * NOTE: This code enables both pullups and pulldowns in order to provide
     * a somewhat accurate reading with an illegal cable.
     */
    if (port->Registers.DeviceID.VERSION_ID == VERSION_302A)
    {
        /* Enable CC1 pull-up and pull-downs and measure */
        port->Registers.Switches.byte[0] = 0x47;
    }
    else
    {
        /* Enable CC pull-ups and pull-downs and CC1 measure */
        port->Registers.Switches.byte[0] = 0xC7;
    }
    DeviceWrite(port->I2cAddr, regSwitches0, 1,
                &(port->Registers.Switches.byte[0]));

    port->CCPin = CC1;
    port->CCTerm = DecodeCCTermination(port);

    if ((port->CCTerm >= CCTypeRdUSB) && (port->CCTerm < CCTypeUndefined))
    {
    }
    else
    {
        port->CCPin = CC2;
        if (port->Registers.DeviceID.VERSION_ID == VERSION_302A)
        {
            /* Enable CC2 pull-up and pull-downs and measure */
            port->Registers.Switches.byte[0] = 0x8B;
        }
        else
        {
            /* Enable CC pull-ups and pull-downs and CC2 measure */
            port->Registers.Switches.byte[0] = 0xCB;
        }

        DeviceWrite(port->I2cAddr, regSwitches0, 1,
                    &(port->Registers.Switches.byte[0]));

        port->CCTerm = DecodeCCTermination(port);
    }

    if ((port->CCTerm >= CCTypeRdUSB) && (port->CCTerm < CCTypeUndefined))
    {
        port->Registers.Mask.M_COMP_CHNG = 0;
        port->Registers.Mask.M_BC_LVL = 0;
        DeviceWrite(port->I2cAddr, regMask, 1, &port->Registers.Mask.byte);

        TimerDisable(&port->StateTimer);
    }
    else
    {
        /* Couldn't find an appropriate termination - detach and try again */
        SetStateUnattached(port);
    }
}

void StateMachineIllegalCable(Port_t *port)
{
    /* This state provides a stable landing point for dangling or illegal
     * cables.  These are either unplugged travel adapters or
     * some flavor of unplugged Type-C to Type-A cable (Rp to VBUS).
     * The combination of capacitance and PullUp/PullDown causes a repeated
     * cycle of attach-detach that could continue ad infinitum or until the
     * cable or travel adapter is plugged in.  This state breaks the loop and
     * waits for a change in termination.
     * NOTE: In most cases this requires VBUS bleed resistor (~7kohm)
     */
    port->TCIdle = true;

    if (port->Registers.Status.I_COMP_CHNG == 1)
    {
        port->CCTerm = DecodeCCTermination(port);

        if (port->CCTerm == CCTypeOpen)
        {
            platform_set_vbus_discharge(port->PortID, false);

            SetStateUnattached(port);
        }
    }
}
