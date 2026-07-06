/**
 * @file canIOMC_app_router.h
 * @brief CAN MessageID -> Software Bus MID listen/route table.
 *
 * Every other RX path in CANIOMC (see CANIOMC_PollAndPublishCanRx in
 * canIOMC_app_cmds.c) is a request/response flow: the OBC asked a specific
 * node for something, so the reply is matched on (SenderID, MessageID)
 * and copied into a dedicated per-subsystem SB packet.
 *
 * This router covers a different case: a CAN message that arrives to the
 * OBC *unprompted* (no request was ever sent for it) and just needs to be
 * forwarded, by MessageID alone, onto whichever app's SB pipe cares about
 * it. Adding a new "listen for this ID, wake up that app" behavior means
 * adding one line to CANIOMC_MsgRouteTable in canIOMC_app_router.c --
 * no new dispatch branch required.
 */

#ifndef CANIOMC_APP_ROUTER_H
#define CANIOMC_APP_ROUTER_H

#include "cfe.h"

/** One CAN MessageID -> SB MID mapping. */
typedef struct
{
    uint16         CanMessageID; /**< 10-bit CAN MessageID to listen for (any sender) */
    CFE_SB_MsgId_t SbMid;        /**< SB MID to publish (empty trigger packet) when matched */
} CANIOMC_MsgRouteEntry_t;

/**
 * @brief Look up a reassembled CAN MessageID in the route table and, if
 * found, publish a trigger packet carrying the full payload on the
 * mapped SB MID.
 *
 * @param CanMessageID The 10-bit CAN MessageID that was just reassembled.
 * @param Payload      Pointer to the fully reassembled payload bytes
 *                     (already de-segmented if the source was multi-frame).
 * @param PayloadLen   Number of valid bytes in Payload (0..CANIOMC_SB_MAX_PAYLOAD).
 * @return true if a route matched and the SB packet was sent, false if
 *         the MessageID isn't in the table (caller should fall back to
 *         its own "unhandled message" logging).
 */
bool CANIOMC_RouteIncomingCanMsg(uint16 CanMessageID, const uint8 *Payload, uint8 PayloadLen);

#endif /* CANIOMC_APP_ROUTER_H */
