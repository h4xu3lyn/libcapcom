#pragma once
#define WIN32_NO_STATUS
#include <Windows.h>
#include <Winternl.h>
#undef WIN32_NO_STATUS
#include <functional>

typedef PVOID(NTAPI* MmGetSystemRoutineAddress_t)(PUNICODE_STRING);

extern bool init_exploit();
extern void execute_in_kernel(std::function<void(MmGetSystemRoutineAddress_t)> user_function);
extern bool cleanup_exploit();