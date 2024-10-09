#pragma once
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <dwmapi.h>
#include <vector>
#include <io.h>
#include <thread>
#include <winioctl.h>


namespace codes {
	// установка драйвера
	constexpr ULONG attach =
		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x696, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

	// чтение памяти
	constexpr ULONG read =
		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x697, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

	// запись в память
	constexpr ULONG write =
		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x698, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
}

struct Request {
	HANDLE process_id;

	PVOID target;
	PVOID buffer;

	SIZE_T size;
	SIZE_T return_size;
};

class Memory {
	public:
		DWORD PID;
		HANDLE driver_handle;
		uint64_t BaseAddress;
		bool Init();

		inline bool attach_to_process(HANDLE driver_handle, const DWORD pid) {
			Request r;
			r.process_id = reinterpret_cast<HANDLE>(pid);

			return DeviceIoControl(driver_handle, codes::attach, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
		}

		template <class T>
		void write_memory(HANDLE driver_handle, const std::uintptr_t addr, const T& value)
		{
			Request r;
			r.target = reinterpret_cast<PVOID>(addr);
			r.buffer = (PVOID)&value;
			r.size = sizeof(T);

			DeviceIoControl(driver_handle, codes::write, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
		}

		template <class T>
		T read_memory(const std::uintptr_t addr) {
			T temp = {};

			Request r;
			r.target = reinterpret_cast<PVOID>(addr);
			r.buffer = &temp;
			r.size = sizeof(T);

			DeviceIoControl(driver_handle, codes::read, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

			return temp;
		}

		template <class T>
		T read_memoryChain(const std::uintptr_t addr, const std::vector<std::uintptr_t>& offsets) {
			T result = {};

			Request r;
			r.target = reinterpret_cast<PVOID>(addr + offsets.at(0));
			r.buffer = &result;
			r.size = sizeof(T);

			DeviceIoControl(driver_handle, codes::read, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

			for (int i = 1; i < offsets.size(); i++) {
				r.target = reinterpret_cast<PVOID>(result + offsets.at(i));
				DeviceIoControl(driver_handle, codes::read, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
				result = *reinterpret_cast<T*>(r.buffer);
			}

			return result;
		}

		static DWORD get_process_id(const wchar_t* process_name) {
			DWORD process_id = 0;
			HANDLE snap_shot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);

			if (snap_shot == INVALID_HANDLE_VALUE) {
				return process_id; // Возвращаем 0 в случае ошибки
			}

			PROCESSENTRY32W entry;
			entry.dwSize = sizeof(PROCESSENTRY32W); // Устанавливаем размер структуры

			// Начинаем с первого процесса
			if (Process32FirstW(snap_shot, &entry)) {
				// Сравниваем имя процесса
				if (_wcsicmp(process_name, entry.szExeFile) == 0) {
					process_id = entry.th32ProcessID; // Сохраняем ID процесса
				}
				else {
					// Ищем дальше
					while (Process32NextW(snap_shot, &entry)) {
						if (_wcsicmp(process_name, entry.szExeFile) == 0) {
							process_id = entry.th32ProcessID; // Сохраняем ID процесса
							break;
						}
					}
				}
			}

			CloseHandle(snap_shot); // Закрываем дескриптор
			return process_id; // Возвращаем ID процесса или 0, если не найден
		}

		static std::uintptr_t get_module_base(const DWORD pid, const wchar_t* module_name)
		{
			std::uintptr_t module_base = 0;

			HANDLE snap_shot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
			if (snap_shot == INVALID_HANDLE_VALUE)
				return module_base;

			MODULEENTRY32W entry = {};
			entry.dwSize = sizeof(decltype(entry));

			if (Module32FirstW(snap_shot, &entry) == TRUE) {
				if (wcsstr(module_name, entry.szModule) != nullptr)
					module_base = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
				else
				{
					while (Module32NextW(snap_shot, &entry) == TRUE)
					{
						module_base = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
						break;
					}
				}

			}

			CloseHandle(snap_shot);

			return module_base;
		}
};
inline Memory m;

/*namespace driver
{
	static HANDLE driver_handle = nullptr;
	extern uint64_t BaseAddress;

	namespace codes {
		// установка драйвера
		constexpr ULONG attach =
			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x696, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

		// чтение памяти
		constexpr ULONG read =
			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x697, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

		// запись в память
		constexpr ULONG write =
			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x698, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
	}

	// реквест между юм и км
	struct Request {
		HANDLE process_id;

		PVOID target;
		PVOID buffer;

		SIZE_T size;
		SIZE_T return_size;
	};

	inline bool attach_to_process(HANDLE driver_handle, const DWORD pid) {
		Request r;
		r.process_id = reinterpret_cast<HANDLE>(pid);

		return DeviceIoControl(driver_handle, codes::attach, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}



	template <class T>
	void write_memory(HANDLE driver_handle, const std::uintptr_t addr, const T& value)
	{
		Request r;
		r.target = reinterpret_cast<PVOID>(addr);
		r.buffer = (PVOID)&value;
		r.size = sizeof(T);

		DeviceIoControl(driver_handle, codes::write, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}

	template <class T>
	T read_memory(const std::uintptr_t addr) {
		T temp = {};

		Request r;
		r.target = reinterpret_cast<PVOID>(addr);
		r.buffer = &temp;
		r.size = sizeof(T);

		DeviceIoControl(driver_handle, codes::read, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

		return temp;
	}

	template <class T>
	T read_memoryChain(const std::uintptr_t addr, const std::vector<std::uintptr_t>& offsets) {
		T result = {};

		Request r;
		r.target = reinterpret_cast<PVOID>(addr + offsets.at(0));
		r.buffer = &result;
		r.size = sizeof(T);

		DeviceIoControl(driver_handle, codes::read, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

		for (int i = 1; i < offsets.size(); i++) {
			r.target = reinterpret_cast<PVOID>(result + offsets.at(i));
			DeviceIoControl(driver_handle, codes::read, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
			result = *reinterpret_cast<T*>(r.buffer);
		}

		return result;
	}


	inline void set_driver_handle(HANDLE handle) {
		driver_handle = handle;
	}

	inline void set_base_adress(uintptr_t adress) {
		BaseAddress = adress;
	}
}

static DWORD get_process_id(const wchar_t* process_name) {
	DWORD process_id = 0;
	HANDLE snap_shot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);

	if (snap_shot == INVALID_HANDLE_VALUE) {
		return process_id; // Возвращаем 0 в случае ошибки
	}

	PROCESSENTRY32W entry;
	entry.dwSize = sizeof(PROCESSENTRY32W); // Устанавливаем размер структуры

	// Начинаем с первого процесса
	if (Process32FirstW(snap_shot, &entry)) {
		// Сравниваем имя процесса
		if (_wcsicmp(process_name, entry.szExeFile) == 0) {
			process_id = entry.th32ProcessID; // Сохраняем ID процесса
		}
		else {
			// Ищем дальше
			while (Process32NextW(snap_shot, &entry)) {
				if (_wcsicmp(process_name, entry.szExeFile) == 0) {
					process_id = entry.th32ProcessID; // Сохраняем ID процесса
					break;
				}
			}
		}
	}

	CloseHandle(snap_shot); // Закрываем дескриптор
	return process_id; // Возвращаем ID процесса или 0, если не найден
}

static std::uintptr_t get_module_base(const DWORD pid, const wchar_t* module_name)
{
	std::uintptr_t module_base = 0;

	HANDLE snap_shot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
	if (snap_shot == INVALID_HANDLE_VALUE)
		return module_base;

	MODULEENTRY32W entry = {};
	entry.dwSize = sizeof(decltype(entry));

	if (Module32FirstW(snap_shot, &entry) == TRUE) {
		if (wcsstr(module_name, entry.szModule) != nullptr)
			module_base = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
		else
		{
			while (Module32NextW(snap_shot, &entry) == TRUE)
			{
				module_base = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
				break;
			}
		}

	}

	CloseHandle(snap_shot);

	return module_base;
}*/