/*******************************************************************************
 * @file     PDProtocol.h
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
#ifndef _PDPROTOCOL_H_
#define	_PDPROTOCOL_H_

/////////////////////////////////////////////////////////////////////////////
//                              Required headers
/////////////////////////////////////////////////////////////////////////////
#include "platform.h"
#include "Port.h"

#ifdef FSC_DEBUG
#include "Log.h"
#endif /* FSC_DEBUG */

/* USB PD Protocol Layer Routines */
void USBPDProtocol(Port_t *port);
void ProtocolSendCableReset(Port_t *port);
void ProtocolIdle(Port_t *port);
void ProtocolResetWait(Port_t *port);
void ProtocolRxWait(void);
void ProtocolGetRxPacket(Port_t *port, FSC_BOOL HeaderReceived);
void ProtocolTransmitMessage(Port_t *port);
void ProtocolSendingMessage(Port_t *port);
void ProtocolWaitForPHYResponse(void);
void ProtocolVerifyGoodCRC(Port_t *port);
void ProtocolSendGoodCRC(Port_t *port, SopType sop);
void ProtocolLoadSOP(Port_t *port, SopType sop);
void ProtocolLoadEOP(Port_t *port);
void ProtocolSendHardReset(Port_t *port);
void ProtocolFlushRxFIFO(Port_t *port);
void ProtocolFlushTxFIFO(Port_t *port);
void ResetProtocolLayer(Port_t *port, FSC_BOOL ResetPDLogic);

#ifdef FSC_DEBUG
/* Logging and debug functions */
FSC_BOOL StoreUSBPDToken(Port_t *port, FSC_BOOL transmitter,
                         USBPD_BufferTokens_t token);
FSC_BOOL StoreUSBPDMessage(Port_t *port, sopMainHeader_t Header,
                           doDataObject_t* DataObject,
                           FSC_BOOL transmitter, FSC_U8 SOPType);
FSC_U8 GetNextUSBPDMessageSize(Port_t *port);
FSC_U8 GetUSBPDBufferNumBytes(Port_t *port);
FSC_BOOL ClaimBufferSpace(Port_t *port, FSC_S32 intReqSize);
FSC_U8 ReadUSBPDBuffer(Port_t *port, FSC_U8* pData, FSC_U8 bytesAvail);

void SendUSBPDHardReset(Port_t *port);

#endif /* FSC_DEBUG */

#endif	/* _PDPROTOCOL_H_ */

