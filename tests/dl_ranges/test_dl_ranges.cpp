#include "fast/dl_walk_bounds.h"
#include "port_dl_ranges.h"
#include "port_frame_profile.h"

#include <cstdio>

int gPortFrameProfileEnabled = 0;

uint64_t port_frame_profile_now_ns(void) {
    return 0;
}

void port_frame_profile_add(PortProfileCategory, uint64_t) {}

namespace {

constexpr uintptr_t kArenaBase = 0x10000000;
constexpr size_t kArenaSize = 0x1000000;
constexpr uintptr_t kArenaEnd = kArenaBase + kArenaSize;
constexpr uintptr_t kFileBase = kArenaEnd + 0x4000;
constexpr size_t kFileSize = 0x800;
constexpr uintptr_t kFileEnd = kFileBase + kFileSize;
constexpr size_t kWalkPastWindow = 0x10000;

int sFailures = 0;

struct Lookup {
    int result;
    uintptr_t begin;
    uintptr_t end;
};

/**
 * Classifies an address and returns the span that shares its answer.
 * @param addr Address to classify.
 * @return The classification and the [begin, end) span reported with it.
 */
Lookup lookUp(uintptr_t addr) {
    uintptr_t base = 0;
    size_t size = 0;
    const int result = port_dl_range_lookup(addr, &base, &size);
    return Lookup{ result, base, base + size };
}

/**
 * Records a failed expectation without stopping the run.
 * @param condition Expectation that must hold.
 * @param name Text printed when it does not.
 */
void expect(bool condition, const char* name) {
    if (!condition) {
        sFailures++;
        std::printf("FAIL: %s\n", name);
    }
}

void shouldReportContainingRange_whenAddressIsInside() {
    // When
    const Lookup arena = lookUp(kArenaBase + 0x10);
    const Lookup file = lookUp(kFileBase + 8);

    // Then
    expect(arena.result == PORT_DL_IN_RANGE && arena.begin == kArenaBase && arena.end == kArenaEnd, "arena in range");
    expect(file.result == PORT_DL_IN_RANGE && file.begin == kFileBase && file.end == kFileEnd, "file in range");
}

void shouldReportGap_whenAddressIsOutsideEveryWindow() {
    // When
    const Lookup below = lookUp(kArenaBase - 0x10);
    const Lookup above = lookUp(kFileEnd + kWalkPastWindow);

    // Then
    expect(below.result == PORT_DL_UNKNOWN && below.end == kArenaBase, "gap below the arena");
    expect(above.result == PORT_DL_UNKNOWN && above.begin == kFileEnd + kWalkPastWindow, "gap above the windows");
}

void shouldReportSpanFromRangeEnd_whenAddressIsInWalkPastWindow() {
    // When
    const Lookup firstCommand = lookUp(kArenaEnd);
    const Lookup deepInWindow = lookUp(kArenaEnd + 0x2000);

    // Then
    expect(firstCommand.result == PORT_DL_WALKED_PAST && firstCommand.begin == kArenaEnd && firstCommand.end == kFileBase,
           "first command after the arena");
    expect(deepInWindow.result == PORT_DL_WALKED_PAST && deepInWindow.begin == kArenaEnd && deepInWindow.end == kFileBase,
           "heap address inside the arena window keeps the arena end as its start");
}

void shouldUseNearestRangeEnd_whenWindowsOverlap() {
    // When
    const Lookup afterFile = lookUp(kFileEnd + 0x20);

    // Then
    expect(afterFile.result == PORT_DL_WALKED_PAST && afterFile.begin == kFileEnd &&
               afterFile.end == kFileEnd + kWalkPastWindow,
           "address after the file starts at the file end");
}

void shouldSplitSpan_whenAnotherRangeEndsInsideTheWindow() {
    // Given
    const uintptr_t overlapBase = kArenaEnd - 0x100;
    const uintptr_t overlapEnd = overlapBase + 0x300;
    port_dl_range_register(reinterpret_cast<const void*>(overlapBase), 0x300, "overlap");

    // When
    const Lookup inside = lookUp(kArenaEnd + 0x100);
    const Lookup after = lookUp(overlapEnd);
    port_dl_range_unregister(reinterpret_cast<const void*>(overlapBase));
    const Lookup unregistered = lookUp(kArenaEnd + 0x100);

    // Then
    expect(inside.result == PORT_DL_IN_RANGE, "overlapping range wins over the arena window");
    expect(after.result == PORT_DL_WALKED_PAST && after.begin == overlapEnd && after.end == kFileBase,
           "span restarts at the overlapping range end");
    expect(unregistered.result == PORT_DL_WALKED_PAST && unregistered.begin == kArenaEnd,
           "span returns to the arena end after unregister");
}

void shouldStopWalk_onlyOnTheCommandsRightAfterARange() {
    // Given
    const size_t commandSize = 16;
    const size_t lastSteppedOff = (Fast::kDLWalkOffCommandSlots * commandSize) - 1;

    // When
    const Lookup firstCommand = lookUp(kArenaEnd);
    const Lookup heapList = lookUp(kArenaEnd + 0x2000);
    const Lookup inside = lookUp(kArenaEnd - commandSize);

    // Then
    expect(Fast::DLWalkSteppedOffRange(firstCommand.result, kArenaEnd, firstCommand.begin, commandSize),
           "the command at the range end is a walk off the range");
    expect(Fast::DLWalkSteppedOffRange(firstCommand.result, kArenaEnd + lastSteppedOff, firstCommand.begin, commandSize),
           "a multi-word command can land up to three slots after the range end");
    expect(!Fast::DLWalkSteppedOffRange(firstCommand.result, kArenaEnd + lastSteppedOff + 1, firstCommand.begin, commandSize),
           "the fourth slot after the range end is not a walk off the range");
    expect(!Fast::DLWalkSteppedOffRange(heapList.result, kArenaEnd + 0x2000, heapList.begin, commandSize),
           "a list in the heap inside the walk-past window is not a walk off the range");
    expect(!Fast::DLWalkSteppedOffRange(inside.result, kArenaEnd - commandSize, inside.begin, commandSize),
           "the last command inside the range is not a walk off the range");
}

void shouldKeepWindowsApart_whenRangesAreAdjacent() {
    // Given
    const uintptr_t adjacentBase = kFileEnd;
    port_dl_range_register(reinterpret_cast<const void*>(adjacentBase), 0x100, "adjacent");

    // When
    const Lookup atJoin = lookUp(kFileEnd);
    const Lookup after = lookUp(adjacentBase + 0x100);
    port_dl_range_unregister(reinterpret_cast<const void*>(adjacentBase));

    // Then
    expect(atJoin.result == PORT_DL_IN_RANGE && atJoin.begin == adjacentBase, "the next range starts where the file ends");
    expect(after.result == PORT_DL_WALKED_PAST && after.begin == adjacentBase + 0x100,
           "the walked-past span starts at the end of the adjacent range");
}

} // namespace

int main() {
    // Given
    port_dl_range_register(reinterpret_cast<const void*>(kArenaBase), kArenaSize, "arena");
    port_dl_range_register(reinterpret_cast<const void*>(kFileBase), kFileSize, "file");

    shouldReportContainingRange_whenAddressIsInside();
    shouldReportGap_whenAddressIsOutsideEveryWindow();
    shouldReportSpanFromRangeEnd_whenAddressIsInWalkPastWindow();
    shouldUseNearestRangeEnd_whenWindowsOverlap();
    shouldSplitSpan_whenAnotherRangeEndsInsideTheWindow();
    shouldStopWalk_onlyOnTheCommandsRightAfterARange();
    shouldKeepWindowsApart_whenRangesAreAdjacent();

    if (sFailures != 0) {
        std::printf("dl range tests: %d failed\n", sFailures);
        return 1;
    }
    std::puts("dl range tests: all passed");
    return 0;
}
