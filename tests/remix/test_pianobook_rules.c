/**
 * test_pianobook_rules.c - unit tests of Mad Piano's book rules (wp/wpmario/wpmarioremixfireballrules.c): the book count
 * of fireball.asm Fireball.Book.create_ (the normal book plus one per bonus ammo) and the spread of each book.
 */

#include "test_check.h"

#include "wp/wpmario/wpmarioremixfireballrules.h"

static void testNoBonusAmmoMakesOneBook(void)
{
    CHECK_EQ(1, wpRemixPianoBookRulesCount(0));
}

static void testEveryBonusAmmoAddsABookToTheNormalOne(void)
{
    CHECK_EQ(2, wpRemixPianoBookRulesCount(1));
    CHECK_EQ(4, wpRemixPianoBookRulesCount(3));
    CHECK_EQ(6, wpRemixPianoBookRulesCount(5));
}

static void testNegativeAmmoStillMakesTheNormalBook(void)
{
    CHECK_EQ(1, wpRemixPianoBookRulesCount(-1));
}

static void testSpreadGrowsAQuarterPerIteration(void)
{
    CHECK_TRUE(wpRemixPianoBookRulesSpreadIntensity(0) == 1.0F);
    CHECK_TRUE(wpRemixPianoBookRulesSpreadIntensity(1) == 1.25F);
    CHECK_TRUE(wpRemixPianoBookRulesSpreadIntensity(5) == 2.25F);
}

void remixTestPianoBookRules(void)
{
    printf("piano book rules\n");
    RUN_TEST(testNoBonusAmmoMakesOneBook);
    RUN_TEST(testEveryBonusAmmoAddsABookToTheNormalOne);
    RUN_TEST(testNegativeAmmoStillMakesTheNormalBook);
    RUN_TEST(testSpreadGrowsAQuarterPerIteration);
}
