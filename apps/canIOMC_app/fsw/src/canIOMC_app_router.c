#include "canIOMC_app_router.h"
#include "canIOMC_app_header_defs.h"
#include "canIOMC_app_msgids.h"
#include "canIOMC_app_msg.h"
#include "cfe_sb_api_typedefs.h" /* CFE_SB_MSGID_WRAP_VALUE */
#include <string.h>

/*
** CAN MessageID -> SB MID listen table.
**
** Add an entry here for every unprompted (not request/response) CAN
** message CANIOMC needs to forward straight onto another app's SB pipe.
*/
static const CANIOMC_MsgRouteEntry_t CANIOMC_MsgRouteTable[] = {
    /* Payload "take photo" trigger, arrives unprompted on 0xA7 */
    {CANIOMC_PAYLOAD_TAKEPHOTO_MSGID, CFE_SB_MSGID_WRAP_VALUE(CANIOMC_PAYLOAD_TAKEPHOTO_MID)},
};

#define CANIOMC_MSG_ROUTE_TABLE_SIZE (sizeof(CANIOMC_MsgRouteTable) / sizeof(CANIOMC_MsgRouteTable[0]))

bool CANIOMC_RouteIncomingCanMsg(uint16 CanMessageID, const uint8 *Payload, uint8 PayloadLen)
{
    uint32 i;

    for (i = 0; i < CANIOMC_MSG_ROUTE_TABLE_SIZE; i++)
    {
        if (CANIOMC_MsgRouteTable[i].CanMessageID == CanMessageID)
        {
            CANIOMC_RouteTriggerPkt_t TriggerPkt;

            memset(&TriggerPkt, 0, sizeof(TriggerPkt));

            if (PayloadLen > CANIOMC_SB_MAX_PAYLOAD)
            {
                PayloadLen = CANIOMC_SB_MAX_PAYLOAD;
            }

            TriggerPkt.PayloadLen = PayloadLen;
            if (Payload != NULL && PayloadLen > 0)
            {
                memcpy(TriggerPkt.Payload, Payload, PayloadLen);
            }

            CFE_MSG_Init(CFE_MSG_PTR(TriggerPkt.TelemetryHeader), CANIOMC_MsgRouteTable[i].SbMid,
                        sizeof(TriggerPkt));
            CFE_SB_TimeStampMsg(CFE_MSG_PTR(TriggerPkt.TelemetryHeader));
            CFE_SB_TransmitMsg(CFE_MSG_PTR(TriggerPkt.TelemetryHeader), true);

            return true;
        }
    }

    return false;
}
