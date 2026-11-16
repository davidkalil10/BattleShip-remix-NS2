/**
 * test_dedede_inhale_rules.c - unit tests of the action ids of the Dedede inhale
 * (ft/ftremix/dedede/ftremixdededeinhalerules.c, Dedede/DededeSpecial.asm:859-2435): the 27 patched status changes of
 * Kirby's SpecialN routines set the Dedede action on Dedede and Kirby's DEDEDE_NSP_* action on a Kirby whose hat id is
 * 0x20 or more (_kirby_hat_check).
 */

#include "test_check.h"

#include "ft/ftremix/dedede/ftremixdededeinhalerules.h"

#define VANILLA_SPECIALN_FIRST 0x10D
#define VANILLA_SPECIALN_COPY 0x115
#define VANILLA_SPECIALAIRN_COPY 0x11E
#define HAT_DEDEDE 0x20
#define HAT_DEDEDE_MOUTH_OPEN 0x21
#define HAT_MARINA 0x1F

static const int sDededeStatuses[] =
{
    0xE7, 0xE8, 0xEF, 0xEA, 0xEB, 0xED, 0xEC, 0xEE, VANILLA_SPECIALN_COPY,
    0xF0, 0xF1, 0xF7, 0xF2, 0xF3, 0xF5, 0xF4, 0xF6, VANILLA_SPECIALAIRN_COPY
};

static const int sKirbyStatuses[] =
{
    0x16C, 0x16D, 0x173, 0x16E, 0x16F, 0x171, 0x170, 0x172, VANILLA_SPECIALN_COPY,
    0x174, 0x175, 0x17B, 0x176, 0x177, 0x179, 0x178, 0x17A, VANILLA_SPECIALAIRN_COPY
};

static void testDededeGetsHisOwnActions(void)
{
    int i;

    for (i = 0; i < 18; i++)
    {
        CHECK_EQ(sDededeStatuses[i], ftRemixDededeInhaleStatus(VANILLA_SPECIALN_FIRST + i));
    }
}

static void testKirbyWithTheDededeHatGetsTheKirbyCopies(void)
{
    int i;

    for (i = 0; i < 18; i++)
    {
        CHECK_EQ(sKirbyStatuses[i], ftRemixDededeInhaleKirbyStatus(HAT_DEDEDE, VANILLA_SPECIALN_FIRST + i));
        CHECK_EQ(sKirbyStatuses[i], ftRemixDededeInhaleKirbyStatus(HAT_DEDEDE_MOUTH_OPEN, VANILLA_SPECIALN_FIRST + i));
    }
}

static void testKirbyBelowHat0x20KeepsTheVanillaInhale(void)
{
    int i;

    for (i = 0; i < 18; i++)
    {
        CHECK_EQ(VANILLA_SPECIALN_FIRST + i, ftRemixDededeInhaleKirbyStatus(0, VANILLA_SPECIALN_FIRST + i));
        CHECK_EQ(VANILLA_SPECIALN_FIRST + i, ftRemixDededeInhaleKirbyStatus(HAT_MARINA, VANILLA_SPECIALN_FIRST + i));
    }
}

static void testHatByteIsComparedUnsigned(void)
{
    CHECK_EQ(0x16E, ftRemixDededeInhaleKirbyStatus(-1, 0x110));
}

static void testStatusesOutsideSpecialNAreKept(void)
{
    CHECK_EQ(0x10C, ftRemixDededeInhaleStatus(0x10C));
    CHECK_EQ(0x11F, ftRemixDededeInhaleStatus(0x11F));
    CHECK_EQ(0x0A, ftRemixDededeInhaleStatus(0x0A));
    CHECK_EQ(0x10C, ftRemixDededeInhaleKirbyStatus(HAT_DEDEDE, 0x10C));
    CHECK_EQ(0x11F, ftRemixDededeInhaleKirbyStatus(HAT_DEDEDE, 0x11F));
    CHECK_EQ(0x16C, ftRemixDededeInhaleKirbyStatus(HAT_DEDEDE, 0x16C));
}

void remixTestDededeInhaleRules(void)
{
    printf("dedede inhale rules\n");
    RUN_TEST(testDededeGetsHisOwnActions);
    RUN_TEST(testKirbyWithTheDededeHatGetsTheKirbyCopies);
    RUN_TEST(testKirbyBelowHat0x20KeepsTheVanillaInhale);
    RUN_TEST(testHatByteIsComparedUnsigned);
    RUN_TEST(testStatusesOutsideSpecialNAreKept);
}
