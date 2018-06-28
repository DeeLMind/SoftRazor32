#include "../sr_pch.h"

IDirectMusicPerformance *   performance = NULL;
IDirectMusicSegment     *   segment     = NULL;
IDirectMusicLoader      *   loader      = NULL;

bool PlayMIDI(PBYTE data,UINT size)
{
    DMUS_OBJECTDESC desc = {0};

    desc.dwSize      = sizeof(DMUS_OBJECTDESC);
    desc.guidClass   = CLSID_DirectMusicSegment;
    desc.pbMemData   = data;
    desc.llMemLength = size;
    desc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_MEMORY;

    if(FAILED(CoInitialize(0))) return false;
    if(FAILED(CoCreateInstance(CLSID_DirectMusicPerformance,0,CLSCTX_INPROC,
        IID_IDirectMusicPerformance,(void**)&performance))) return false;
    if(FAILED(CoCreateInstance(CLSID_DirectMusicLoader,0,CLSCTX_INPROC,
        IID_IDirectMusicLoader,(void**)&loader))) return false;
    if(FAILED(loader->GetObject(&desc,IID_IDirectMusicSegment,(void**)&segment))) return false;
    if(FAILED(performance->Init(0,0,0))) return false;
    if(FAILED(performance->AddPort(0))) return false;
    if(FAILED(segment->SetParam(GUID_StandardMIDIFile,-1,0,0,performance))) return false;
    if(FAILED(segment->SetParam(GUID_Download,-1,0,0,performance))) return false;
    if(FAILED(segment->SetRepeats(DMUS_SEG_REPEAT_INFINITE))) return false;
    if(FAILED(performance->PlaySegment(segment,0,0,0))) return false;
    return true;
}

void StopMIDI()
{
    if(segment)
    {
        if(performance)
        {
            performance->Stop(segment,0,0,0);
            segment->SetParam(GUID_Unload,-1,0,0,(void**)performance);
        }
        segment->Release();
        segment=0;
    }
    if(loader)
    {
        loader->Release();
        loader=0;
    }
    if(performance)
    {
        performance->CloseDown();
        performance->Release();
        performance=0;
    }
    CoUninitialize();
}