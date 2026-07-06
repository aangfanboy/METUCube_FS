/**
 *
 * @file mpptMC_app_extern_typedefs.h
 *
 * @brief MpptMC Application Extern Typedefs
 *
 * Declarations and definitions for MpptMC application specific extern typedefs
 */

 #ifndef MPPTMC_EXTERN_TYPEDEFS_H
 #define MPPTMC_EXTERN_TYPEDEFS_H

  typedef struct
 {
     uint32 outputPowerMilliwatts; /**< \brief Total MPPT output power */
     uint32 mpptHealth; /**< \brief MPPT health status */
 } MpptMC_MinimalTelemetry_t;

 #endif /* MPPTMC_EXTERN_TYPEDEFS_H */