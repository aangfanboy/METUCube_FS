
#include "commMC_app.h"
#include "commMC_app_cmds.h"
#include "commMC_app_msgids.h"

#include "adcsMC_app_extern_typedefs.h"
#include "adcsttMC_app_extern_typedefs.h"
#include "powerMC_app_extern_typedefs.h"

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
        .position_vector = {100.0, 200.0, 300.0},
        .rw_speeds = {1500, 1600, 1700}
    };

    AdcsttMC_MinimalTelemetry_t adcstt_telemetry_data = {
        .quaternion = {0.7071, 0.0, 0.7071, 0.0},
        .angular_velocity = {0.05, 0.1, 0.15},
        .speed_vector = {5.0, -2.0, 0.5},
        .position_vector = {50.0, 100.0, 150.0},
        .rw_speeds = {1200, 1300, 1400}
    };

    PowerMC_MinimalTelemetry_t power_telemetry_data = {
        .batteryPercentage = 85,
        .batteryHealth = 95
    };

    // Prepare the data to send
    COMMMC_APP_MinimalTelemetryPacket_t minimal_tm_packet;
    minimal_tm_packet.TelemetryPayload.AdcsTelemetry = adcs_telemetry_data;
    minimal_tm_packet.TelemetryPayload.AdcsttTelemetry = adcstt_telemetry_data;
    minimal_tm_packet.TelemetryPayload.PowerTelemetry = power_telemetry_data;

    uint32 crc32OfPayload = 0;
    // Calculate the CRC32 of the telemetry payload
    compute_crc32((const void *)&minimal_tm_packet.TelemetryPayload,
                  sizeof(minimal_tm_packet.TelemetryPayload), &crc32OfPayload);
    
    uint32 packetDataLength = (uint32)(sizeof(minimal_tm_packet.TelemetryPayload) + sizeof(minimal_tm_packet.TelemetrySecondaryHeader));

    minimal_tm_packet.TelemetryHeader = COMMMC_APP_CREATE_TELEMETRY_HEADER(COMMMC_APP_MINIMAL_TM_MTID, packetDataLength, 3, 0); // 3 for non-sequence control, 0 for sequence count since not valid
    minimal_tm_packet.TelemetrySecondaryHeader = COMMMC_APP_CREATE_TELEMETRY_SECONDARY_HEADER(crc32OfPayload);

    status = COMMMC_APP_SEND_DATA_TO_GROUND(port, (const unsigned char *)&minimal_tm_packet, sizeof(minimal_tm_packet));

    return status;
}

