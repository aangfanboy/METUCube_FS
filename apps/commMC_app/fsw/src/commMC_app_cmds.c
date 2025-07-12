
#include "commMC_app.h"
#include "commMC_app_cmds.h"
#include "commMC_app_msgids.h"

#include "adcsMC_app_extern_typedefs.h"
#include "payloadMC_app_extern_typedefs.h"

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

CFE_Status_t COMMMC_APP_SEND_HK_TO_SB()
{
    CFE_Status_t status = CFE_SUCCESS;

    status = COMMMC_appPrepareHkPacket();
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(COMMMC_HK_PREP_ERR_EID, CFE_EVS_EventType_ERROR,
                          "COMMMC: Error preparing HK packet, status: %d", status);
        return status;
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(COMMMC_AppData.HkPacket.TelemetryHeader));
    status = CFE_SB_TransmitMsg(CFE_MSG_PTR(COMMMC_AppData.HkPacket.TelemetryHeader), true);

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(COMMMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "COMMMC: Error sending HK packet to SB, status: %d", status);
        return status;
    }

    CFE_EVS_SendEvent(COMMMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_INFORMATION,
                      "COMMMC: HK packet sent successfully");
    
    return status;
}

CFE_Status_t COMMMC_APP_SEND_MINIMAL_TM_TO_GROUND()
{
    CFE_Status_t status = CFE_SUCCESS;

    const char *port = "/dev/ttyUSB0"; // Example port, adjust as necessary
    AdcsMC_MinimalTelemetry_t adcs_telemetry_data = {
        .quaternion = {1.0, 0.0, 0.0, 0.0},
        .angular_velocity = {0.1, 0.2, 0.3},
        .speed_vector = {7.5, -3.2, 1.1},
        .position_vector = {100.0, 200.0, 300.0}
    };

    PayloadMC_MinimalTelemetry_t payload_telemetry_data = {
        .batteryPercentage = 85
    };

    // Prepare the data to send
    COMMMC_APP_MinimalTelemetryPacket_t minimal_tm_packet;
    minimal_tm_packet.TelemetryPayload.AdcsTelemetry = adcs_telemetry_data;
    minimal_tm_packet.TelemetryPayload.PayloadTelemetry = payload_telemetry_data;

    uint32 crc32OfPayload = CFE_ES_CalculateCRC((const void *)&minimal_tm_packet.TelemetryPayload,
                                                  sizeof(minimal_tm_packet.TelemetryPayload), 0, CFE_ES_CrcType_CRC_32);
    
    uint32 packetDataLength = (uint32)(sizeof(minimal_tm_packet.TelemetryPayload) + sizeof(minimal_tm_packet.TelemetrySecondaryHeader));

    minimal_tm_packet.TelemetryHeader = COMMMC_APP_CREATE_TELEMETRY_HEADER(COMMMC_APP_MINIMAL_TM_MTID, packetDataLength);
    minimal_tm_packet.TelemetrySecondaryHeader = COMMMC_APP_CREATE_TELEMETRY_SECONDARY_HEADER(crc32OfPayload);

    OS_printf("COMMMC: Payload ADCS size: %zu, Payload size: %zu\n, Total Payload size: %zu\n",
              sizeof(minimal_tm_packet.TelemetryPayload.AdcsTelemetry),
              sizeof(minimal_tm_packet.TelemetryPayload.PayloadTelemetry),
              sizeof(minimal_tm_packet.TelemetryPayload));
    OS_printf("COMMMC: Header size: %zu\n",
              sizeof(minimal_tm_packet.TelemetryHeader));
    OS_printf("COMMMC: Secondary Header size: %zu\n",
              sizeof(minimal_tm_packet.TelemetrySecondaryHeader));
    OS_printf("COMMMC: Total packet size: %zu\n",
              sizeof(minimal_tm_packet));
    OS_printf("COMMMC: CRC32 of Payload: 0x%08X\n", crc32OfPayload);

    status = COMMMC_APP_SEND_DATA_TO_GROUND(port, (const unsigned char *)&minimal_tm_packet, sizeof(minimal_tm_packet));

    return status;
}

CFE_Status_t COMMMC_APP_SEND_DATA_TO_GROUND(const char *port, const unsigned char *data, size_t length) {
    CFE_Status_t status = CFE_SUCCESS;

    int fd = open(port, O_WRONLY | O_NOCTTY);
    if (fd < 0) return 1;

    struct termios tty;
    tcgetattr(fd, &tty);

    tty.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    tty.c_iflag = 0;
    tty.c_oflag = 0;
    tty.c_lflag = 0;

    tcsetattr(fd, TCSANOW, &tty);

    write(fd, data, length);

    close(fd);

    return status;
}

COMMMC_APP_TelemetryHeaderPacket_t COMMMC_APP_CREATE_TELEMETRY_HEADER(uint32 packetIdentificationMTID, uint32 sizeOfPayloadAndSecondaryHeader) {
    COMMMC_APP_TelemetryHeaderPacket_t telemetry_header;

    telemetry_header.packetVersion = 1; // Set to 1, can be modified as needed
    telemetry_header.packetIdentificationType = 0; // Set to 0 for telemetry
    telemetry_header.packetIdentificationMTID = packetIdentificationMTID; // Set to the appropriate Message ID
    telemetry_header.packetSequenceControl = 3; // Set to 3 for unsegmented telemetry
    telemetry_header.packetSequenceCount = 0; // Initialize sequence count to 0
    telemetry_header.packetDataLength = sizeOfPayloadAndSecondaryHeader; // Set to the size of the payload and secondary header

    return telemetry_header;
}

COMMMC_APP_TelemetrySecondaryHeaderPacket_t COMMMC_APP_CREATE_TELEMETRY_SECONDARY_HEADER(uint32 crc32OfPayload) {
    COMMMC_APP_TelemetrySecondaryHeaderPacket_t telemetry_secondary_header;
    CFE_TIME_SysTime_t current_time;

    current_time = CFE_TIME_GetTime();

    telemetry_secondary_header.timestamp = current_time.Seconds;
    telemetry_secondary_header.crc32 = crc32OfPayload;

    return telemetry_secondary_header;
}
