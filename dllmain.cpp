#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

HMODULE g_dll_instance;
// some of cs2 most important libraries
std::vector<std::string> modules = {"client.dll",
                                    "server.dll",
                                    "host.dll",
                                    "matchmaking.dll",
                                    "animationsystem.dll",
                                    "assetpreview.dll",
                                    "dbghelp.dll",
                                    "embree3.dll",
                                    "engine2.dll",
                                    "inputsystem.dll",
                                    "materialsystem2.dll",
                                    "meshsystem.dll",
                                    "navsystem.dll",
                                    "networksystem.dll",
                                    "panorama.dll",
                                    "panoramauiclient.dll",
                                    "particles.dll",
                                    "rendersystemdx11.dll",
                                    "rendersystemempty.dll",
                                    "rendersystemvulkan.dll",
                                    "scenesystem.dll",
                                    "schemasystem.dll",
                                    "soundsystem.dll",
                                    "tier0.dll",
                                    "vphysics2.dll",
                                    "vscript.dll",
                                    "worldrenderer.dll"};

uintptr_t addOffset(uintptr_t address, uintptr_t offset) {
  return address + offset;
}

uintptr_t GetAbsoluteAddress(uintptr_t address) {
  if (address != 0) {
    int relative = *reinterpret_cast<int*>(
        address);  // relative is IMM32 (rip-relative offset)
    address +=
        relative +
        sizeof(relative);  // sizeof(relative) == 4 because rip-relative offset
                           // is calculated after IMM32 instruction
  }

  return address;
}

using CreateInterfaceCallback = uintptr_t* (*)();

struct CreateInterfaceNode {
  CreateInterfaceCallback callback_function;
  char* interface_name;
  CreateInterfaceNode* next_node;
};

HMODULE GetModuleHandleSafe(const char* module_name) {
  HMODULE handle = GetModuleHandleA(module_name);
  if (handle == NULL) {
    return nullptr;
  }
  return handle;
}

void DumpAllInterfaces() {
  for (auto iterator = modules.begin(); iterator != modules.end(); ++iterator) {
    HMODULE handle = GetModuleHandleSafe(iterator->c_str());
    if (handle == nullptr) {
      // std::cout << "Failed to get handle: " << iterator->c_str() <<
      // std::endl;
      continue;
    }
    // std::cout << "Got " << iterator->c_str() << " handle\n";
    FARPROC create_interface = GetProcAddress(handle, "CreateInterface");
    if (create_interface == NULL) {
      // std::cout << iterator->c_str()
      //           << " does not have create interface function\n";
      continue;
    }
    // std::cout << "Got " << iterator->c_str() << " create interface\n";
    auto root_node = (CreateInterfaceNode*)*(uintptr_t*)GetAbsoluteAddress(
        addOffset((uintptr_t)create_interface, 3));
    // std::cout << "Pointer to interface structure address: " << std::hex <<
    // GetAbsoluteAddress(
    //     addOffset((uintptr_t)create_interface, 3)) << std::endl;
    for (CreateInterfaceNode* it = root_node; it != nullptr;
         it = it->next_node) {
      std::cout << std::hex << iterator->c_str() << " -> " << it->interface_name
                << " -> " << it->callback_function() << std::endl;
    }
  }
}

void CreateConsole() {
  AllocConsole();
  SetConsoleTitleW(L"Debug Console");
  FILE* f;
  freopen_s(&f, "CONOUT$", "w", stdout);
  std::ios::sync_with_stdio(true);
  std::cout.clear();
}

void DestroyConsole() {
  fclose(stdout);
  FreeConsole();
  FreeLibraryAndExitThread(g_dll_instance, EXIT_SUCCESS);
}

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
      g_dll_instance = hModule;
      CreateConsole();
      DumpAllInterfaces();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}
