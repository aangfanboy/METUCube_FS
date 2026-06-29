/**
 *
 * @file canIOMC_app_extern_typedefs.h
 * 
 * @brief CANIOMC Application Extern Typedefs
 * 
 * Declarations and definitions for CANIOMC application specific extern typedefs
 */

 #ifndef CANIOMC_EXTERN_TYPEDEFS_H
 #define CANIOMC_EXTERN_TYPEDEFS_H

  typedef struct
 {
     uint32 batteryPercentage; /**< \brief Battery percentage of the power subsystem */
     uint32 batteryHealth; /**< \brief Battery health status */
 } CANIOMC_MinimalTelemetry_t;
 
 #endif /* CANIOMC_EXTERN_TYPEDEFS_H */