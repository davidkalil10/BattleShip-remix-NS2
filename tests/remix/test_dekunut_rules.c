/**
 * test_dekunut_rules.c - unit tests of the Deku Nut knockback scale store (it/itremix/itremixdekunutrules.c).
 */

#include "test_check.h"

#include "it/itremix/itremixdekunutrules.h"

static void testHalfwordStoreReplacesTheHighHalfOfTheScale(void)
{
    CHECK_EQ(0x01000000, itRemixDekuNutRulesKnockbackScale(0));
    CHECK_EQ(0x01000064, itRemixDekuNutRulesKnockbackScale(100));
    CHECK_EQ(0x0100FFFF, itRemixDekuNutRulesKnockbackScale(0xABCDFFFF));
}

static void testSecondStoreKeepsTheValue(void)
{
    uint32_t spawn_scale = itRemixDekuNutRulesKnockbackScale(100);

    CHECK_EQ(spawn_scale, itRemixDekuNutRulesKnockbackScale(spawn_scale));
}

static void testScaleIsNotThePlain256(void)
{
    CHECK_TRUE(itRemixDekuNutRulesKnockbackScale(0) != 0x0100);
}

void remixTestDekuNutRules(void)
{
    RUN_TEST(testHalfwordStoreReplacesTheHighHalfOfTheScale);
    RUN_TEST(testSecondStoreKeepsTheValue);
    RUN_TEST(testScaleIsNotThePlain256);
}
