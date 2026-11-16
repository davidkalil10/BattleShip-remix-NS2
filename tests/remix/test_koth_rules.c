/**
 * test_koth_rules.c - unit tests of the King of the Hill rules (gm/gmremixkothrules.c): king, win timer, meters, flash, hill pick.
 */

#include "test_check.h"

#include "gm/gmremixkothrules.h"

#define BUFFER 30

static void testAnEmptyHillHasNoKing(void)
{
    GMRemixKothRulesFighter fighters[2] = { { 0, 0, 0 }, { 0, 0, 1 } };
    int eligible[2];

    CHECK_EQ(GMREMIX_KOTHRULES_NO_KING, gmRemixKothRulesCandidate(fighters, 2, eligible));
    CHECK_EQ(0, eligible[0]);
}

static void testOneFighterOnTheHillIsTheCandidate(void)
{
    GMRemixKothRulesFighter fighters[2] = { { 0, 0, 0 }, { 1, 0, 1 } };
    int eligible[2];

    CHECK_EQ(1, gmRemixKothRulesCandidate(fighters, 2, eligible));
    CHECK_EQ(1, eligible[1]);
}

static void testTeammatesShareTheHill(void)
{
    GMRemixKothRulesFighter fighters[3] = { { 1, 0, 2 }, { 1, 0, 2 }, { 0, 0, 0 } };
    int eligible[3];

    CHECK_EQ(2, gmRemixKothRulesCandidate(fighters, 3, eligible));
    CHECK_EQ(1, eligible[0]);
    CHECK_EQ(1, eligible[1]);
}

static void testTwoTeamsOnTheHillContestIt(void)
{
    GMRemixKothRulesFighter fighters[2] = { { 1, 0, 0 }, { 1, 0, 1 } };
    int eligible[2];

    CHECK_EQ(GMREMIX_KOTHRULES_CONTESTED, gmRemixKothRulesCandidate(fighters, 2, eligible));
}

static void testHoldingAnEnemyContestsTheHillAndLosesTheClaim(void)
{
    GMRemixKothRulesFighter fighters[2] = { { 1, 1, 0 }, { 0, 0, 1 } };
    int eligible[2];

    CHECK_EQ(GMREMIX_KOTHRULES_CONTESTED, gmRemixKothRulesCandidate(fighters, 2, eligible));
    CHECK_EQ(0, eligible[0]);
}

static void testAContestStartsTheBuffer(void)
{
    int buffer = 0;

    CHECK_EQ(GMREMIX_KOTHRULES_CONTESTED, gmRemixKothRulesSettle(GMREMIX_KOTHRULES_CONTESTED, &buffer, BUFFER));
    CHECK_EQ(BUFFER, buffer);
}

static void testTheBufferHoldsTheHillUntilItRunsOut(void)
{
    int buffer = 2;

    CHECK_EQ(GMREMIX_KOTHRULES_CONTESTED, gmRemixKothRulesSettle(1, &buffer, BUFFER));
    CHECK_EQ(GMREMIX_KOTHRULES_CONTESTED, gmRemixKothRulesSettle(1, &buffer, BUFFER));
    CHECK_EQ(1, gmRemixKothRulesSettle(1, &buffer, BUFFER));
}

static void testMeterSitsAtTheBottomUnlessTheDragonKingHudApplies(void)
{
    float frame_y;
    float bar_y;

    gmRemixKothRulesMeterY(GMREMIX_KOTHRULES_DK_HUD_DEFAULT, 0, &frame_y, &bar_y);
    CHECK_EQ(180, (int)frame_y);
    CHECK_EQ(188, (int)bar_y);
    gmRemixKothRulesMeterY(GMREMIX_KOTHRULES_DK_HUD_DEFAULT, 1, &frame_y, &bar_y);
    CHECK_EQ(8, (int)frame_y);
    CHECK_EQ(16, (int)bar_y);
    gmRemixKothRulesMeterY(GMREMIX_KOTHRULES_DK_HUD_ALWAYS, 0, &frame_y, &bar_y);
    CHECK_EQ(16, (int)bar_y);
    gmRemixKothRulesMeterY(GMREMIX_KOTHRULES_DK_HUD_OFF, 1, &frame_y, &bar_y);
    CHECK_EQ(188, (int)bar_y);
}

static void testMeterXFollowsThePort(void)
{
    CHECK_EQ(40, (int)gmRemixKothRulesMeterX(0));
    CHECK_EQ(110, (int)gmRemixKothRulesMeterX(1));
    CHECK_EQ(250, (int)gmRemixKothRulesMeterX(3));
}

