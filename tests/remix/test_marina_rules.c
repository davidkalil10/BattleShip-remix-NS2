/**
 * test_marina_rules.c - unit tests of Marina's midair jump rules and Cargo shake gem rule
 * (ft/ftremix/marina/ftremixmarinarules.c): jump_height_table and jump_timing_patch_ of Marina/Marina.asm:802-882, and
 * the gem block of shake_main_ in Marina/MarinaSpecial.asm:76-96.
 */

#include "test_check.h"

#include "ft/ftremix/marina/ftremixmarinarules.h"

#define STATUS_JUMP_AERIAL_F 0x18
#define STATUS_JUMP_AERIAL_B 0x19
#define STATUS_FALL 0x1A
#define STATUS_JUMP_F 0x16
#define HITSTATUS_NORMAL 1
#define HITSTATUS_INVINCIBLE 2
#define HITSTATUS_INTANGIBLE 3

static void testEachMidairJumpIsLowerThanTheLast(void)
{
    CHECK_EQ(0, ftRemixMarinaJumpStickRange(0));
    CHECK_EQ(80, ftRemixMarinaJumpStickRange(1));
    CHECK_EQ(70, ftRemixMarinaJumpStickRange(2));
    CHECK_EQ(60, ftRemixMarinaJumpStickRange(3));
    CHECK_EQ(50, ftRemixMarinaJumpStickRange(4));
    CHECK_EQ(40, ftRemixMarinaJumpStickRange(5));
}

static void testJumpCountsOutsideTheTableGiveNoHeight(void)
{
    CHECK_EQ(0, ftRemixMarinaJumpStickRange(6));
    CHECK_EQ(0, ftRemixMarinaJumpStickRange(7));
    CHECK_EQ(0, ftRemixMarinaJumpStickRange(-1));
}

static void testJumpIsLockedDuringAMidairJumpUntilTheFlag(void)
{
    CHECK_EQ(1, ftRemixMarinaJumpIsLocked(STATUS_JUMP_AERIAL_F, 0));
    CHECK_EQ(1, ftRemixMarinaJumpIsLocked(STATUS_JUMP_AERIAL_B, 0));
    CHECK_EQ(0, ftRemixMarinaJumpIsLocked(STATUS_JUMP_AERIAL_F, 1));
    CHECK_EQ(0, ftRemixMarinaJumpIsLocked(STATUS_JUMP_AERIAL_B, 1));
}

static void testJumpIsFreeOutsideAMidairJump(void)
{
    CHECK_EQ(0, ftRemixMarinaJumpIsLocked(STATUS_FALL, 0));
    CHECK_EQ(0, ftRemixMarinaJumpIsLocked(STATUS_JUMP_F, 0));
    CHECK_EQ(0, ftRemixMarinaJumpIsLocked(STATUS_FALL, 1));
}

static void testShakeDamageNeedsDamageAndAVulnerableVictim(void)
{
    CHECK_EQ(1, ftRemixMarinaShakeDamageApplies(1, HITSTATUS_NORMAL));
    CHECK_EQ(0, ftRemixMarinaShakeDamageApplies(0, HITSTATUS_NORMAL));
    CHECK_EQ(0, ftRemixMarinaShakeDamageApplies(1, HITSTATUS_INVINCIBLE));
    CHECK_EQ(0, ftRemixMarinaShakeDamageApplies(1, HITSTATUS_INTANGIBLE));
}

static void testGemSpawnsOnceOnTheDamageFrame(void)
{
    CHECK_EQ(1, ftRemixMarinaShakeSpawnsGem(1, 0));
    CHECK_EQ(0, ftRemixMarinaShakeSpawnsGem(0, 0));
    CHECK_EQ(0, ftRemixMarinaShakeSpawnsGem(1, 1));
    CHECK_EQ(0, ftRemixMarinaShakeSpawnsGem(0, 1));
}

void remixTestMarinaRules(void)
{
    printf("marina rules\n");
    RUN_TEST(testEachMidairJumpIsLowerThanTheLast);
    RUN_TEST(testJumpCountsOutsideTheTableGiveNoHeight);
    RUN_TEST(testJumpIsLockedDuringAMidairJumpUntilTheFlag);
    RUN_TEST(testJumpIsFreeOutsideAMidairJump);
    RUN_TEST(testShakeDamageNeedsDamageAndAVulnerableVictim);
    RUN_TEST(testGemSpawnsOnceOnTheDamageFrame);
}
