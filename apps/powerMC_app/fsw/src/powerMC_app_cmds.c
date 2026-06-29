
#include "powerMC_app.h"
#include "powerMC_app_cmds.h"
#include "powerMC_app_msgids.h"
#include "canIOMC_app_msg.h"
#include "canIOMC_app_msgids.h"

#include "cfe.h"


CFE_Status_t POWERMC_APP_SEND_HK_TO_SB()
{
    CFE_Status_t status = CFE_SUCCESS;

    status = POWERMC_appPrepareHkPacket();
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(POWERMC_HK_PREP_ERR_EID, CFE_EVS_EventType_ERROR,
                          "POWERMC: Error preparing HK packet, status: %d", status);

        POWERMC_AppData.ErrCounter++;

        return status;
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(POWERMC_AppData.HkPacket.TelemetryHeader));
    status = CFE_SB_TransmitMsg(CFE_MSG_PTR(POWERMC_AppData.HkPacket.TelemetryHeader), true);

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(POWERMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "POWERMC: Error sending HK packet to SB, status: %d", status);

        POWERMC_AppData.ErrCounter++;

        return status;
    }

    CFE_EVS_SendEvent(POWERMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_INFORMATION,
                      "POWERMC: HK packet sent successfully");
    
    return status;
}

CFE_Status_t POWERMC_APP_SEND_HK_CAN_REQUEST_TO_SB(void)
{
    CFE_Status_t status = CFE_SUCCESS;
    CANIOMC_CanPacketSB_t CanHkRequest;

    /* 1. cFS Mesaj Header'ını Başlat (Mesajı CANIOMC_CMD_MID hedefine yolluyoruz) */
    CFE_MSG_Init(CFE_MSG_PTR(CanHkRequest.MessageHeader), 
                 CFE_SB_ValueToMsgId(CANIOMC_CMD_MID), 
                 sizeof(CANIOMC_CanPacketSB_t));

    /* 2. 29-bit CAN ID'yi Paketle 
    ** Priority: 01 (High), Sender: 0x02 (OBC Power), Receiver: 0x03 (EPS), 
    ** MsgID: 10 (HK Istek Kodu), SeqType: 0, SeqCount: 1 
    */
    CanHkRequest.Header.Priority = 0x01;
    CanHkRequest.Header.SenderID = 0x02;
    CanHkRequest.Header.ReceiverID = 0x03;
    CanHkRequest.Header.MessageID = 0x000A; // HK Request Command
    CanHkRequest.Header.SeqType = 0x00;
    CanHkRequest.Header.SeqCount = 0x01;

    // no payload for this command, just a request
    CanHkRequest.Payload.RawBytes[0] = 0x00;

    /* 4. Zaman damgası vur */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(CanHkRequest.MessageHeader));

    /* 5. Software Bus'a Fırlat */
    status = CFE_SB_TransmitMsg(CFE_MSG_PTR(CanHkRequest.MessageHeader), true);

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(POWERMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "POWERMC: CAN HK Request SB'ye gonderilemedi, status: 0x%08X", (unsigned int)status);
        
        POWERMC_AppData.ErrCounter++;
        return status;
    }

    CFE_EVS_SendEvent(POWERMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_DEBUG,
                      "POWERMC: CAN HK Request basariyla CAN_IO'ya iletildi");
    
    POWERMC_AppData.CmdCounter++;
    
    return status;
}
