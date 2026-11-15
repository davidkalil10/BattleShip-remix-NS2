/**
 * test_japanese_sounds_rules.c - unit tests of the Japanese Sounds toggle rules (ft/ftremix/ftremixjapanesesoundsrules.c):
 * which fighters use the J sounds and the J hit sound table of fighter hitboxes (Character.asm apply_sound_type_).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixjapanesesoundsrules.h"

#define TEST_ROW_PUNCH 0
#define TEST_ROW_KICK 1
#define TEST_ROW_COIN 2
#define TEST_ROW_BAT 7

static void testDefaultUsesJOnlyForJFighters(void)
{
    CHECK_TRUE(ftRemixJapaneseSoundsUseJ(FTREMIXJSFX_TOGGLE_DEFAULT, 1));
    CHECK_TRUE(!ftRemixJapaneseSoundsUseJ(FTREMIXJSFX_TOGGLE_DEFAULT, 0));
}

static void testAlwaysAndNeverIgnoreTheFighter(void)
{
    CHECK_TRUE(ftRemixJapaneseSoundsUseJ(FTREMIXJSFX_TOGGLE_ALWAYS, 0));
    CHECK_TRUE(ftRemixJapaneseSoundsUseJ(FTREMIXJSFX_TOGGLE_ALWAYS, 1));
    CHECK_TRUE(!ftRemixJapaneseSoundsUseJ(FTREMIXJSFX_TOGGLE_NEVER, 0));
    CHECK_TRUE(!ftRemixJapaneseSoundsUseJ(FTREMIXJSFX_TOGGLE_NEVER, 1));
}

static void testJTableReplacesPunchAndKickRows(void)
{
    CHECK_EQ(0x93, ftRemixJapaneseHitCollisionFgmRule(1, TEST_ROW_PUNCH, 0, 0x28));
    CHECK_EQ(0x92, ftRemixJapaneseHitCollisionFgmRule(1, TEST_ROW_PUNCH, 1, 0x26));
    CHECK_EQ(0x91, ftRemixJapaneseHitCollisionFgmRule(1, TEST_ROW_PUNCH, 2, 0x25));
    CHECK_EQ(0x90, ftRemixJapaneseHitCollisionFgmRule(1, TEST_ROW_KICK, 0, 0x22));
    CHECK_EQ(0x8F, ftRemixJapaneseHitCollisionFgmRule(1, TEST_ROW_KICK, 1, 0x20));
    CHECK_EQ(0x8E, ftRemixJapaneseHitCollisionFgmRule(1, TEST_ROW_KICK, 2, 0x1F));
}

static void testJTableKeepsTheOtherRowsAndTheBatPunchIds(void)
{
    CHECK_EQ(0x26, ftRemixJapaneseHitCollisionFgmRule(1, TEST_ROW_BAT, 0, 0x26));
    CHECK_EQ(0x25, ftRemixJapaneseHitCollisionFgmRule(1, TEST_ROW_BAT, 1, 0x25));
    CHECK_EQ(0x1234, ftRemixJapaneseHitCollisionFgmRule(1, TEST_ROW_COIN, 0, 0x1234));
}

static void testUTableKeepsEveryId(void)
{
    CHECK_EQ(0x28, ftRemixJapaneseHitCollisionFgmRule(0, TEST_ROW_PUNCH, 0, 0x28));
    CHECK_EQ(0x1F, ftRemixJapaneseHitCollisionFgmRule(0, TEST_ROW_KICK, 2, 0x1F));
    CHECK_EQ(0x26, ftRemixJapaneseHitCollisionFgmRule(0, TEST_ROW_BAT, 0, 0x26));
}

void remixTestJapaneseSoundsRules(void)
{
    printf("japanese sounds rules\n");
    RUN_TEST(testDefaultUsesJOnlyForJFighters);
    RUN_TEST(testAlwaysAndNeverIgnoreTheFighter);
    RUN_TEST(testJTableReplacesPunchAndKickRows);
    RUN_TEST(testJTableKeepsTheOtherRowsAndTheBatPunchIds);
    RUN_TEST(testUTableKeepsEveryId);
}
