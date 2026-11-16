/**
 * test_basketball_bounce_rules.c - unit tests of the Basketball bounce hitlag (it/itremix/itremixbasketballrules.c).
 */

#include "test_check.h"

#include "it/itremix/itremixbasketballrules.h"

static void testHitlagRoundsToNearestInsteadOfTruncating(void)
{
    CHECK_EQ(3, itRemixBasketballRulesBounceHitlag(51.15F));
    CHECK_EQ(2, itRemixBasketballRulesBounceHitlag(30.195F));
    CHECK_EQ(3, itRemixBasketballRulesBounceHitlag(50.0F));
}

static void testHitlagBelowHalfKeepsTheTruncatedValue(void)
{
    CHECK_EQ(2, itRemixBasketballRulesBounceHitlag(42.322F));
    CHECK_EQ(2, itRemixBasketballRulesBounceHitlag(34.976F));
    CHECK_EQ(1, itRemixBasketballRulesBounceHitlag(13.49F));
}

static void testStillBallGetsOneFrame(void)
{
    CHECK_EQ(1, itRemixBasketballRulesBounceHitlag(0.0F));
}

void remixTestBasketballBounceRules(void)
{
    RUN_TEST(testHitlagRoundsToNearestInsteadOfTruncating);
    RUN_TEST(testHitlagBelowHalfKeepsTheTruncatedValue);
    RUN_TEST(testStillBallGetsOneFrame);
}
