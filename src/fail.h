#ifndef DPOR_FAIL_H
#define DPOR_FAIL_H

#include <assert.h>

#define mc_assert(__X) \
do {                   \
     assert(__X);      \
} while(0);            \


#endif //DPOR_FAIL_H
