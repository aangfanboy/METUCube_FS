/**
 *
 * @file adcsttMC_app_extern_typedefs.h
 * 
 * @brief AdcsttMC Application Extern Typedefs
 * 
 * Declarations and definitions for AdcsttMC application specific extern typedefs
 */

 #ifndef ADCSTTMC_EXTERN_TYPEDEFS_H
 #define ADCSTTMC_EXTERN_TYPEDEFS_H

  typedef struct
 {
     float quaternion[4];  /**< Quaternion representing the attitude */
     float angular_velocity[3];  /**< Angular velocity vector */
     float speed_vector[3];  /**< Speed vector */
     float position_vector[3];  /**< Position vector */
     uint32 rw_speeds[3];  /**< Reaction wheel speeds */
 } AdcsttMC_MinimalTelemetry_t;
 
 #endif /* ADCSTTMC_EXTERN_TYPEDEFS_H */