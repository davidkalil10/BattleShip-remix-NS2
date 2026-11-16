/**
 * test_crash_bonus_rules.c - unit tests of the 1-up easter egg of Crash's bonus stages (gr/grremix/grremixcrashbonusrules.c,
 * Hazards.asm crash_btt_main and crash_btp_main).
 */

#include "test_check.h"

#include "gr/grremix/grremixcrashbonusrules.h"
#include "gr/grremix/grremixids.h"

#include <stddef.h>

static void testCrashBreakTheTargetsWatchesLineNineAndDObjFive(void)
{
    const GRRemixCrashBonusOneUp *one_up = grRemixCrashBonusRulesFind(nGRRemixStage_BTT_CRASH);

    CHECK_TRUE(one_up != NULL);
    CHECK_EQ(9, one_up->line_id);
    CHECK_EQ(5, one_up->dobj_index);
}

static void testCrashBoardThePlatformsWatchesLine0x16AndDObjFifteen(void)
{
    const GRRemixCrashBonusOneUp *one_up = grRemixCrashBonusRulesFind(nGRRemixStage_BTP_CRASH);

    CHECK_TRUE(one_up != NULL);
    CHECK_EQ(0x16, one_up->line_id);
    CHECK_EQ(15, one_up->dobj_index);
}

static void testOtherBonusStagesHaveNoOneUp(void)
{
    CHECK_TRUE(grRemixCrashBonusRulesFind(nGRRemixStage_BTT_PEACH) == NULL);
    CHECK_TRUE(grRemixCrashBonusRulesFind(nGRRemixStage_BTP_SONIC) == NULL);
    CHECK_TRUE(grRemixCrashBonusRulesFind(nGRRemixStage_REMIX_RTTF) == NULL);
}

static void testOnlyTheOneUpLineCollectsIt(void)
{
    const GRRemixCrashBonusOneUp *one_up = grRemixCrashBonusRulesFind(nGRRemixStage_BTT_CRASH);

    CHECK_EQ(1, grRemixCrashBonusRulesIsCollected(one_up, 9));
    CHECK_EQ(0, grRemixCrashBonusRulesIsCollected(one_up, 8));
    CHECK_EQ(0, grRemixCrashBonusRulesIsCollected(one_up, -1));
}

void remixTestCrashBonusRules(void)
{
    printf("crash bonus rules\n");
    RUN_TEST(testCrashBreakTheTargetsWatchesLineNineAndDObjFive);
    RUN_TEST(testCrashBoardThePlatformsWatchesLine0x16AndDObjFifteen);
    RUN_TEST(testOtherBonusStagesHaveNoOneUp);
    RUN_TEST(testOnlyTheOneUpLineCollectsIt);
}
