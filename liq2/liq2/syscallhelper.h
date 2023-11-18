#pragma once
#include "Windows.h"
#include "structs.h"

void GetModuleInfo(DWORD process_id, WCHAR *module_name, ModuleInfo *out);
void GetProcessHandle(HANDLE *process_handle_ptr, DWORD *proc_id);