static void testFfaTagsFollowTheColorOrderOfTheAsmAndACpuIsGray(void)
{
    CHECK_EQ(GMREMIX_KOTHRULES_TAG_RED, gmRemixKothRulesTag(0, 0, 0));
    CHECK_EQ(GMREMIX_KOTHRULES_TAG_BLUE, gmRemixKothRulesTag(1, 0, 0));
    CHECK_EQ(GMREMIX_KOTHRULES_TAG_YELLOW, gmRemixKothRulesTag(2, 0, 0));
    CHECK_EQ(GMREMIX_KOTHRULES_TAG_GREEN, gmRemixKothRulesTag(3, 0, 0));
    CHECK_EQ(GMREMIX_KOTHRULES_TAG_GRAY, gmRemixKothRulesTag(2, 0, 1));
}

static void testTeamTagsUseTheTeamColorOrderAndNeverGray(void)
{
    CHECK_EQ(GMREMIX_KOTHRULES_TAG_RED, gmRemixKothRulesTag(0, 1, 0));
    CHECK_EQ(GMREMIX_KOTHRULES_TAG_BLUE, gmRemixKothRulesTag(1, 1, 0));
    CHECK_EQ(GMREMIX_KOTHRULES_TAG_GREEN, gmRemixKothRulesTag(2, 1, 0));
    CHECK_EQ(GMREMIX_KOTHRULES_TAG_YELLOW, gmRemixKothRulesTag(3, 1, 1));
}

static void testTagColorsAreTheColorTagValuesOfTheAsm(void)
{
    CHECK_EQ(0xED3636FF, gmRemixKothRulesTagColor(GMREMIX_KOTHRULES_TAG_RED));
    CHECK_EQ(0x4E4EE9FF, gmRemixKothRulesTagColor(GMREMIX_KOTHRULES_TAG_BLUE));
    CHECK_EQ(0x4EB94EFF, gmRemixKothRulesTagColor(GMREMIX_KOTHRULES_TAG_GREEN));
    CHECK_EQ(0xFFDF1AFF, gmRemixKothRulesTagColor(GMREMIX_KOTHRULES_TAG_YELLOW));
    CHECK_EQ(0xACACACFF, gmRemixKothRulesTagColor(GMREMIX_KOTHRULES_TAG_GRAY));
}

static void testTheContestedGrayStartsDarkAndLightensAsTheBufferRunsOut(void)
{
    CHECK_EQ(0x6E, gmRemixKothRulesContestedGray(BUFFER));
    CHECK_EQ(0x8C, gmRemixKothRulesContestedGray(15));
    CHECK_EQ(0xA8, gmRemixKothRulesContestedGray(1));
}

static void testOnlyTheKingKeepsItsWinTimerAndNoKingResetsEveryTimer(void)
{
    CHECK_EQ(1, gmRemixKothRulesWinnerTimeKept(2, 2));
    CHECK_EQ(0, gmRemixKothRulesWinnerTimeKept(1, 2));
    CHECK_EQ(0, gmRemixKothRulesWinnerTimeKept(0, GMREMIX_KOTHRULES_NO_KING));
    CHECK_EQ(0, gmRemixKothRulesWinnerTimeKept(3, GMREMIX_KOTHRULES_CONTESTED));
}

static void testMeterFramesUseTheDamageIconColorsAndBarsTheDarkerOnes(void)
{
    CHECK_EQ(0xFFA0A0FF, gmRemixKothRulesMeterColor(GMREMIX_KOTHRULES_TAG_RED));
    CHECK_EQ(0xB4B4FFFF, gmRemixKothRulesMeterColor(GMREMIX_KOTHRULES_TAG_BLUE));
    CHECK_EQ(0xA0FFA0FF, gmRemixKothRulesMeterColor(GMREMIX_KOTHRULES_TAG_GREEN));
    CHECK_EQ(0xE6E678FF, gmRemixKothRulesMeterColor(GMREMIX_KOTHRULES_TAG_YELLOW));
    CHECK_EQ(0xDCDCDCFF, gmRemixKothRulesMeterColor(GMREMIX_KOTHRULES_TAG_GRAY));
    CHECK_EQ(0xC96A6AFF, gmRemixKothRulesBarColor(GMREMIX_KOTHRULES_TAG_RED));
    CHECK_EQ(0x7E7EC9FF, gmRemixKothRulesBarColor(GMREMIX_KOTHRULES_TAG_BLUE));
    CHECK_EQ(0x4EAD4EFF, gmRemixKothRulesBarColor(GMREMIX_KOTHRULES_TAG_GREEN));
    CHECK_EQ(0xB2B244FF, gmRemixKothRulesBarColor(GMREMIX_KOTHRULES_TAG_YELLOW));
    CHECK_EQ(0x888888FF, gmRemixKothRulesBarColor(GMREMIX_KOTHRULES_TAG_GRAY));
}

static void testAFlashStartsFromZeroAndBouncesBetweenZeroAndOneHundred(void)
{
    int alpha = 0;
    int delta = GMREMIX_KOTHRULES_FLASH_STEP;
    int i;

    gmRemixKothRulesFlashStep(&alpha, &delta);
    CHECK_EQ(5, alpha);

    for (i = 1; i < 20; i++)
    {
        gmRemixKothRulesFlashStep(&alpha, &delta);
    }
    CHECK_EQ(100, alpha);
    CHECK_EQ(-5, delta);

    for (i = 0; i < 20; i++)
    {
        gmRemixKothRulesFlashStep(&alpha, &delta);
    }
    CHECK_EQ(0, alpha);
    CHECK_EQ(5, delta);
}

