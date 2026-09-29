#pragma once
#include "common.h"

namespace mem {

inline uintptr_t ModuleBase(const char* name) {
    return (uintptr_t)GetModuleHandleA(name);
}

inline uint32_t ModuleSize(uintptr_t base) {
    if (!base) return 0;
    __try {
        auto dos = (const IMAGE_DOS_HEADER*)base;
        if (dos->e_magic != IMAGE_DOS_SIGNATURE) return 0;
        auto nt = (const IMAGE_NT_HEADERS*)(base + dos->e_lfanew);
        if (nt->Signature != IMAGE_NT_SIGNATURE) return 0;
        return nt->OptionalHeader.SizeOfImage;
    }
    __except (1) {
        return 0;
    }
}

template <typename T>
inline bool Read(uintptr_t addr, T& out) {
    if (addr < 0x10000ULL) return false;
    __try {
        const volatile unsigned char* src = (const volatile unsigned char*)addr;
        unsigned char* dst = (unsigned char*)&out;
        for (size_t i = 0; i < sizeof(T); ++i) dst[i] = src[i];
        return true;
    }
    __except (1) {
        return false;
    }
}

template <typename T>
inline bool Write(uintptr_t addr, const T& value) {
    if (addr < 0x10000ULL) return false;
    __try {
        *(volatile T*)addr = value;
        return true;
    }
    __except (1) {
        return false;
    }
}

template <typename T>
inline T ReadOr(uintptr_t addr, T def = T{}) {
    T v{};
    if (!Read(addr, v)) return def;
    return v;
}

inline bool ValidPtr(uintptr_t p) {
    return p >= 0x100000000ULL && p < 0x00007FFFFFFFFFFFULL && (p & 7) == 0;
}

inline bool ReadStr(uintptr_t addr, char* out, int cap) {
    if (!out || cap <= 0) return false;
    out[0] = 0;
    if (addr < 0x10000ULL) return false;

    int written = 0;
    for (int i = 0; i < cap - 1; i += 8) {
        uint64_t chunk = 0;
        if (!Read(addr + i, chunk)) {
            return written > 0;
        }
        for (int b = 0; b < 8; ++b) {
            char c = (char)((chunk >> (b * 8)) & 0xFF);
            if (c == 0) { out[written] = 0; return written > 0; }
            if (i + b >= cap - 1) { out[written] = 0; return written > 0; }
            out[written++] = c;
        }
    }
    out[written] = 0;
    return written > 0;
}

}

namespace rtti {

struct Entry {
    uint32_t rva;
    const char* name;
};

const char* Lookup(uint32_t rva);

const char* ClassOf(uintptr_t clientBase, uintptr_t vptr);

void DetectDelta(uintptr_t clientBase, const uintptr_t* objects, int count);
bool Ready();

}
