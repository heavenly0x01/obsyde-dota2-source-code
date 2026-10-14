#pragma once
#include <cstdint>
#include <vector>

namespace scan {

struct FieldBind {
    const char* name;
    uintptr_t* var;
    const char* anchors[3];
};

extern const FieldBind g_fields[];
extern const int g_fieldCount;

void Resolve(uintptr_t base);
void Tick();
void EsGlobals(uintptr_t base, std::vector<uintptr_t>& out);

}
