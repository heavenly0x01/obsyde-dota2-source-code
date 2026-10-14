#include "mem.h"
#include <unordered_map>
#include <vector>
#include <cstring>
#include <algorithm>

namespace rtti {

static std::vector<Entry> g_tab;
static std::vector<char> g_arena;
static int g_delta = 0;
static bool g_ready = false;
static uintptr_t g_clientBase = 0;

static const char* Arena(const char* s, size_t n) {
    size_t old = g_arena.size();
    g_arena.resize(old + n + 1);
    if (n) memcpy(g_arena.data() + old, s, n);
    g_arena[old + n] = 0;
    return g_arena.data() + old;
}

static bool MangledChar(char ch) {
    return (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') ||
           ch == '_' || ch == '@' || ch == '$' || ch == '?' || ch == '.' || ch == '<' ||
           ch == '>' || ch == '-' || ch == '`';
}

static const char* Demangle(const char* n, size_t len) {
    if (len >= 4 && memcmp(n, ".?A", 3) == 0) {
        size_t i = 3;
        unsigned char c = (unsigned char)n[3];
        if (c == 'W') {
            i += 2;
        } else if (c == 'V' || c == 'U' || c == 'T' || c == 'X' || c == 'Y' || c == 'Z' ||
                   c == '7' || c == '8' || c == '9') {
            ++i;
        }
        if (i <= len) {
            const char* body = n + i;
            size_t blen = len - i;
            size_t end = blen;
            for (size_t k = 0; k + 1 < blen; ++k)
                if (body[k] == '@' && body[k + 1] == '@') { end = k; break; }
            char buf[512];
            size_t o = 0;
            for (size_t k = 0; k < end && o + 3 < sizeof(buf); ++k) {
                if (body[k] == '$') break;
                if (body[k] == '@') {
                    buf[o++] = ':';
                    buf[o++] = ':';
                    continue;
                }
                buf[o++] = body[k];
            }
            while (o >= 2 && buf[o - 2] == ':' && buf[o - 1] == ':') o -= 2;
            if (o > 0) return Arena(buf, o);
        }
    }
    return Arena(n, len);
}

static bool TdClass(uintptr_t base, uint32_t size, uint32_t td, const char** out) {
    if (td + 20 >= size) return false;
    uintptr_t vf = *(const uintptr_t*)(base + td);
    if (vf < base || vf - base >= size) return false;
    const char* s = (const char*)(base + td + 16);
    if (memcmp(s, ".?A", 3) != 0) return false;
    size_t avail = size - (td + 16);
    size_t len = 0;
    while (len < avail && len < 300 && s[len]) {
        if (!MangledChar(s[len])) return false;
        ++len;
    }
    if (len < 5 || len >= 300 || len >= avail) return false;
    bool at = false;
    for (size_t k = 0; k < len; ++k)
        if (s[k] == '@') { at = true; break; }
    if (!at) return false;
    *out = s;
    return true;
}

bool Build(uintptr_t base) {
    if (g_ready) return true;
    if (!base) return false;
    uint32_t size = mem::ModuleSize(base);
    if (!size) return false;
    mem::ModSec secs[64];
    int ns = mem::ModSections(base, secs, 64);
    if (ns <= 0) return false;
    g_arena.reserve(32u << 20);

    struct Td { uint32_t rva; const char* cls; };
    std::vector<Td> tds;
    std::unordered_map<uint32_t, uint32_t> tdAt;
    for (int i = 0; i < ns; ++i) {
        if (secs[i].flags & 0x20000000u) continue;
        if (secs[i].va >= size) continue;
        uint32_t vs = secs[i].vsize;
        if (vs > size - secs[i].va) vs = size - secs[i].va;
        uint32_t lo = secs[i].va;
        uint32_t end = lo + vs;
        for (uint32_t r = lo; r + 20 < end; r += 8) {
            const char* raw = nullptr;
            if (!TdClass(base, size, r, &raw)) continue;
            Td t;
            t.rva = r;
            t.cls = Demangle(raw, strlen(raw));
            tdAt[r] = (uint32_t)tds.size();
            tds.push_back(t);
        }
    }

    std::unordered_map<uint32_t, uint32_t> colAt;
    for (int i = 0; i < ns; ++i) {
        if (secs[i].flags & 0x20000000u) continue;
        if (secs[i].va >= size) continue;
        uint32_t vs = secs[i].vsize;
        if (vs > size - secs[i].va) vs = size - secs[i].va;
        uint32_t lo = secs[i].va;
        uint32_t end = lo + vs;
        for (uint32_t r = lo; r + 24 <= end; r += 4) {
            uint32_t sig;
            memcpy(&sig, (const void*)(base + r), 4);
            if (sig != 1) continue;
            uint32_t td;
            memcpy(&td, (const void*)(base + r + 12), 4);
            if (!tdAt.count(td)) continue;
            uint32_t chd;
            memcpy(&chd, (const void*)(base + r + 16), 4);
            if (chd >= size) continue;
            uint32_t self;
            memcpy(&self, (const void*)(base + r + 20), 4);
            if (self && self != r) continue;
            colAt[r] = r;
        }
    }

    std::vector<Entry> tab;
    for (int i = 0; i < ns; ++i) {
        if (secs[i].flags & 0x20000000u) continue;
        if (secs[i].va >= size) continue;
        uint32_t vs = secs[i].vsize;
        if (vs > size - secs[i].va) vs = size - secs[i].va;
        uint32_t lo = secs[i].va;
        uint32_t end = lo + vs;
        for (uint32_t r = lo; r + 16 <= end; r += 8) {
            uintptr_t v = *(const uintptr_t*)(base + r);
            if (v < base || v - base >= size) continue;
            uint32_t col = (uint32_t)(v - base);
            if (!colAt.count(col)) continue;
            uintptr_t f = *(const uintptr_t*)(base + r + 8);
            if (f < base || f - base >= size) continue;
            bool inCode = false;
            for (int k = 0; k < ns; ++k) {
                if (!(secs[k].flags & 0x20000000u)) continue;
                uint32_t fr = (uint32_t)(f - base);
                if (fr >= secs[k].va && fr < secs[k].va + secs[k].vsize) { inCode = true; break; }
            }
            if (!inCode) continue;
            uint32_t td;
            memcpy(&td, (const void*)(base + col + 12), 4);
            auto it = tdAt.find(td);
            if (it == tdAt.end()) continue;
            Entry e;
            e.rva = r + 8;
            e.name = tds[it->second].cls;
            tab.push_back(e);
        }
    }

    if (tab.size() < 1000) return false;
    std::sort(tab.begin(), tab.end(),
              [](const Entry& a, const Entry& b) { return a.rva < b.rva; });
    g_tab.swap(tab);
    g_delta = 0;
    g_ready = true;
    return true;
}

const char* Lookup(uint32_t rva) {
    int lo = 0, hi = (int)g_tab.size() - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        uint32_t v = g_tab[mid].rva;
        if (v == rva) return g_tab[mid].name;
        if (v < rva) lo = mid + 1;
        else hi = mid - 1;
    }
    return nullptr;
}

uint32_t VtableByName(const char* name) {
    if (!name || !g_ready) return 0;
    for (const Entry& e : g_tab)
        if (strcmp(e.name, name) == 0) return e.rva;
    return 0;
}

int Count() {
    return (int)g_tab.size();
}

static const char* LookupDelta(uintptr_t clientBase, uintptr_t vptr, int delta) {
    if (!clientBase || vptr < clientBase) return nullptr;
    uint64_t r64 = (uint64_t)(vptr - clientBase);
    if (r64 + (uint64_t)(delta < 0 ? -delta : delta) > 0x10000000ULL) return nullptr;
    uint32_t rva = (uint32_t)((int64_t)r64 - delta);
    return Lookup(rva);
}

bool Ready() { return g_ready; }

int Delta() { return g_delta; }

void DetectDelta(uintptr_t clientBase, const uintptr_t* objects, int count) {
    g_clientBase = clientBase;
    int best = 0, bestScore = 0;

    for (int d = -512; d <= 512; d += 4) {
        int score = 0;
        for (int i = 0; i < count; ++i) {
            if (!objects[i]) continue;
            if (LookupDelta(clientBase, objects[i], d)) ++score;
        }
        int ad = d < 0 ? -d : d, ab = best < 0 ? -best : best;
        if (score > bestScore || (score == bestScore && score > 0 && ad < ab)) {
            bestScore = score; best = d;
        }
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
