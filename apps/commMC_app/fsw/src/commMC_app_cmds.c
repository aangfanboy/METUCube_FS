
#include "commMC_app.h"
#include "commMC_app_cmds.h"
#include "commMC_app_msgids.h"

#include "adcsMC_app_extern_typedefs.h"
#include "powerMC_app_extern_typedefs.h"

// System / POSIX includes for serial and TCP
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

// TCP socket headers
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

// Default ground endpoint; can be overridden at compile time
#ifndef COMMMC_APP_GROUND_ENDPOINT
#define COMMMC_APP_GROUND_ENDPOINT "tcp://127.0.0.1:5000"
#endif

CFE_Status_t COMMMC_APP_SEND_HK_TO_SB()
{
    CFE_Status_t status = CFE_SUCCESS;

    status = COMMMC_appPrepareHkPacket();
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(COMMMC_HK_PREP_ERR_EID, CFE_EVS_EventType_ERROR,
                          "COMMMC: Error preparing HK packet, status: %d", status);

        COMMMC_AppData.ErrCounter++;

        return status;
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(COMMMC_AppData.HkPacket.TelemetryHeader));
    status = CFE_SB_TransmitMsg(CFE_MSG_PTR(COMMMC_AppData.HkPacket.TelemetryHeader), true);

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(COMMMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "COMMMC: Error sending HK packet to SB, status: %d", status);

        COMMMC_AppData.ErrCounter++;

        return status;
    }

    CFE_EVS_SendEvent(COMMMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_INFORMATION,
                      "COMMMC: HK packet sent successfully");
    
    return status;
}

