#ifndef POWERMC_APP_CONFIG_H
#define POWERMC_APP_CONFIG_H

#define POWERMC_APP_PIPE_DEPTH         12
#define POWERMC_APP_PERF_ID            0x64

#define POWERMC_APP_INIT_INF_EID 1
#define POWERMC_APP_INIT_ERR_EID 2
#define POWERMC_APP_PIPE_ERR_EID 3
#define POWERMC_APP_HK_SEND_ERR_EID 4
#define POWERMC_APP_MID_ERR_EID 5
#define POWERMC_APP_MSG_RECEIVED_EID 6
#define POWERMC_APP_MSG_SENT_EID 7
#define POWERMC_APP_CMD_ERR_EID 8

#define CFE_MISSION_POWERMC_APP_HK_TLM_TOPICID    0xA1
#define CFE_MISSION_POWERMC_APP_SEND_HK_TOPICID   0xB1
#define CFE_MISSION_POWERMC_APP_CMD_TOPICID       0xC1

#define POWERMC_APP_HK_TLM_MID  CFE_PLATFORM_TLM_TOPICID_TO_MIDV(CFE_MISSION_POWERMC_APP_HK_TLM_TOPICID)
#define POWERMC_APP_SEND_HK_MID  CFE_PLATFORM_CMD_TOPICID_TO_MIDV(CFE_MISSION_POWERMC_APP_SEND_HK_TOPICID)
#define POWERMC_APP_CMD_MID      CFE_PLATFORM_CMD_TOPICID_TO_MIDV(CFE_MISSION_POWERMC_APP_CMD_TOPICID)


#endif /* POWERMC_APP_CONFIG_H */