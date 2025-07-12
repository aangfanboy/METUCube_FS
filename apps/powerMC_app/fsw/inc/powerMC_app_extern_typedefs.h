/**
 *
 * @file powerMC_app_extern_typedefs.h
 * 
 * @brief PowerMC Application Extern Typedefs
 * 
 * Declarations and definitions for PowerMC application specific extern typedefs
 */

 #ifndef POWERMC_EXTERN_TYPEDEFS_H
 #define POWERMC_EXTERN_TYPEDEFS_H

  typedef struct
 {
     uint32 batteryPercentage; /**< \brief Battery percentage of the power subsystem */
     uint32 batteryHealth; /**< \brief Battery health status */
 } PowerMC_MinimalTelemetry_t;
 
 #endif /* POWERMC_EXTERN_TYPEDEFS_H */