CFE_Status_t COMMMC_APP_SEND_MINIMAL_TM_TO_GROUND()
{
    CFE_Status_t status = CFE_SUCCESS;

    const char *port = COMMMC_APP_GROUND_ENDPOINT; // Endpoint can be serial path or tcp://host:port
    AdcsMC_MinimalTelemetry_t adcs_telemetry_data = {
        .quaternion = {1.0, 0.0, 0.0, 0.0},
        .angular_velocity = {0.1, 0.2, 0.3},
        .speed_vector = {7.5, -3.2, 1.1},
        .position_vector = {100.0, 200.0, 300.0},
        .rw_speeds = {1500, 1600, 1700}
    };

    PowerMC_MinimalTelemetry_t power_telemetry_data = {
        .batteryPercentage = 85,
        .batteryHealth = 95
    };

    // Prepare the data to send
    COMMMC_APP_MinimalTelemetryPacket_t minimal_tm_packet;
    minimal_tm_packet.TelemetryPayload.AdcsTelemetry = adcs_telemetry_data;
    minimal_tm_packet.TelemetryPayload.PowerTelemetry = power_telemetry_data;
    // print the size of minimal_tm_packet with OS_print
    OS_printf("Size of minimal_tm_packet: %zu bytes\n", sizeof(minimal_tm_packet));

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

        COMMMC_AppData.ErrCounter++;

        return CFE_SEVERITY_ERROR;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    if (file_size < 0) {
        CFE_EVS_SendEvent(COMMMC_FILE_SIZE_ERR_EID, CFE_EVS_EventType_ERROR,
                            "COMMMC: Error getting file size for %s", file_path);

        COMMMC_AppData.ErrCounter++;

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

        COMMMC_AppData.ErrCounter++;

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
    status = COMMMC_APP_SEND_DATA_TO_GROUND(COMMMC_APP_GROUND_ENDPOINT, (const unsigned char *)&file_transfer_init_packet, sizeof(file_transfer_init_packet));
    if (status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(COMMMC_FILE_SEND_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "COMMMC: Error sending file transfer init packet to ground");

        COMMMC_AppData.ErrCounter++;

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

                COMMMC_AppData.ErrCounter++;

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
    status = COMMMC_APP_SEND_DATA_TO_GROUND(COMMMC_APP_GROUND_ENDPOINT, (const unsigned char *)&file_transfer_packet, sizeof(file_transfer_packet));

        if (status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(COMMMC_FILE_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                                "COMMMC: Error sending file transfer packet to ground, counter: %u, status: %d", counter, status);

            COMMMC_AppData.ErrCounter++;   
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
    status = COMMMC_APP_SEND_DATA_TO_GROUND(COMMMC_APP_GROUND_ENDPOINT, (const unsigned char *)&file_transfer_packet, sizeof(file_transfer_packet));

    fclose(file);

    if (status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(COMMMC_FILE_SEND_LAST_ERR_EID, CFE_EVS_EventType_ERROR,
                          "COMMMC: Error sending last file transfer packet to ground, counter: %u, status: %d", counter, status);

        COMMMC_AppData.ErrCounter++;

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
    // This function supports two endpoint types:
    // 1) Serial device path like "/dev/ttyUSB0"
    // 2) TCP endpoint in the form "tcp://<host>:<port>" (e.g., tcp://127.0.0.1:5000)
    CFE_Status_t status = CFE_SUCCESS;

    if (port && (strncmp(port, "tcp://", 6) == 0 || strncmp(port, "TCP://", 6) == 0))
    {
        // Parse host and port
        const char *addr = port + 6; // skip tcp://
        const char *colon = strrchr(addr, ':');
        if (!colon || colon == addr || *(colon + 1) == '\0')
        {
            return 1; // invalid endpoint format
        }

        char host[256];
        char port_str[16];
        size_t host_len = (size_t)(colon - addr);
        if (host_len >= sizeof(host)) host_len = sizeof(host) - 1;
        memcpy(host, addr, host_len);
        host[host_len] = '\0';
        strncpy(port_str, colon + 1, sizeof(port_str) - 1);
        port_str[sizeof(port_str) - 1] = '\0';

        // Convert port to number and validate range
        char *endptr = NULL;
        long port_num_long = strtol(port_str, &endptr, 10);
        if (endptr == port_str || *endptr != '\0' || port_num_long <= 0 || port_num_long > 65535)
        {
            return 1; // invalid port
        }
        uint16_t port_num = (uint16_t)port_num_long;

        // Build IPv4 socket address
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
        {
            return 1;
        }

        struct sockaddr_in sa;
        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_port = htons(port_num);

        // Try numeric IPv4 first
        if (inet_pton(AF_INET, host, &sa.sin_addr) != 1)
        {
            // Fallback to DNS resolution (IPv4 only)
            struct hostent *he = gethostbyname(host);
            if (!he || he->h_addrtype != AF_INET || he->h_length != 4)
            {
                close(sock);
                return 1;
            }
            memcpy(&sa.sin_addr, he->h_addr_list[0], (size_t)he->h_length);
        }

        if (connect(sock, (struct sockaddr *)&sa, sizeof(sa)) != 0)
        {
            close(sock);
            return 1;
        }

        // Send all data (handle partial sends)
        size_t total = 0;
        while (total < length)
        {
            ssize_t n = send(sock, data + total, length - total, 0);
            if (n < 0)
            {
                if (errno == EINTR)
                    continue;
                close(sock);
                return 1;
            }
            if (n == 0)
            {
                // Connection closed prematurely
                close(sock);
                return 1;
            }
            total += (size_t)n;
        }

        close(sock);
        return status;
    }
    else
    {
        // Fallback to serial device path
        int fd = open(port, O_WRONLY | O_NOCTTY);
        if (fd < 0) return 1;

        struct termios tty;
        if (tcgetattr(fd, &tty) != 0)
        {
            close(fd);
            return 1;
        }

        tty.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
        tty.c_iflag = 0;
        tty.c_oflag = 0;
        tty.c_lflag = 0;

        if (tcsetattr(fd, TCSANOW, &tty) != 0)
        {
            close(fd);
            return 1;
        }

        // Write may be partial on some systems; loop until all data is written
        size_t total = 0;
        while (total < length)
        {
            ssize_t n = write(fd, data + total, length - total);
            if (n < 0)
            {
                if (errno == EINTR)
                    continue;
                close(fd);
                return 1;
            }
            total += (size_t)n;
        }

        close(fd);
        return status;
    }
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