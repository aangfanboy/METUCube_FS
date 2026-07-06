
#include "adcsMC_app.h"
#include "adcsMC_app_cmds.h"
#include "adcsMC_app_msgids.h"
#include "canIOMC_app_msg.h"
#include "canIOMC_app_msgids.h"
#include "canIOMC_app_header_defs.h"

#include "cfe.h"
#include <string.h>


CFE_Status_t ADCSMC_APP_SEND_HK_TO_SB()
{
    CFE_Status_t status = CFE_SUCCESS;

    status = ADCSMC_appPrepareHkPacket();
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ADCSMC_HK_PREP_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ADCSMC: Error preparing HK packet, status: %d", status);

        ADCSMC_AppData.ErrCounter++;

        return status;
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(ADCSMC_AppData.HkPacket.TelemetryHeader));
    status = CFE_SB_TransmitMsg(CFE_MSG_PTR(ADCSMC_AppData.HkPacket.TelemetryHeader), true);

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ADCSMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ADCSMC: Error sending HK packet to SB, status: %d", status);

        ADCSMC_AppData.ErrCounter++;

        return status;
    }

    CFE_EVS_SendEvent(ADCSMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_INFORMATION,
                      "ADCSMC: HK packet sent successfully");

    return status;
}

CFE_Status_t ADCSMC_APP_SEND_HK_CAN_REQUEST_TO_SB(void)
{
    CFE_Status_t          status = CFE_SUCCESS;
    CANIOMC_CanPacketSB_t CanHkRequest;

    memset(&CanHkRequest, 0, sizeof(CanHkRequest));

    CFE_MSG_Init(CFE_MSG_PTR(CanHkRequest.MessageHeader),
                 CFE_SB_ValueToMsgId(CANIOMC_CMD_MID),
                 sizeof(CANIOMC_CanPacketSB_t));

    /* CAN header fields — SeqType/SeqCount are set by the segmentation engine */
    CanHkRequest.Header.Priority   = CANIOMC_HKPRIORITY;
    CanHkRequest.Header.SenderID   = CANIOMC_OBC_ID;
    CanHkRequest.Header.ReceiverID = CANIOMC_ADCS_ID;
    CanHkRequest.Header.MessageID  = CANIOMC_ADCS_HK_MSGID;

    /* No payload — this is a pure request frame */
    CanHkRequest.PayloadLen = 0;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(CanHkRequest.MessageHeader));

    status = CFE_SB_TransmitMsg(CFE_MSG_PTR(CanHkRequest.MessageHeader), true);

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ADCSMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ADCSMC: CAN HK Request could not be sent to SB, status: 0x%08X", (unsigned int)status);

        ADCSMC_AppData.ErrCounter++;
        return status;
    }

    CFE_EVS_SendEvent(ADCSMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_DEBUG,
                      "ADCSMC: CAN HK Request forwarded to CAN_IO successfully");

    ADCSMC_AppData.CmdCounter++;

    return status;
}

CFE_Status_t ADCSMC_ProcessAdcsTlm(const CFE_SB_Buffer_t *SBBufPtr)
{
    const CANIOMC_AdcsTlmPacket_t *AdcsPkt;

    if (SBBufPtr == NULL)
    {
        return CFE_SUCCESS;
    }

    AdcsPkt = (const CANIOMC_AdcsTlmPacket_t *)SBBufPtr;

    memcpy(ADCSMC_AppData.QuaternionEst,  AdcsPkt->Adcs.QuaternionEst,  sizeof(ADCSMC_AppData.QuaternionEst));
    memcpy(ADCSMC_AppData.AngularVelEst,  AdcsPkt->Adcs.AngularVelEst,  sizeof(ADCSMC_AppData.AngularVelEst));
    memcpy(ADCSMC_AppData.BiasEst,        AdcsPkt->Adcs.BiasEst,        sizeof(ADCSMC_AppData.BiasEst));
    memcpy(ADCSMC_AppData.PosEst,         AdcsPkt->Adcs.PosEst,         sizeof(ADCSMC_AppData.PosEst));
    memcpy(ADCSMC_AppData.VelEst,         AdcsPkt->Adcs.VelEst,         sizeof(ADCSMC_AppData.VelEst));
    memcpy(ADCSMC_AppData.PqEst,          AdcsPkt->Adcs.PqEst,          sizeof(ADCSMC_AppData.PqEst));
    memcpy(ADCSMC_AppData.PbEst,          AdcsPkt->Adcs.PbEst,          sizeof(ADCSMC_AppData.PbEst));
    memcpy(ADCSMC_AppData.SunUnitVector1, AdcsPkt->Adcs.SunUnitVector1, sizeof(ADCSMC_AppData.SunUnitVector1));
    memcpy(ADCSMC_AppData.SunUnitVector2, AdcsPkt->Adcs.SunUnitVector2, sizeof(ADCSMC_AppData.SunUnitVector2));
    memcpy(ADCSMC_AppData.MagUnitVector1, AdcsPkt->Adcs.MagUnitVector1, sizeof(ADCSMC_AppData.MagUnitVector1));
    memcpy(ADCSMC_AppData.MagUnitVector2, AdcsPkt->Adcs.MagUnitVector2, sizeof(ADCSMC_AppData.MagUnitVector2));
    memcpy(ADCSMC_AppData.SunSensorTemp,  AdcsPkt->Adcs.SunSensorTemp,  sizeof(ADCSMC_AppData.SunSensorTemp));

    ADCSMC_AppData.AdcsMissCount = 0;

    CFE_EVS_SendEvent(ADCSMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_DEBUG,
                      "ADCSMC: ADCS cache updated (q0=%d.%02d)",
                      (int)ADCSMC_AppData.QuaternionEst[0],
                      (int)((ADCSMC_AppData.QuaternionEst[0] - (int)ADCSMC_AppData.QuaternionEst[0]) * 100));

    return CFE_SUCCESS;
}