typedef struct KothTestDraws
{
    const int *lines;
    int count;
    int next;

} KothTestDraws;

static int kothTestDrawLine(void *context)
{
    KothTestDraws *draws = (KothTestDraws *)context;
    int line = draws->lines[draws->next % draws->count];

    draws->next++;
    return line;
}

static void testInvalidLinesAreDrawnAgainWithoutUsingATry(void)
{
    const int lines[] = { -1, -1, -1, -1, -1, 4 };
    KothTestDraws draws = { lines, 6, 0 };

    CHECK_EQ(4, gmRemixKothRulesPickHill(2, kothTestDrawLine, &draws));
    CHECK_EQ(6, draws.next);
}

static void testTheFourthRepeatOfTheCurrentHillKeepsIt(void)
{
    const int lines[] = { 2 };
    KothTestDraws draws = { lines, 1, 0 };

    CHECK_EQ(2, gmRemixKothRulesPickHill(2, kothTestDrawLine, &draws));
    CHECK_EQ(4, draws.next);
}

static void testThreeRepeatsAndInvalidLinesStillReachANewHill(void)
{
    const int lines[] = { 2, -1, 2, -1, 2, 7 };
    KothTestDraws draws = { lines, 6, 0 };

    CHECK_EQ(7, gmRemixKothRulesPickHill(2, kothTestDrawLine, &draws));
}

static void testAStageWithNoValidLineKeepsTheHillInsteadOfHanging(void)
{
    const int lines[] = { -1 };
    KothTestDraws draws = { lines, 1, 0 };

    CHECK_EQ(-1, gmRemixKothRulesPickHill(-1, kothTestDrawLine, &draws));
    CHECK_EQ(GMREMIX_KOTHRULES_HILL_INVALID_MAX + 1, draws.next);
}

static void testAHiddenHillTimesOutOnItsHundredAndTwentyFirstHiddenFrame(void)
{
    int hidden = 0;
    int i;

    for (i = 0; i < 120; i++)
    {
        CHECK_EQ(0, gmRemixKothRulesHiddenTimeout(1, &hidden, 120));
    }
    CHECK_EQ(120, hidden);
    CHECK_EQ(1, gmRemixKothRulesHiddenTimeout(1, &hidden, 120));
    CHECK_EQ(0, hidden);
}

static void testAShownHillClearsTheHiddenCount(void)
{
    int hidden = 57;

    CHECK_EQ(0, gmRemixKothRulesHiddenTimeout(0, &hidden, 120));
    CHECK_EQ(0, hidden);
}

void remixTestKothRules(void)
{
    printf("koth rules\n");
    RUN_TEST(testAnEmptyHillHasNoKing);
    RUN_TEST(testOneFighterOnTheHillIsTheCandidate);
    RUN_TEST(testTeammatesShareTheHill);
    RUN_TEST(testTwoTeamsOnTheHillContestIt);
    RUN_TEST(testHoldingAnEnemyContestsTheHillAndLosesTheClaim);
    RUN_TEST(testAContestStartsTheBuffer);
    RUN_TEST(testTheBufferHoldsTheHillUntilItRunsOut);
    RUN_TEST(testMeterSitsAtTheBottomUnlessTheDragonKingHudApplies);
    RUN_TEST(testMeterXFollowsThePort);
    RUN_TEST(testFfaTagsFollowTheColorOrderOfTheAsmAndACpuIsGray);
    RUN_TEST(testTeamTagsUseTheTeamColorOrderAndNeverGray);
    RUN_TEST(testTagColorsAreTheColorTagValuesOfTheAsm);
    RUN_TEST(testTheContestedGrayStartsDarkAndLightensAsTheBufferRunsOut);
    RUN_TEST(testOnlyTheKingKeepsItsWinTimerAndNoKingResetsEveryTimer);
    RUN_TEST(testMeterFramesUseTheDamageIconColorsAndBarsTheDarkerOnes);
    RUN_TEST(testAFlashStartsFromZeroAndBouncesBetweenZeroAndOneHundred);
    RUN_TEST(testInvalidLinesAreDrawnAgainWithoutUsingATry);
    RUN_TEST(testTheFourthRepeatOfTheCurrentHillKeepsIt);
    RUN_TEST(testThreeRepeatsAndInvalidLinesStillReachANewHill);
    RUN_TEST(testAStageWithNoValidLineKeepsTheHillInsteadOfHanging);
    RUN_TEST(testAHiddenHillTimesOutOnItsHundredAndTwentyFirstHiddenFrame);
    RUN_TEST(testAShownHillClearsTheHiddenCount);
}