CFE_Status_t COMMMC_APP_SEND_FILE_TO_GROUND(const char *file_path){
    CFE_Status_t status = CFE_SUCCESS;

    // Open the file
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        CFE_EVS_SendEvent(COMMMC_FILE_OPEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "COMMMC: Error opening file %s", file_path);
        return CFE_SEVERITY_ERROR;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    if (file_size < 0) {
        CFE_EVS_SendEvent(COMMMC_FILE_SIZE_ERR_EID, CFE_EVS_EventType_ERROR,
                            "COMMMC: Error getting file size for %s", file_path);
        fclose(file);
        return CFE_SEVERITY_ERROR;
    }
    uint32 totalFileSize = (uint32)file_size;
    fseek(file, 0, SEEK_SET); // Reset file pointer to the beginning

    uint16 pduDataLengthBytes = 256; // Example PDU data length, adjust as necessary
    uint16 numberOfPDUs = (uint16)((totalFileSize + pduDataLengthBytes - 1) / pduDataLengthBytes); // Calculate number of PDUs

    // calculate the SHA-256 hash of the file
    unsigned char fileHash[32]; // 256 bits for SHA-256
    if (compute_sha256(file, fileHash) != CFE_SUCCESS) {
        CFE_EVS_SendEvent(COMMMC_FILE_HASH_ERR_EID, CFE_EVS_EventType_ERROR,
                            "COMMMC: Error calculating file hash for %s", file_path);
        fclose(file);
        return CFE_SEVERITY_ERROR;
    }

    // Prepare the file transfer header
    COMMMC_APP_FileTransferHeaderPacket_t file_transfer_header = {
        .fileDataID = 0, // Unique identifier for the file transfer
        .uniqueFileID = 12, // Unique identifier for the file transfer session
        .fileSize = totalFileSize, // Size of the file being transferred
        .fileHash = {0}, // Initialize file hash
        .numberOfPDUs = numberOfPDUs, // Number of PDUs in the file transfer
        .pduDataLength = pduDataLengthBytes // Length of each PDU in the file transfer
    };

    // Copy the file hash into the header
    memcpy(file_transfer_header.fileHash, fileHash, sizeof(file_transfer_header.fileHash));
    // Prepare the telemetry secondary header
    COMMMC_APP_TelemetrySecondaryHeaderPacket_t telemetry_secondary_header;
    COMMMC_APP_TelemetryHeaderPacket_t telemetry_header;
    COMMMC_APP_FileTransferPacket_t file_transfer_packet;

    // init file data buffer
    memset(file_transfer_packet.FileData, 0, sizeof(file_transfer_packet.FileData));

    telemetry_secondary_header = COMMMC_APP_CREATE_TELEMETRY_SECONDARY_HEADER(0); // 0 since no payload CRC for the init packet

    // Create the file transfer init packet

    COMMMC_APP_FileTransferInitPacket_t file_transfer_init_packet;
    file_transfer_init_packet.FileTransferHeader = file_transfer_header;
    file_transfer_init_packet.TelemetrySecondaryHeader = telemetry_secondary_header;

    // Send the file transfer init packet to ground
    status = COMMMC_APP_SEND_DATA_TO_GROUND("/dev/ttyUSB0", (const unsigned char *)&file_transfer_init_packet, sizeof(file_transfer_init_packet));
    if (status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(COMMMC_FILE_SEND_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "COMMMC: Error sending file transfer init packet to ground");
        fclose(file);
        return status;
    }

    CFE_EVS_SendEvent(COMMMC_FILE_SEND_INIT_SUCCESS_EID, CFE_EVS_EventType_INFORMATION,
                      "COMMMC: File transfer init packet sent successfully to ground");

    uint32 counter = 0;
    uint32 controlNumber = 0;
    uint32 crc32OfPayload = 0;
    size_t bytes_read = 0; // Variable to store the number of bytes read from the file

    bool halfChunk = false; // Flag to indicate if we are sending a half chunk

    // Read the file contents
    unsigned char buffer[pduDataLengthBytes]; // Buffer for file data
    while (1) {
        bytes_read = fread(buffer, 1, sizeof(buffer), file);
        if (bytes_read == 0) {
            if (feof(file)) {
                break; // End of file reached
            } else {
                CFE_EVS_SendEvent(COMMMC_FILE_READ_ERR_EID, CFE_EVS_EventType_ERROR,
                                    "COMMMC: Error reading file %s", file_path);
                fclose(file);
                return CFE_SEVERITY_ERROR;
            }
        }
        
        if (counter == 0) {
            controlNumber = 1;  // Set control number for the first chunk
        } else {
            controlNumber = 0;  // Set control number for subsequent chunks
        }

        compute_crc32((const void *)buffer, bytes_read, &crc32OfPayload);
        
        if (bytes_read < sizeof(buffer)) {
            halfChunk = true; // If we read less than the buffer size, we are sending a half chunk
            counter++; // Increment counter for the last chunk
            break; // If less than buffer size, we are at the end of the file
        }
        
        // Create the telemetry header for this chunk
        telemetry_header = COMMMC_APP_CREATE_TELEMETRY_HEADER(COMMMC_APP_FILE_SEND_TM_MTID, 
                                                                                  (uint32)(bytes_read + sizeof(COMMMC_APP_TelemetrySecondaryHeaderPacket_t)), 
                                                                                  controlNumber, 
                                                                                  counter);

        // Create the telemetry secondary header for this chunk
        telemetry_secondary_header = COMMMC_APP_CREATE_TELEMETRY_SECONDARY_HEADER(crc32OfPayload);
        // Create the file transfer packet for this chunk
        file_transfer_packet.TelemetryHeader = telemetry_header;
        file_transfer_packet.TelemetrySecondaryHeader = telemetry_secondary_header;
        memcpy(file_transfer_packet.FileData, buffer, bytes_read); // Copy the read data into the file transfer packet

        // Send the file transfer packet to ground
        status = COMMMC_APP_SEND_DATA_TO_GROUND("/dev/ttyUSB0", (const unsigned char *)&file_transfer_packet, sizeof(file_transfer_packet));

        if (status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(COMMMC_FILE_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                                "COMMMC: Error sending file transfer packet to ground, counter: %u, status: %d", counter, status);
        }

        counter++;
    }

    controlNumber = 2; // Set control number for the last chunk
    if (!halfChunk) {
        bytes_read = 0; // If we didn't read a half chunk, set bytes_read to 0
        crc32OfPayload = 0; // Set CRC32 to 0 for the last chunk
        
        // set whole buffer to 0 for the last chunk
        memset(buffer, 0, sizeof(buffer));
    }

    telemetry_header = COMMMC_APP_CREATE_TELEMETRY_HEADER(COMMMC_APP_FILE_SEND_TM_MTID,
                                                           (uint32)(bytes_read + sizeof(COMMMC_APP_TelemetrySecondaryHeaderPacket_t)),
                                                           controlNumber,
                                                           counter);

    telemetry_secondary_header = COMMMC_APP_CREATE_TELEMETRY_SECONDARY_HEADER(crc32OfPayload);

    // Create the file transfer packet for the last chunk
    file_transfer_packet.TelemetryHeader = telemetry_header;
    file_transfer_packet.TelemetrySecondaryHeader = telemetry_secondary_header;
    memcpy(file_transfer_packet.FileData, buffer, sizeof(buffer));

    // Send the last file transfer packet to ground
    status = COMMMC_APP_SEND_DATA_TO_GROUND("/dev/ttyUSB0", (const unsigned char *)&file_transfer_packet, sizeof(file_transfer_packet));

    fclose(file);

    if (status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(COMMMC_FILE_SEND_LAST_ERR_EID, CFE_EVS_EventType_ERROR,
                          "COMMMC: Error sending last file transfer packet to ground, counter: %u, status: %d", counter, status);
        return status;
    }

    CFE_EVS_SendEvent(COMMMC_FILE_SEND_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "COMMMC: File %s sent successfully to ground", file_path);

    return CFE_SUCCESS;
}

