/**
 * test_bury_rules.c - unit tests of the Remix bury timer (ft/ftremix/ftremixburyrules.c).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixburyrules.h"

static void testTimerRoundsToNearestLikeCvtWS(void)
{
    CHECK_EQ(99, ftRemixBuryRulesTimer(0));
    CHECK_EQ(100, ftRemixBuryRulesTimer(1));
    CHECK_EQ(100, ftRemixBuryRulesTimer(2));
    CHECK_EQ(101, ftRemixBuryRulesTimer(3));
    CHECK_EQ(102, ftRemixBuryRulesTimer(4));
    CHECK_EQ(169, ftRemixBuryRulesTimer(100));
}

static void testHalfwayProductRoundsToEven(void)
{
    CHECK_EQ(103, ftRemixBuryRulesTimer(5));
    CHECK_EQ(109, ftRemixBuryRulesTimer(15));
}

void remixTestBuryRules(void)
{
    printf("bury rules\n");
    RUN_TEST(testTimerRoundsToNearestLikeCvtWS);
    RUN_TEST(testHalfwayProductRoundsToEven);
}
