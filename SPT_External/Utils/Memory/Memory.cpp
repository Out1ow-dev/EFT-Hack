#include "memory.h"

namespace driver {
    uintptr_t BaseAddress;
}

bool Memory::Init()
{
	PID = get_process_id(L"EscapeFromTarkov.exe");
	const HANDLE driver = CreateFile("\\\\.\\OutlowDriver", GENERIC_READ, 0,
		nullptr, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, nullptr);

	if (driver == INVALID_HANDLE_VALUE) {
		std::cout << "Failed to create our driver handle\n";
		std::cin.get();
		return 1;
	}

	if (attach_to_process(driver, PID) == true)
	{
		std::cout << "[+++] Attach success!!!\n";
	}
	else
		std::cout << "[-] Attach failed!\n";

	BaseAddress = get_module_base(PID, L"UnityPlayer.dll");

    return true;
}

