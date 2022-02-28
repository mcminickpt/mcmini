#ifndef DPOR_FAIL_H
#define DPOR_FAIL_H

#include <assert.h>

#define mc_assert(__X) \
do {                   \
     assert(__X);      \
} while(0)             \

#define mc_assert_main_thread(message)
#define mc_report_undefined_behavior(__X, ...)


#endif //DPOR_FAIL_H
