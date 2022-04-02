
#include "unity.h"

extern int32_t getMicroVoltsADC(uint32_t ADCCode);

void setUp(void)
{

}

void tearDown(void)
{
}

void test_getMicroVoltsADC(void)
{
   TEST_ASSERT_EQUAL(0, getMicroVoltsADC(0));
   TEST_ASSERT_EQUAL(5*1e6, getMicroVoltsADC(0x7fffff));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_getMicroVoltsADC);
    return UNITY_END();
}

