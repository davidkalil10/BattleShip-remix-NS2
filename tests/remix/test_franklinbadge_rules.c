/**
 * test_franklinbadge_rules.c - unit tests of the Franklin Badge countdown rules (it/itremix/itremixfranklinbadgerules.c).
 */

#include "test_check.h"

#include "it/itremix/itremixfranklinbadgerules.h"

#define FLAGS_FOX_REFLECTOR 0x0400
#define FLAGS_FASTFALL 0x0008

static void testBadgeHitboxKeepsTheReflectFlagOn(void)
{
    CHECK_EQ(nITRemixFranklinBadgeMaintainReflect, itRemixFranklinBadgeRulesGetMaintain(1, 0));
    CHECK_EQ(nITRemixFranklinBadgeMaintainReflect, itRemixFranklinBadgeRulesGetMaintain(1, FLAGS_FOX_REFLECTOR));
}

static void testActiveReflectorOrAbsorberKeepsItsHitbox(void)
{
    CHECK_EQ(nITRemixFranklinBadgeMaintainNone, itRemixFranklinBadgeRulesGetMaintain(0, FLAGS_FOX_REFLECTOR));
    CHECK_EQ(nITRemixFranklinBadgeMaintainNone, itRemixFranklinBadgeRulesGetMaintain(0, FLAGS_FOX_REFLECTOR | FLAGS_FASTFALL));
    CHECK_EQ(nITRemixFranklinBadgeMaintainNone, itRemixFranklinBadgeRulesGetMaintain(0, ITREMIX_FRANKLINBADGE_RULES_FLAGS_ABSORB));
}

static void testNessFlagLosesItsReflectBit(void)
{
    CHECK_EQ(nITRemixFranklinBadgeMaintainDropReflect, itRemixFranklinBadgeRulesGetMaintain(0, ITREMIX_FRANKLINBADGE_RULES_FLAGS_NESS));
}

static void testBadgeTakesTheHitboxBackWhenTheOtherMoveEnded(void)
{
    CHECK_EQ(nITRemixFranklinBadgeMaintainRestore, itRemixFranklinBadgeRulesGetMaintain(0, 0));
    CHECK_EQ(nITRemixFranklinBadgeMaintainRestore, itRemixFranklinBadgeRulesGetMaintain(0, FLAGS_FASTFALL));
    CHECK_EQ(nITRemixFranklinBadgeMaintainRestore, itRemixFranklinBadgeRulesGetMaintain(0, 0x0080));
}

static void testExpiryLeavesAnotherReflectorOn(void)
{
    CHECK_EQ(1, itRemixFranklinBadgeRulesKeepsReflectAtExpiry(nITRemixFranklinBadgeMaintainNone, FLAGS_FOX_REFLECTOR));
    CHECK_EQ(1, itRemixFranklinBadgeRulesKeepsReflectAtExpiry(nITRemixFranklinBadgeMaintainDropReflect, ITREMIX_FRANKLINBADGE_RULES_FLAGS_NESS));
}

static void testExpiryClearsTheBadgeFlag(void)
{
    CHECK_EQ(0, itRemixFranklinBadgeRulesKeepsReflectAtExpiry(nITRemixFranklinBadgeMaintainReflect, FLAGS_FOX_REFLECTOR));
    CHECK_EQ(0, itRemixFranklinBadgeRulesKeepsReflectAtExpiry(nITRemixFranklinBadgeMaintainNone, ITREMIX_FRANKLINBADGE_RULES_FLAGS_ABSORB));
}

static void testExpiryOnTheRestoreFrameClearsTheFlag(void)
{
    CHECK_EQ(0, itRemixFranklinBadgeRulesKeepsReflectAtExpiry(nITRemixFranklinBadgeMaintainRestore, 0));
}

void remixTestFranklinBadgeRules(void)
{
    RUN_TEST(testBadgeHitboxKeepsTheReflectFlagOn);
    RUN_TEST(testActiveReflectorOrAbsorberKeepsItsHitbox);
    RUN_TEST(testNessFlagLosesItsReflectBit);
    RUN_TEST(testBadgeTakesTheHitboxBackWhenTheOtherMoveEnded);
    RUN_TEST(testExpiryLeavesAnotherReflectorOn);
    RUN_TEST(testExpiryClearsTheBadgeFlag);
    RUN_TEST(testExpiryOnTheRestoreFrameClearsTheFlag);
}
