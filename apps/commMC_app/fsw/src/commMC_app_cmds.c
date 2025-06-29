
#include "commMC_app.h"
#include "commMC_app_cmds.h"
#include "commMC_app_msgids.h"

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
    const unsigned char data[] = "Minimal Telemetry Data\n"; // Example data, adjust as necessary
    status = COMMMC_APP_SEND_DATA_TO_GROUND(port, data, sizeof(data));

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
