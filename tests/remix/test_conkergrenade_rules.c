/**
 * test_conkergrenade_rules.c - unit tests of the Conker grenade fuse cut (it/itremix/itremixconkergrenaderules.c).
 */

#include "test_check.h"

#include "it/itremix/itremixconkergrenaderules.h"

static void testFuseCutRoundsTheSpeedToNearest(void)
{
    CHECK_EQ(100, itRemixConkerGrenadeRulesHurtFuse(130, 19.9615F));
    CHECK_EQ(121, itRemixConkerGrenadeRulesHurtFuse(149, 18.899F));
}

static void testFuseCutBelowHalfKeepsTheWholeSpeed(void)
{
    CHECK_EQ(102, itRemixConkerGrenadeRulesHurtFuse(130, 19.4F));
    CHECK_EQ(130, itRemixConkerGrenadeRulesHurtFuse(130, 0.0F));
}

static void testFuseNeverGoesBelowTwelveFrames(void)
{
    CHECK_EQ(12, itRemixConkerGrenadeRulesHurtFuse(30, 40.0F));
    CHECK_EQ(12, itRemixConkerGrenadeRulesHurtFuse(5, 0.0F));
}

void remixTestConkerGrenadeRules(void)
{
    RUN_TEST(testFuseCutRoundsTheSpeedToNearest);
    RUN_TEST(testFuseCutBelowHalfKeepsTheWholeSpeed);
    RUN_TEST(testFuseNeverGoesBelowTwelveFrames);
}
