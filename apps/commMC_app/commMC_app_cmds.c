
#include "commMC_app.h"
#include "commMC_app_cmds.h"
#include "commMC_config_msgids.h"

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

/************************************************************************
 * Local function prototypes
 ************************************************************************/
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SAMPLE NOOP commands                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t COMMMC_APP_NoopCmd(const COMMMC_APP_NoopCmd_t *Msg)
{
    COMMMC_AppData.CmdCounter++;

    CFE_EVS_SendEvent(COMMMC_APP_MSG_SENT_EID, CFE_EVS_EventType_INFORMATION, "COMMMC: NOOP command");

    return CFE_SUCCESS;
}

CFE_Status_t COMMMC_APP_SEND_HK_TO_SB(const COMMMC_APP_SendDataCmd_t *Msg)
{
    COMMMC_AppData.CmdCounter++;

    CFE_EVS_SendEvent(COMMMC_APP_MSG_SENT_EID, CFE_EVS_EventType_INFORMATION, "COMMMC: Send HK command");

    COMMMC_AppData.HkTlm.Payload.someCommDoubleValue = COMMMC_AppData.someCommDoubleValue;
    COMMMC_AppData.HkTlm.Payload.someCommIntValue = COMMMC_AppData.someCommIntValue;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(COMMMC_AppData.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(COMMMC_AppData.HkTlm.TelemetryHeader), true);

    CFE_EVS_SendEvent(COMMMC_APP_MSG_SENT_EID, CFE_EVS_EventType_INFORMATION, "COMMMC: Double Value: %f, Int Value: %d", COMMMC_AppData.HkTlm.Payload.someCommDoubleValue, COMMMC_AppData.HkTlm.Payload.someCommIntValue);
    return CFE_SUCCESS;
}

CFE_Status_t COMMMC_APP_SEND_DATA_TO_IP()
{
    COMMMC_AppData.CmdCounter++;

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        OS_printf("Socket creation failed");
        return CFE_ERROR;
    }

    // Configure server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) { // Replace with server IP
        OS_printf("Invalid address");
        return CFE_ERROR;
    }

    // Connect to server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        OS_printf("Connection failed");
        return CFE_ERROR;
    }

    printf("Connected to server!\n");

    // Send a message to server
    const char* msg = "Hello from C client!";
    send(client_socket, msg, strlen(msg), 0);
    printf("Sent message to server.\n");

    close(client_socket);
    return CFE_SUCCESS;

}

