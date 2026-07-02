/**
 * @file canIOMC_segmentation.c
 * @brief Generic CAN segmentation / reassembly engine implementation.
 */

#include "canIOMC_segmentation.h"
#include "cfe.h"
#include <string.h>

/* ------------------------------------------------------------------ */
/* TX                                                                  */
/* ------------------------------------------------------------------ */

int32 CANIO_SendSegmented(const CANIOMC_CAN_Header_t *Hdr,
                          const uint8 *Data, uint8 Len)
{
    CANIO_Frame_t  Frame;
    uint8          seqCount  = 0;
    uint8          seqType;
    uint8          offset    = 0;
    uint8          chunkLen;
    int32          status;

    if (Hdr == NULL)
    {
        return CFE_STATUS_BAD_COMMAND_CODE;
    }

    /* Determine framing mode upfront */
    bool multiFrame = (Len > CANIO_HAL_MAX_DLC;);

    if (Len == 0){return CFE_SUCCESS}

    do
    {
        chunkLen = (Len - offset);
        if (chunkLen > CANIO_HAL_MAX_DLC)
        {
            chunkLen = CANIO_HAL_MAX_DLC;
        }

        /* Assign SeqType */
        if (!multiFrame)
        {
            seqType = CANIO_SEQ_SINGLE;
        }
        else if (seqCount == 0)
        {
            seqType = CANIO_SEQ_FIRST;
        }
        else if ((offset + chunkLen) >= Len)
        {
            seqType = CANIO_SEQ_LAST;
        }
        else
        {
            seqType = CANIO_SEQ_CONT;
        }

        Frame.CanId = CANIO_PACK_ID(Hdr->Priority,
                                    Hdr->SenderID,
                                    Hdr->ReceiverID,
                                    Hdr->MessageID,
                                    seqType,
                                    seqCount);

        memset(Frame.Data, 0, CANIO_HAL_MAX_DLC);
        if (Data != NULL && chunkLen > 0)
        {
            memcpy(Frame.Data, Data + offset, chunkLen);
        }
        Frame.Len = CANIO_HAL_MAX_DLC;   /* always send full 8-byte frame */

        status = CANIO_HAL_Send(&Frame);
        if (status != CFE_SUCCESS)
        {
            return status;
        }

        offset   += chunkLen;
        seqCount++;

    } while (offset < Len);
    /* Len==0 case: sends exactly one SINGLE frame with zero payload */

    return CFE_SUCCESS;
}

/* ------------------------------------------------------------------ */
/* RX helpers                                                          */
/* ------------------------------------------------------------------ */

static CANIO_ReassemblySlot_t *FindSlot(CANIO_ReassemblySlot_t *Slots,
                                        uint8 SenderID, uint8 ReceiverID,
                                        uint16 MessageID)
{
    int i;
    for (i = 0; i < CANIO_MAX_REASSEMBLY_SLOTS; i++)
    {
        if (Slots[i].Active &&
            Slots[i].SenderID   == SenderID   &&
            Slots[i].ReceiverID == ReceiverID  &&
            Slots[i].MessageID  == MessageID)
        {
            return &Slots[i];
        }
    }
    return NULL;
}

static CANIO_ReassemblySlot_t *AllocSlot(CANIO_ReassemblySlot_t *Slots,
                                         uint8 SenderID, uint8 ReceiverID,
                                         uint16 MessageID)
{
    int i;
    for (i = 0; i < CANIO_MAX_REASSEMBLY_SLOTS; i++)
    {
        if (!Slots[i].Active)
        {
            memset(&Slots[i], 0, sizeof(Slots[i]));
            Slots[i].Active     = true;
            Slots[i].SenderID   = SenderID;
            Slots[i].ReceiverID = ReceiverID;
            Slots[i].MessageID  = MessageID;
            return &Slots[i];
        }
    }
    return NULL;  /* all slots busy */
}

static void FreeSlot(CANIO_ReassemblySlot_t *Slot)
{
    Slot->Active = false;
}

