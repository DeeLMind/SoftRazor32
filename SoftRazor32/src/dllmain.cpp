#include "../sr_pch.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// The following symbol used to force inclusion of this module for _USRDLL
#ifdef _X86_
extern "C" { int _afxForceUSRDLL; }
#else
extern "C" { int __afxForceUSRDLL; }
#endif

GlobalConfig GC;

BOOL WINAPI DllMain(HMODULE hModule,DWORD call_type,LPVOID lpReserved)
{
  switch (call_type)
  {
  case DLL_PROCESS_ATTACH:
  {
    PWCHAR pos;

    setlocale(LC_ALL, "");
    InitCommonControls();

    if (!AfxWinInit(hModule, NULL, NULL /*::GetCommandLine()*/, SW_NORMAL))
    {
      LastCode = 1;
      return FALSE;
    }

    if (NtVer < _WIN32_WINNT_WINXP)
    {
      LastCode = 2;
      MessageBox(0, NVE_TEXT, ERR_TITLE, MB_ICONERROR);
      return FALSE;
    }

    PAA |= (NtVer >= _WIN32_WINNT_VISTA) ? 0xFFFF : 0x0FFF;
    TAA |= (NtVer >= _WIN32_WINNT_VISTA) ? 0xFFFF : 0x03FF;

    GC.LoadAllConfig();

    GetModuleFileNameW(hModule, DllDir, MAX_PATH);
    pos = wcschr(DllDir, L'\\');
    if (pos) *pos = 0;
    wcscat_s(DllDir, MAX_PATH, L"\\DBIB");
    //REX_DebugInfoDatabase::SetGlobalWorkDir(DllDir);
    if (pos) *pos = 0;
  }
    break;
  case DLL_PROCESS_DETACH:
    if (mf) mf->Release();
    AfxWinTerm();
    break;
  }
  return TRUE;
}