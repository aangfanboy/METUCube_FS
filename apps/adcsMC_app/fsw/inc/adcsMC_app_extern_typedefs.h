/**
 *
 * @file adcsMC_app_extern_typedefs.h
 * 
 * @brief AdcsMC Application Extern Typedefs
 * 
 * Declarations and definitions for AdcsMC application specific extern typedefs
 */

 #ifndef ADCSMC_EXTERN_TYPEDEFS_H
 #define ADCSMC_EXTERN_TYPEDEFS_H

 typedef struct
 {
     float quaternion[4];  /**< Quaternion representing the attitude */
     float angular_velocity[3];  /**< Angular velocity vector */
     float speed_vector[3];  /**< Speed vector */
     float position_vector[3];  /**< Position vector */
     uint32 rw_speeds[3];  /**< Reaction wheel speeds */
 } AdcsMC_MinimalTelemetry_t;
 
 #endif /* ADCSMC_EXTERN_TYPEDEFS_H */