COMMMC_APP_TelemetryHeaderPacket_t COMMMC_APP_CREATE_TELEMETRY_HEADER(uint32 packetIdentificationMTID, uint32 sizeOfPayloadAndSecondaryHeader, uint32 packetSequenceControl, uint32 packetSequenceCount) {
    COMMMC_APP_TelemetryHeaderPacket_t telemetry_header;

    telemetry_header.packetVersion = 1; // Set to 1, can be modified as needed
    telemetry_header.packetIdentificationType = 0; // Set to 0 for telemetry
    telemetry_header.packetIdentificationMTID = packetIdentificationMTID; // Set to the appropriate Message ID
    telemetry_header.packetSequenceControl = packetSequenceControl; // Set to the appropriate sequence control value
    telemetry_header.packetSequenceCount = packetSequenceCount; // Initialize sequence count to 0
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

void COMMMC_APP_LISTENER_TASK(void)
{
    int serial_fd = open("/dev/ttyUSB0", O_RDONLY | O_NOCTTY | O_NONBLOCK);
    if (serial_fd < 0)
    {
        CFE_EVS_SendEvent(0, CFE_EVS_EventType_ERROR, "COMM: Failed to open /dev/ttyUSB0");
        return;
    }

    // Configure serial port
    struct termios tty;
    tcgetattr(serial_fd, &tty);
    cfsetispeed(&tty, B9600);  // Adjust to your baud rate
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;  // 8-bit
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_iflag = 0;
    tcsetattr(serial_fd, TCSANOW, &tty);
    unsigned char buffer[256];  // Buffer for incoming data

    while (1)
    {

        ssize_t n = read(serial_fd, buffer, sizeof(buffer));
        if (n > 0)
        {
            OS_printf("Received %zd bytes: ", n);
        }

        OS_TaskDelay(100);  // Sleep for 10ms
    }

    close(serial_fd);
}