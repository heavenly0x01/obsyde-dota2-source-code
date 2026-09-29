#include "mem.h"

namespace rtti {

using RttiEntry = Entry;

#include "rtti_client.inc"

static const RttiEntry* kTable = kClientRtti;
static const int kCount = (int)(sizeof(kClientRtti) / sizeof(kClientRtti[0]));

static int g_delta = 0;
static bool g_ready = false;
static uintptr_t g_clientBase = 0;

const char* Lookup(uint32_t rva) {
    int lo = 0, hi = kCount - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        uint32_t v = kTable[mid].rva;
        if (v == rva) return kTable[mid].name;
        if (v < rva) lo = mid + 1;
        else hi = mid - 1;
    }
    return nullptr;
}

static const char* LookupDelta(uintptr_t clientBase, uintptr_t vptr, int delta) {
    if (!clientBase || vptr < clientBase) return nullptr;
    uint64_t r64 = (uint64_t)(vptr - clientBase);
    if (r64 + (uint64_t)(delta < 0 ? -delta : delta) > 0x10000000ULL) return nullptr;
    uint32_t rva = (uint32_t)((int64_t)r64 - delta);
    return Lookup(rva);
}

bool Ready() { return g_ready; }

void DetectDelta(uintptr_t clientBase, const uintptr_t* objects, int count) {
    g_clientBase = clientBase;
    static const int deltas[] = { 0, 8, -8, 16, -16, 4, -4, 24, -24 };
    int best = 0, bestScore = 0;

    for (int d : deltas) {
        int score = 0;
        for (int i = 0; i < count; ++i) {
            if (!objects[i]) continue;
            if (LookupDelta(clientBase, objects[i], d)) ++score;
        }
        if (score > bestScore) { bestScore = score; best = d; }
    }

    g_delta = best;
    g_ready = bestScore >= 2;
}

const char* ClassOf(uintptr_t clientBase, uintptr_t vptr) {
    if (!vptr) return nullptr;
    if (g_ready) {
        const char* n = LookupDelta(clientBase, vptr, g_delta);
        if (n) return n;
    }
    if (!clientBase) return nullptr;

    static const int deltas[] = { 0, 8, -8, 16, -16, 4, -4 };
    for (int d : deltas) {
        const char* n = LookupDelta(clientBase, vptr, d);
        if (n) return n;
    }
    return nullptr;
}

}
