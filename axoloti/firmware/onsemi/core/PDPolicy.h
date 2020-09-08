/*******************************************************************************
 * @file     PDPolicy.h
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
#ifndef _PDPOLICY_H_
#define _PDPOLICY_H_

#include "platform.h"
#include "Port.h"

#ifdef FSC_DEBUG
#include "Log.h"
#endif /* FSC_DEBUG */

void USBPDEnable(Port_t *port, FSC_BOOL DeviceUpdate, SourceOrSink TypeCDFP);
void USBPDDisable(Port_t *port, FSC_BOOL DeviceUpdate);

void USBPDPolicyEngine(Port_t *port);
void PolicyErrorRecovery(Port_t *port);

#if (defined(FSC_HAVE_SRC) || \
     (defined(FSC_HAVE_SNK) && defined(FSC_HAVE_ACCMODE)))
void PolicySourceSendHardReset(Port_t *port);
void PolicySourceSoftReset(Port_t *port, SopType sop);
void PolicySourceSendSoftReset(Port_t *port);
void PolicySourceStartup(Port_t *port);
void PolicySourceDiscovery(Port_t *port);
void PolicySourceSendCaps(Port_t *port);
void PolicySourceDisabled(Port_t *port);
void PolicySourceTransitionDefault(Port_t *port);
void PolicySourceNegotiateCap(Port_t *port);
void PolicySourceTransitionSupply(Port_t *port);
void PolicySourceCapabilityResponse(Port_t *port);
void PolicySourceReady(Port_t *port);
void PolicySourceGiveSourceCap(Port_t *port);
void PolicySourceGetSourceCap(Port_t *port);
void PolicySourceGetSinkCap(Port_t *port);
void PolicySourceSendPing(Port_t *port);
void PolicySourceGotoMin(Port_t *port);
void PolicySourceGiveSinkCap(Port_t *port);
void PolicySourceSendDRSwap(Port_t *port);
void PolicySourceEvaluateDRSwap(Port_t *port);
void PolicySourceSendVCONNSwap(Port_t *port);
void PolicySourceEvaluateVCONNSwap(Port_t *port);
void PolicySourceSendPRSwap(Port_t *port);
void PolicySourceEvaluatePRSwap(Port_t *port);
void PolicySourceWaitNewCapabilities(Port_t *port);
void PolicySourceAlertReceived(Port_t *port);
#endif /* FSC_HAVE_SRC || (FSC_HAVE_SNK && FSC_HAVE_ACCMODE) */

#ifdef FSC_HAVE_SNK
void PolicySinkSendHardReset(Port_t *port);
void PolicySinkSoftReset(Port_t *port);
void PolicySinkSendSoftReset(Port_t *port);
void PolicySinkTransitionDefault(Port_t *port);
void PolicySinkStartup(Port_t *port);
void PolicySinkDiscovery(Port_t *port);
void PolicySinkWaitCaps(Port_t *port);
void PolicySinkEvaluateCaps(Port_t *port);
void PolicySinkSelectCapability(Port_t *port);
void PolicySinkTransitionSink(Port_t *port);
void PolicySinkReady(Port_t *port);
void PolicySinkGiveSinkCap(Port_t *port);
void PolicySinkGetSinkCap(Port_t *port);
void PolicySinkGiveSourceCap(Port_t *port);
void PolicySinkGetSourceCap(Port_t *port);
void PolicySinkSendDRSwap(Port_t *port);
void PolicySinkEvaluateDRSwap(Port_t *port);
void PolicySinkSendVCONNSwap(Port_t *port);
void PolicySinkEvaluateVCONNSwap(Port_t *port);
void PolicySinkSendPRSwap(Port_t *port);
void PolicySinkEvaluatePRSwap(Port_t *port);
void PolicySinkAlertReceived(Port_t *port);
#endif /* FSC_HAVE_SNK */

void PolicyNotSupported(Port_t *port);
void PolicyInvalidState(Port_t *port);
void policyBISTReceiveMode(Port_t *port);
void policyBISTFrameReceived(Port_t *port);
void policyBISTCarrierMode2(Port_t *port);
void policyBISTTestData(Port_t *port);

#ifdef FSC_HAVE_EXT_MSG
void PolicyGetCountryCodes(Port_t *port);
void PolicyGiveCountryCodes(Port_t *port);
void PolicyGiveCountryInfo(Port_t *port);
void PolicyGetPPSStatus(Port_t *port);
void PolicyGivePPSStatus(Port_t *port);
#endif /* FSC_HAVE_EXT_MSG */

void PolicySendGenericCommand(Port_t *port);
void PolicySendGenericData(Port_t *port);

void PolicySendHardReset(Port_t *port);

FSC_U8 PolicySendCommand(Port_t *port, FSC_U8 Command, PolicyState_t nextState,
                         FSC_U8 subIndex, SopType sop);

FSC_U8 PolicySendData(Port_t *port, FSC_U8 MessageType, void* data,
                      FSC_U32 len, PolicyState_t nextState,
                      FSC_U8 subIndex, SopType sop, FSC_BOOL extMsg);

void UpdateCapabilitiesRx(Port_t *port, FSC_BOOL IsSourceCaps);

void processDMTBIST(Port_t *port);

#ifdef FSC_HAVE_VDM
/* Shim functions for VDM code */
void InitializeVdmManager(Port_t *port);
void convertAndProcessVdmMessage(Port_t *port, SopType sop);
void doVdmCommand(Port_t *port);
void doDiscoverIdentity(Port_t *port);
void doDiscoverSvids(Port_t *port);
void PolicyGiveVdm(Port_t *port);
void PolicyVdm(Port_t *port);
void autoVdmDiscovery(Port_t *port);
void PolicySendCableReset(Port_t *port);
void processCableResetState(Port_t *port);
#endif /* FSC_HAVE_VDM */

SopType TokenToSopType(FSC_U8 data);

#endif /* _PDPOLICY_H_ */
