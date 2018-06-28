#include "../sr_pch.h"

GlobalConfig::GlobalConfig() : fdeval(0), ldeval(0)
{
  UINT i,los = 0;
  WCHAR stmp[MAX_PATH];

  GetModuleFileName(DllBaseAddr,stmp,MAX_PATH);

  for (i = 0; i < MAX_PATH;i++)
  {
    if (!stmp[i]) break;
    if (stmp[i] == _T('\\')) los = i;
  }

  stmp[los] = 0;
  swprintf_s(WorkDir, MAX_PATH, L"%s\\", stmp);
  swprintf_s(CfgFile, MAX_PATH, L"%s\\srdbg.ini", stmp);
}

void GlobalConfig::LoadAllConfig()
{
  BOOL rval = TRUE;
  HANDLE hFile;
  DWORD dwtmp;
  WCHAR stmp[MAX_PATH];

  hFile = CreateFile(CfgFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (hFile == INVALID_HANDLE_VALUE)
  {
    ResetAllConfig();
    SaveAllConfig();
  }
  else
    CloseHandle(hFile);

  dwtmp = GetPrivateProfileString(L"Base", L"FirstException", L"00000000", stmp, MAX_PATH, CfgFile);
  if (_stscanf_s(stmp, HEXFORMAT, &dwtmp) == 1)
    fdeval = dwtmp;
  else
    fdeval = 0;

  dwtmp = GetPrivateProfileString(L"Base", L"LastException", L"00000000", stmp, MAX_PATH, CfgFile);
  if (_stscanf_s(stmp, HEXFORMAT, &dwtmp) == 1)
    ldeval = dwtmp;
  else
    ldeval = 0;
}

BOOL GlobalConfig::SaveAllConfig()
{
  BOOL rval = TRUE;
  WCHAR stmp[MAX_PATH];

  swprintf_s(stmp, MAX_PATH, HEXFORMAT, fdeval);
  if (!WritePrivateProfileString(L"Base", L"FirstException", stmp, CfgFile))
    rval = FALSE;

  swprintf_s(stmp, MAX_PATH, HEXFORMAT, ldeval);
  if (!WritePrivateProfileString(L"Base", L"LastException", stmp, CfgFile))
    rval = FALSE;

  return rval;
}

void GlobalConfig::ResetAllConfig()
{
  fdeval = 0;
  ldeval = 0xFFFFF;
}

LPCTCH GlobalConfig::GetWorkDirectory()
{
  return WorkDir;
}