#include "Cheat.h"

void Cheat::UpdateList()
{
	while (g.Run)
	{
		std::vector<Player> TempPlayer;
        std::vector<Exfil> TempExfil;
        Sleep(5);

        if (!EFT.InitAddress())
            continue;

        // PlayerList
        uintptr_t RegisteredPlayer = m.read_memory<uintptr_t>(EFT.localGameWorld + Offsets::LocalGameWorld::RegisteredPlayers);
        UnityList PlayerArray = m.read_memory<UnityList>(RegisteredPlayer + 0x10);

        if (!PlayerArray.Base || PlayerArray.Count <= 0)
            continue;

        local.ptr = m.read_memory<uintptr_t>(PlayerArray.Base + 0x20);

        for (int i = 1; i < PlayerArray.Count; i++)
        {
            Player player{};
            player.ptr = m.read_memory<uintptr_t>(PlayerArray.Base + 0x20 + (i * 0x8));

            if (!player.Update())
                continue;

            TempPlayer.push_back(player);
        }

        // Exfil
        uintptr_t ExfilController = m.read_memory<uintptr_t>(EFT.localGameWorld + Offsets::LocalGameWorld::ExfilController);
        uintptr_t ExfilArray = m.read_memory<uintptr_t>(ExfilController + 0x20);
        //UnityList ExfilArray = m.read_memory<UnityList>(ExfilController + 0x10); // UnityList->Count is don't work (i don't checked CE/Reclass)

        for (int j = 0; j < 32; j++)
        {
            Exfil exfil{};
            exfil.ptr = m.read_memory<uintptr_t>(ExfilArray + 0x20 + (j * 0x8));

            if (exfil.ptr == NULL)
                break;
            else if (!exfil.Update())
                continue;

            TempExfil.push_back(exfil);
        }

        EntityList = TempPlayer;
        ExfilList = TempExfil;
        TempPlayer.clear();
        TempExfil.clear();

        Sleep(1000);
	}
}
