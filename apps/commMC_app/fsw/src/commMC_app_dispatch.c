#include "commMC_app.h"
#include "commMC_app_dispatch.h"
#include "commMC_app_imgxfer.h"
#include "commMC_app_extern_typedefs.h"
#include "canIOMC_app_msgids.h"
#include "payloadMC_app_msgids.h"
#include "payloadMC_app_msg.h"
#include "ds_msgids.h"

#include "hk_msgids.h"

void COMMMC_appTaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId    = CFE_SB_INVALID_MSG_ID;
    CFE_Status_t status = CFE_SUCCESS;

    const COMMMC_APP_ProcessCmd_t *CmdPtr;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);
    
    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case COMMMC_CMD_MID:
            CFE_EVS_SendEvent(COMMMC_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "COMMMC: Received command packet");

            COMMMC_AppData.CmdCounter++;

            CmdPtr = (const COMMMC_APP_ProcessCmd_t *)SBBufPtr;
            
            if (CmdPtr->OutMsgToSend == COMMMC_APP_COMMAND_TASK_ID_SEND_MINIMAL_TM_TO_GROUND)
            {
                status = COMMMC_APP_SEND_MINIMAL_TM_TO_GROUND();
                if (status != CFE_SUCCESS)
                {
                    CFE_EVS_SendEvent(COMMMC_SEND_MINIMAL_TM_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "COMMMC: Error sending minimal telemetry to ground, status: %d", status);
                } else {
                    CFE_EVS_SendEvent(COMMMC_SEND_MINIMAL_TM_SUCCESS_EID, CFE_EVS_EventType_INFORMATION,
                                      "COMMMC: Successfully sent minimal telemetry to ground");
                }
            }
            else if (CmdPtr->OutMsgToSend == COMMMC_APP_COMMAND_TASK_ID_SEND_MAX)
            {
                // Handle sending maximum telemetry to ground

            }
            else
            {
                CFE_EVS_SendEvent(COMMMC_UNKNOWN_COMMAND_TASK_ID_ERR_EID, CFE_EVS_EventType_ERROR,
                                    "COMMMC: Invalid command task ID = %u", CmdPtr->OutMsgToSend);
            }
            break; 

        case COMMMC_SEND_HK_MID:
            CFE_EVS_SendEvent(COMMMC_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "COMMMC: Received HK request");

            COMMMC_APP_SEND_HK_TO_SB();
            break;

        case CANIOMC_COMM_TLM_MID:
            COMMMC_ProcessCommTlm(SBBufPtr);
            break;

        case PAYLOADMC_IMAGING_MODE_MID:
        {
            const PAYLOADMC_ImagingModePkt_t *ImagingPkt = (const PAYLOADMC_ImagingModePkt_t *)SBBufPtr;
            COMMMC_AppData.IsImaging = ImagingPkt->IsImaging;
            break;
        }

        /* --- Image transfer to the COMM card over SPI --- */
        case DS_COMP_TLM_MID:
            /* DS finished a file; remember it if it's a photo */
            COMMMC_ImgXfer_OnDsFileComplete(SBBufPtr);
            break;

        case CANIOMC_COMM_IMG_REQUEST_MID:
            /* COMM asked for the last payload image — start the transfer */
            COMMMC_ImgXfer_OnRequest(SBBufPtr);
            break;

        case CANIOMC_IMG_XFER_BEGIN_ACK_MID:
            COMMMC_ImgXfer_OnBeginAck(SBBufPtr);
            break;

        case CANIOMC_IMG_CHUNK_ACK_MID:
            COMMMC_ImgXfer_OnChunkAck(SBBufPtr);
            break;

        case CANIOMC_IMG_XFER_RESULT_MID:
            COMMMC_ImgXfer_OnResult(SBBufPtr);
            break;

        case CANIOMC_SPI_TX_DONE_MID:
            COMMMC_ImgXfer_OnSpiDone(SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(COMMMC_UNKNOWN_MSG_ERR_EID, CFE_EVS_EventType_ERROR,
                              "COMMMC: invalid packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break; 
    }
}
