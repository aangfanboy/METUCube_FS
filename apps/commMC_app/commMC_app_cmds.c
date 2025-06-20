
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

CFE_Status_t COMMMC_APP_SEND_DATA_TO_IP(void)
{
    COMMMC_AppData.CmdCounter++;

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        OS_printf("Socket creation failed");
        return -1;
    }

    // Configure server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    if (inet_pton(AF_INET, "192.168.1.111188", &server_addr.sin_addr) <= 0) { // Replace with server IP
        OS_printf("Invalid address");
        return -1;
    }

    // Connect to server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        OS_printf("Connection failed");
        return -1;
    }

    printf("Connected to server!\n");

    // Send a message to server
    const char* msg = "Hello from C client!";
    send(client_socket, msg, strlen(msg), 0);
    printf("Sent message to server.\n");

    close(client_socket);
    return CFE_SUCCESS;
}

CFE_Status_t COMMMC_APP_SEND_PING_WAIT_ANSWER(char *buffer, size_t buffer_size)
{
    COMMMC_AppData.CmdCounter++;

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        OS_printf("Socket creation failed");
        return -1;
    }

    // Configure server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    if (inet_pton(AF_INET, "192.168.1.88", &server_addr.sin_addr) <= 0) {
        OS_printf("Invalid address");
        return CFE_TIME_BAD_ARGUMENT;
    }
    // Connect to server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        OS_printf("Connection failed");
        return CFE_TIME_BAD_ARGUMENT;
    }
    printf("Connected to server!\n");

    int pipeResult;
    
    while (1) {
        // receive data from server
        ssize_t bytes_received = recv(client_socket, buffer, buffer_size - 1, 0);
        if (bytes_received < 0) {
            OS_printf("Receive failed");
            close(client_socket);
            return -1;
        } else if (bytes_received == 0) {
            OS_printf("Server closed connection");
            break;
        }

        buffer[bytes_received] = '\0'; // Null-terminate the received string
        printf("Received data: %s\n", buffer);

        pipeResult = COMMMC_APP_SEND_MESSAGE_TO_TASKPIPE(client_socket, buffer);

        if (pipeResult == 0) {
            break; // Exit command received
        } else if (pipeResult == -1) {
            OS_printf("Error sending message to server");
            break;
        }

    }

    return CFE_SUCCESS;
}

// create a function that sends a message to the server given a message and a socket
int COMMMC_APP_SEND_MESSAGE_TO_TASKPIPE(int client_socket, const char* msg)
{
    COMMMC_AppData.CmdCounter++;

    // check given message for options such as sendHK, exit, etc.
    if (strcmp(msg, "sendHK") == 0) {
        // send HK message
        const char* hk_msg = "This is a HK message";
        send(client_socket, hk_msg, strlen(hk_msg), 0);
        printf("Sent HK to server.\n");
    } else if (strcmp(msg, "exit") == 0) {
        printf("Exit command received. Closing connection.\n");
        const char* exit_msg = "exitApproved";
        send(client_socket, exit_msg, strlen(exit_msg), 0);
        printf("Sent exitApproved to server.\n");
        return 0;
    } else {
        // send the message
        send(client_socket, msg, strlen(msg), 0);
        printf("Sent message to server: %s\n", msg);
    }

    return 1;
}
