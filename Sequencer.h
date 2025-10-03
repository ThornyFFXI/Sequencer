#ifndef __ASHITA_Sequencer_H_INCLUDED__
#define __ASHITA_Sequencer_H_INCLUDED__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "Ashita.h"

class Sequencer : IPlugin
{
private:
    IAshitaCore* m_AshitaCore;
    ILogManager* m_LogManager;
    IDirect3DDevice8* m_Direct3DDevice;
    uint32_t m_PluginId;
    uint32_t mActiveSequence;

public:
    const char* GetName(void) const override
    {
        return "Sequencer";
    }
    const char* GetAuthor(void) const override
    {
        return "Thorny";
    }
    const char* GetDescription(void) const override
    {
        return "Modifies packet sequence IDs to prevent actions from Ashitacast/LuAshitacast from double-registering.";
    }
    const char* GetLink(void) const override
    {
        return "Insert link here.";
    }
    double GetVersion(void) const override
    {
        return 1.01f;
    }
    int32_t GetPriority(void) const override
    {
        return 0;
    }
    uint32_t GetFlags(void) const override
    {
        return (uint32_t)Ashita::PluginFlags::None;
    }
	
    bool Initialize(IAshitaCore* core, ILogManager* logger, const uint32_t id) override;
    void Release(void) override;
    void PreProcess(bool outgoing, uint32_t size, uint8_t* data);
    void PostProcess(bool outgoing, uint32_t size, uint8_t* data);
};
#endif