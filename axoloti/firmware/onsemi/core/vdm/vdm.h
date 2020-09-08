/*******************************************************************************
 * @file     vdm.h
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
#ifndef __VDM_MANAGER_H__
#define __VDM_MANAGER_H__

#include "vendor_info.h"
#include "platform.h"
#include "fsc_vdm_defs.h"
#include "vdm_callbacks_defs.h"
#include "PD_Types.h"
#include "Port.h"

#ifdef FSC_HAVE_VDM

#define SVID_DEFAULT SVID1_SOP
#define MODE_DEFAULT 0x0001
#define SVID_AUTO_ENTRY 0x1057
#define MODE_AUTO_ENTRY 0x0001

#define NUM_VDM_MODES 6
#define MAX_NUM_SVIDS_PER_SOP 30
#define MAX_SVIDS_PER_MESSAGE 12
#define MIN_DISC_ID_RESP_SIZE 3

/* Millisecond values ticked by 1ms timer. */
#define tVDMSenderResponse 27 * TICK_SCALE_TO_MS
#define tVDMWaitModeEntry  45 * TICK_SCALE_TO_MS
#define tVDMWaitModeExit   45 * TICK_SCALE_TO_MS

/*
 * Initialization functions.
 */
FSC_S32 initializeVdm(Port_t *port);

/*
 * Functions to go through PD VDM flow.
 */
/* Initiations from DPM */
FSC_S32 requestDiscoverIdentity(Port_t *port, SopType sop);
FSC_S32 requestDiscoverSvids(Port_t *port, SopType sop);
FSC_S32 requestDiscoverModes(Port_t *port, SopType sop, FSC_U16 svid);
FSC_S32 requestSendAttention(Port_t *port, SopType sop, FSC_U16 svid,
                             FSC_U8 mode);
FSC_S32 requestEnterMode(Port_t *port, SopType sop, FSC_U16 svid,
                         FSC_U32 mode_index);
FSC_S32 requestExitMode(Port_t *port, SopType sop, FSC_U16 svid,
                        FSC_U32 mode_index);
FSC_S32 requestExitAllModes(void);

/* receiving end */
FSC_S32 processVdmMessage(Port_t *port, SopType sop, FSC_U32* arr_in,
                          FSC_U32 length_in);
FSC_S32 processDiscoverIdentity(Port_t *port, SopType sop, FSC_U32* arr_in,
                                FSC_U32 length_in);
FSC_S32 processDiscoverSvids(Port_t *port, SopType sop, FSC_U32* arr_in,
                             FSC_U32 length_in);
FSC_S32 processDiscoverModes(Port_t *port, SopType sop, FSC_U32* arr_in,
                             FSC_U32 length_in);
FSC_S32 processEnterMode(Port_t *port, SopType sop, FSC_U32* arr_in,
                         FSC_U32 length_in);
FSC_S32 processExitMode(Port_t *port, SopType sop, FSC_U32* arr_in,
                        FSC_U32 length_in);
FSC_S32 processAttention(Port_t *port, SopType sop, FSC_U32* arr_in,
                         FSC_U32 length_in);
FSC_S32 processSvidSpecific(Port_t *port, SopType sop, FSC_U32* arr_in,
                            FSC_U32 length_in);

/* Private function */
FSC_BOOL evalResponseToSopVdm(Port_t *port, doDataObject_t vdm_hdr);
FSC_BOOL evalResponseToCblVdm(Port_t *port, doDataObject_t vdm_hdr);
void sendVdmMessageWithTimeout(Port_t *port, SopType sop, FSC_U32* arr,
                               FSC_U32 length, FSC_S32 n_pe);
void vdmMessageTimeout(Port_t *port);
void startVdmTimer(Port_t *port, FSC_S32 n_pe);
void sendVdmMessageFailed(Port_t *port);
void resetPolicyState(Port_t *port);

void sendVdmMessage(Port_t *port, SopType sop, FSC_U32 * arr, FSC_U32 length,
                    PolicyState_t next_ps);

FSC_BOOL evaluateModeEntry (Port_t *port, FSC_U32 mode_in);

#endif /* FSC_HAVE_VDM */
#endif /* __VDM_MANAGER_H__ */
