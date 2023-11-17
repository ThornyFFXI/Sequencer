#include "Sequencer.h"
#pragma comment(lib, "psapi.lib")
#include <psapi.h>
#include "thirdparty/detours.h"
#pragma comment(lib, "thirdparty/detours.lib")

Sequencer* pPlugin = 0;

extern "C"
{
    auto Ashita_ParsePackets = static_cast<uint32_t(__thiscall*)(void*, bool, uint32_t, uint8_t*)>(nullptr);
}

uint32_t __fastcall Mine_ParsePackets(void* pPacketManager, void* pEDX, bool outgoing, uint32_t size, uint8_t* data)
{
    pPlugin->PreProcess(outgoing, size, data);
    size = Ashita_ParsePackets(pPacketManager, outgoing, size, data);
    pPlugin->PostProcess(outgoing, size, data);

    return size;
}

__declspec(dllexport) IPlugin* __stdcall expCreatePlugin(const char* args)
{
    UNREFERENCED_PARAMETER(args);

    return (IPlugin*)(new Sequencer());
}

__declspec(dllexport) double __stdcall expGetInterfaceVersion(void)
{
    return ASHITA_INTERFACE_VERSION;
}

bool Sequencer::Initialize(IAshitaCore* core, ILogManager* logger, const uint32_t id)
{
    pPlugin      = this;
    m_AshitaCore = core;
    m_LogManager = logger;
    m_PluginId   = id;

    // Get the address to ashita's detoured packet handler..
    auto ptrEncryptBuffer     = m_AshitaCore->GetPointerManager()->Get("packets.encryptbuffer");
    DWORD trampolineOffset    = *(int32_t*)((uint8_t*)ptrEncryptBuffer + 1);
    uintptr_t trampolinedCall = ptrEncryptBuffer + 5 + trampolineOffset;

    // Offset to ashita's packet parsing function..
    uintptr_t parseCall       = trampolinedCall + 0x20;
    uintptr_t parseAddress    = parseCall + 5 + *(int32_t*)((uint8_t*)parseCall + 1);
    Ashita_ParsePackets       = (decltype(Ashita_ParsePackets)(parseAddress));

    // Detour ashita's packet parsing function..
    ::DetourTransactionBegin();
    ::DetourUpdateThread(::GetCurrentThread());
    ::DetourAttach(&(PVOID&)Ashita_ParsePackets, Mine_ParsePackets);
    ::DetourTransactionCommit();

    return true;
}

void Sequencer::Release(void)
{
    // Restore packet parsing function to normal.
    ::DetourTransactionBegin();
    ::DetourUpdateThread(::GetCurrentThread());
    ::DetourDetach(&(PVOID&)Ashita_ParsePackets, Mine_ParsePackets);
    ::DetourTransactionCommit();
}

void Sequencer::PreProcess(bool outgoing, uint32_t size, uint8_t* data)
{
    if (!outgoing)
        return;

    mActiveSequence = 0xFFFFFFFF;
    auto offset = outgoing ? 0 : 0x1C;
    while (offset < size)
    {
        auto packet = data + offset;
        auto pid   = (uint16_t)(*(uint16_t*)packet & 0x01FF);
        auto psize = (uint32_t)(*(uint16_t*)packet >> 0x09) * 0x04;

        // Detect action packets in the initial game-provided chunks.
        if ((pid == 0x1A) || (pid == 0x37))
            mActiveSequence = *((uint16_t*)(packet + 2));

        offset += psize;
    }
}

void Sequencer::PostProcess(bool outgoing, uint32_t size, uint8_t* data)
{
    if ((mActiveSequence == 0xFFFFFFFF) || (!outgoing))
        return;

    auto offset = outgoing ? 0 : 0x1C;
    while (offset < size)
    {
        auto packet = data + offset;
        auto pid   = (uint16_t)(*(uint16_t*)packet & 0x01FF);
        auto psize = (uint32_t)(*(uint16_t*)packet >> 0x09) * 0x04;

        // Detect action and equip packets in the post-ashita chunks.
        if ((pid == 0x1A) || (pid == 0x37) || (pid == 0x50) || (pid == 0x51))
        {
            #ifdef DEBUG_MODE
            auto sequence = *((uint16_t*)(packet + 2));
            if (sequence != mActiveSequence)
            {
                m_AshitaCore->GetChatManager()->Writef(0, false, "Edited sequence.  %04X -> %04X", sequence, mActiveSequence);
                *((uint16_t*)(packet + 2)) = mActiveSequence;
            }
            #else
            *((uint16_t*)(packet + 2)) = mActiveSequence;
            #endif
        }

        offset += psize;
    }
}