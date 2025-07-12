/**
 *
 * @file payloadMC_app_extern_typedefs.h
 * 
 * @brief PayloadMC Application Extern Typedefs
 * 
 * Declarations and definitions for PayloadMC application specific extern typedefs
 */

 #ifndef PAYLOADMC_EXTERN_TYPEDEFS_H
 #define PAYLOADMC_EXTERN_TYPEDEFS_H

 typedef struct
 {
     uint32 batteryPercentage; /**< \brief Battery percentage of the payload */
 } PayloadMC_MinimalTelemetry_t;
 
 #endif /* PAYLOADMC_EXTERN_TYPEDEFS_H */