/**
 * Conditionals_post.h
 * Definições internas que dependem das configurações (Configuration.h/adv.h) e dos pinos (pins.h),
 * mas que **não devem ser editadas pelo usuário**.
 */

...

#ifndef BED_COUNT
  #define BED_COUNT ( \
      (TEMP_SENSOR_BED0 > 0 ? 1 : 0) + \
      (TEMP_SENSOR_BED1 > 0 ? 1 : 0) + \
      (TEMP_SENSOR_BED2 > 0 ? 1 : 0) + \
      (TEMP_SENSOR_BED3 > 0 ? 1 : 0) )
#endif

#if BED_COUNT > 1
  #define HAS_MULTI_BEDS 1
#else
  #define HAS_MULTI_BEDS 0
#endif  


