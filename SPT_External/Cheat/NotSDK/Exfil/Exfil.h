#pragma once
#include "../NotSDK.h"

/*
	[ Status ]
	1 : Close
	2 : NotReady
	4 : Open
	6 : Ready (Labのエレベーターとかがこれだったはず
*/

class Exfil
{
private:
	bool IsValid();
	Vector3 GetTransformPosition(uintptr_t transform);
public:
	uintptr_t ptr;

	int status;
	Vector3 pos;
	char Name[256]{};

	bool Update()
	{
		if (!IsValid())
			return false;

		status = m.read_memory<int>(ptr + 0xA8);

		if (status == 1)
			return false;

		uintptr_t TransformInternal = m.read_memoryChain<std::uintptr_t>(ptr, { 0x10, 0x30, 0x30, 0x8, 0x28, 0x10 });
		pos = GetTransformPosition(TransformInternal);

		if (pos == Vector3(0.f, 0.f, 0.f))
			return false;

		uintptr_t eSetting = m.read_memory<uintptr_t>(ptr + 0x58);
		uintptr_t NamePtr = m.read_memory<uintptr_t>(eSetting + 0x10);

		if (!NamePtr)
			return false;

		int length = m.read_memory<int>(NamePtr + 0x10);

		for (int i = 0; i < length; i++)
			Name[i] = m.read_memory<char>(NamePtr + 0x14 + (i * 0x2));

		return true;
	}
};
