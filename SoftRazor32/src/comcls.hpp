#ifndef _SR_INC_COMMON_H_
#include "common.h"
#endif

#include "g_type.h"
#include "../../HMM/hmm.h"

#define MAX_DEBUGIF             32

#define CSDI_DBMAGIC            0x42445343        //"CSDB"
#define CSDI_FLMAGIC            0x4C465343        //"CSFL"
#define CSDI_SDMAGIC            0x44535343        //"CSSD"
#define CSDI_BDMAGIC            0x44425343        //"CSBD"
#define CSDI_VER0               0x0001
#define CSDI_VER1               0x0001

#define CSDISD_UNKNOWN          0                 //unknown
#define CSDISD_BYTE             1                 //data byte
#define CSDISD_WORD             2                 //data word
#define CSDISD_DWORD            3                 //data dword
#define CSDISD_QWORD            4                 //data qword
#define CSDISD_SINFLT           5                 //data single float
#define CSDISD_DOUFLT           6                 //data double float
#define CSDISD_NCODE            7                 //Instruction
#define CSDISD_PCODE            8                 //P-Code
#define CSDISD_IL               9                 //Intermediate Language
#define CSDISD_CHAR             10
#define CSDISD_ASCII            11                //ascii text
#define CSDISD_UTF8             12                //utf8 text
#define CSDISD_UTF16            13                //utf16 text
#define CSDISD_UDSTRUCT         14                //user defined struct
#define CSDISD_OTHER            15

#define CSGETSD_TYPE(val)       ((val) & 0x000F)
#define CSGETSD_DESCR0(val)     ((val) & 0x03F0)

#define CSDB_ADFLAG_NOMD5       0x00000001U


class GlobalConfig
{
private:
  WCHAR           WorkDir[MAX_PATH];
  WCHAR           CfgFile[MAX_PATH];

public:
  DWORD           fdeval;
  DWORD           ldeval;

  GlobalConfig();
  void            LoadAllConfig();
  BOOL            SaveAllConfig();
  void            ResetAllConfig();
  BOOL            LoadConfig(DWORD uFlag);
  BOOL            SaveConfig(DWORD uFlag);
  BOOL            ReadConfig(LPCTCH AppName, LPCTCH KeyName, PWCHAR lpBuffer, UINT nMax);
  BOOL            WriteConfig(LPCTCH AppName, LPCTCH KeyName, LPCTCH lpBuffer);
  LPCTCH          GetWorkDirectory();
};

class MemoryManager
{
private:
  static PMEMORY_MODIFY         MM_HEADER;

public:
  MemoryManager() {}
  ~MemoryManager() {}

  /*查询内存更改*/
  UINT QueryMemoryModify(PBYTE MemAddr, UINT MemSize);
  /*尝试修改内存*/
  BOOL TryModifyMemory(PBYTE destAddr, PBYTE pData, UINT DataSize);
  /*还原内存*/
  BOOL RevertMemory(PBYTE MemAddr, UINT DataSize);
  /*还原全部内存*/
  BOOL RevertAllMemory(PBYTE MemAddr);
  /*获取原始内存*/
  UINT GetOriginalMemory(PBYTE MemAddr, PBYTE destAddr, UINT MemAndDestSize);
};