/* Append up to 8 bytes into a slot; returns false on overflow */
static bool AppendToSlot(CANIO_ReassemblySlot_t *Slot,
                         const uint8 *Data, uint8 DataLen)
{
    if ((Slot->Len + DataLen) > CANIO_REASSEMBLY_BUF_SIZE)
    {
        return false;
    }
    memcpy(Slot->Buf + Slot->Len, Data, DataLen);
    Slot->Len += DataLen;
    return true;
}

/* ------------------------------------------------------------------ */
/* RX                                                                  */
/* ------------------------------------------------------------------ */

int32 CANIO_FeedFrame(CANIO_ReassemblySlot_t *Slots,
                      const CANIO_Frame_t    *Frame,
                      uint8  *OutBuf,
                      uint8  *OutLen,
                      uint8  *OutSenderID,
                      uint16 *OutMessageID)
{
    uint8  seqType   = (uint8)CANIO_UNPACK_SEQTYPE(Frame->CanId);
    uint8  seqCount  = (uint8)CANIO_UNPACK_SEQCOUNT(Frame->CanId);
    uint8  senderID  = (uint8)CANIO_UNPACK_SENDERID(Frame->CanId);
    uint8  receiverID= (uint8)CANIO_UNPACK_RECEIVERID(Frame->CanId);
    uint16 messageID = (uint16)CANIO_UNPACK_MESSAGEID(Frame->CanId);

    CANIO_ReassemblySlot_t *Slot;

    switch (seqType)
    {
        /* ---------------------------------------------------------- */
        case CANIO_SEQ_SINGLE:
        /* ---------------------------------------------------------- */
            memcpy(OutBuf, Frame->Data, Frame->Len);
            *OutLen      = Frame->Len;
            *OutSenderID = senderID;
            *OutMessageID= messageID;
            return CFE_SUCCESS;

        /* ---------------------------------------------------------- */
        case CANIO_SEQ_FIRST:
        /* ---------------------------------------------------------- */
            /* Discard any stale slot for the same stream and open a fresh one */
            Slot = FindSlot(Slots, senderID, receiverID, messageID);
            if (Slot != NULL)
            {
                FreeSlot(Slot);
            }

            Slot = AllocSlot(Slots, senderID, receiverID, messageID);
            if (Slot == NULL)
            {
                CFE_ES_WriteToSysLog("CANIO SEG: no free reassembly slot\n");
                return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
            }

            AppendToSlot(Slot, Frame->Data, Frame->Len);
            Slot->NextSeqCount = seqCount + 1;
            return CANIO_REASSEMBLY_PENDING;

        /* ---------------------------------------------------------- */
        case CANIO_SEQ_CONT:
        /* ---------------------------------------------------------- */
            Slot = FindSlot(Slots, senderID, receiverID, messageID);
            if (Slot == NULL)
            {
                /* No matching first frame seen — drop */
                return CANIO_REASSEMBLY_PENDING;
            }

            AppendToSlot(Slot, Frame->Data, Frame->Len);
            Slot->NextSeqCount = seqCount + 1;
            return CANIO_REASSEMBLY_PENDING;

        /* ---------------------------------------------------------- */
        case CANIO_SEQ_LAST:
        /* ---------------------------------------------------------- */
            Slot = FindSlot(Slots, senderID, receiverID, messageID);
            if (Slot == NULL)
            {
                return CANIO_REASSEMBLY_PENDING;
            }

            if (!AppendToSlot(Slot, Frame->Data, Frame->Len))
            {
                FreeSlot(Slot);
                CFE_ES_WriteToSysLog("CANIO SEG: reassembly overflow\n");
                return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
            }

            memcpy(OutBuf, Slot->Buf, Slot->Len);
            *OutLen       = Slot->Len;
            *OutSenderID  = senderID;
            *OutMessageID = messageID;

            FreeSlot(Slot);
            return CFE_SUCCESS;

        default:
            return CANIO_REASSEMBLY_PENDING;
    }
}
