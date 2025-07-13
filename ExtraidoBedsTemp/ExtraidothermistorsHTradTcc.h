/*thermistors.h*/

#if ENABLED(HAL_ADC_FILTERED)
  #define OVERSAMPLENR 1
#else
  #define OVERSAMPLENR 16
#endif

...

#if ANY_THERMISTOR_IS(1) // beta25 = 4092 K, R25 = 100 kOhm, Pull-up = 4.7 kOhm, "EPCOS"
  #include "thermistor_1.h"
#endif
#if ANY_THERMISTOR_IS(133) // beta25 = 4092 K, R25 = 100 kOhm, Pull-up = 4.7 kOhm, "EPCOS"
  #undef OV
  #define OV(N) (N)
  #include "thermistor_133.h"
#endif
...