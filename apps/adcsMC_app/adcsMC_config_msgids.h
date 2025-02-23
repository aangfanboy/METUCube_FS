#ifndef ADCSMC_APP_CONFIG_H
#define ADCSMC_APP_CONFIG_H

#define ADCSMC_APP_PIPE_DEPTH         12
#define ADCSMC_APP_PERF_ID            0x64

#define ADCSMC_APP_INIT_INF_EID 1
#define ADCSMC_APP_INIT_ERR_EID 2
#define ADCSMC_APP_PIPE_ERR_EID 3
#define ADCSMC_APP_HK_SEND_ERR_EID 4
#define ADCSMC_APP_MID_ERR_EID 5
#define ADCSMC_APP_MSG_RECEIVED_EID 6
#define ADCSMC_APP_MSG_SENT_EID 7
#define ADCSMC_APP_CMD_ERR_EID 8

#define CFE_MISSION_ADCSMC_APP_HK_TLM_TOPICID    0xA3
#define CFE_MISSION_ADCSMC_APP_SEND_HK_TOPICID   0xB3
#define CFE_MISSION_ADCSMC_APP_CMD_TOPICID       0xC3

#define ADCSMC_APP_HK_TLM_MID  CFE_PLATFORM_TLM_TOPICID_TO_MIDV(CFE_MISSION_ADCSMC_APP_HK_TLM_TOPICID)
#define ADCSMC_APP_SEND_HK_MID  CFE_PLATFORM_CMD_TOPICID_TO_MIDV(CFE_MISSION_ADCSMC_APP_SEND_HK_TOPICID)
#define ADCSMC_APP_CMD_MID      CFE_PLATFORM_CMD_TOPICID_TO_MIDV(CFE_MISSION_ADCSMC_APP_CMD_TOPICID)


#endif /* ADCSMC_APP_CONFIG_H */