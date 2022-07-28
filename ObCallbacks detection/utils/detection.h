#pragma once
#include "../src/Driver.h"

NTSTATUS enumCallbacks(POBJECT_TYPE objectType);
PCHAR getNameFromAddr(UINT_PTR addr);
PVOID getModuleList();