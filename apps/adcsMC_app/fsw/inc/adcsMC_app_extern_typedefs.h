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
     double quaternion[4];  /**< Quaternion representing the attitude */
     double angular_velocity[3];  /**< Angular velocity vector */
     double speed_vector[3];  /**< Speed vector */
     double position_vector[3];  /**< Position vector */
 } AdcsMC_MinimalTelemetry_t;
 
 #endif /* ADCSMC_EXTERN_TYPEDEFS_H */