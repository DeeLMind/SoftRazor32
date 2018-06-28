#include "../sr_pch.h"

const GUID vbclsid = { 0xFCFB3D2E, 0xA0FA, 0x1068, 0xA7, 0x38, 0x08, 0x00, 0x2B, 0x33, 0x71, 0xB5 };

HMODULE DllBaseAddr = NULL;
DWORD PAA = (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE);
DWORD TAA = (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE);
DWORD LastCode = 0;
WORD NtVer = NULL;
WCHAR DllDir[MAX_PATH];

COLORREF crAsmFGC[128] =
{
  RGB(0, 0, 0),             //CD_NULL
  RGB(0, 0, 0),             //CD_IGNORE
  RGB(24, 24, 24),          //CD_INST
  RGB(0, 0, 255),           //CD_STACK
  RGB(128, 64, 0),          //CD_FMS
  RGB(255, 0, 0),           //CD_CJMP
  RGB(0, 0, 0),             //CD_JMP
  RGB(32, 0, 0),            //CD_CALL
  RGB(0, 32, 0),            //CD_RET
  RGB(255, 64, 64),         //CD_PRIVINST
  RGB(64, 255, 64),         //CD_PF
  RGB(250, 0, 0),           //CD_PFLOCK
  RGB(0, 250, 0),           //CD_PFREP
  RGB(0, 0, 0),             //CD_DEFCLR
  RGB(64, 128, 230),        //CD_INT
  RGB(0, 0, 250),           //CD_SYS
  RGB(160, 160, 0),         //CD_DATA
  RGB(160, 32, 32),         //CD_TEXT
  RGB(32, 255, 160),        //CD_SYMBOL
  RGB(255, 255, 255),       //CD_ERROR
  RGB(128, 0, 128),         //CD_GPRS
  RGB(0, 128, 128),         //CD_FSRS
  RGB(255, 0, 255),         //CD_SEG
  RGB(255, 0, 255),         //CD_SSRS
  RGB(32, 64, 160),         //CD_MEMSTK
  RGB(32, 64, 160),         //CD_MEMOTH
  RGB(64, 160, 96),         //CD_IMM
  RGB(128, 0, 0),           //CD_IMMADDR
  RGB(128, 0, 255),         //CD_DECIMAL
  RGB(64, 0, 128),          //CD_DISTANCE
  RGB(0, 0, 128),           //CD_SIZE
  RGB(255, 0, 0),           //CD_CHANGED
};

COLORREF crAsmBGC[128] =
{
  RGB(0, 0, 0),             //CD_NULL
  RGB(0, 0, 0),             //CD_IGNORE
  SR_TRANSPARENT,           //CD_INST
  SR_TRANSPARENT,           //CD_STACK
  SR_TRANSPARENT,           //CD_FMS
  RGB(255, 255, 64),        //CD_CJMP
  RGB(255, 255, 0),         //CD_JMP
  RGB(32, 255, 255),        //CD_CALL
  RGB(255, 32, 255),        //CD_RET
  RGB(128, 255, 255),       //CD_PRIVINST
  RGB(255, 160, 255),       //CD_PF
  RGB(32, 250, 170),        //CD_PFLOCK
  RGB(32, 64, 170),         //CD_PFREP
  SR_TRANSPARENT,           //CD_DEFCLR
  SR_TRANSPARENT,           //CD_INT
  RGB(128, 128, 128),       //CD_SYS
  SR_TRANSPARENT,           //CD_DATA
  SR_TRANSPARENT,           //CD_TEXT
  SR_TRANSPARENT,           //CD_SYMBOL
  RGB(255, 0, 0),           //CD_ERROR
  SR_TRANSPARENT,           //CD_GPRS
  SR_TRANSPARENT,           //CD_FSRS
  SR_TRANSPARENT,           //CD_SEG
  SR_TRANSPARENT,           //CD_SSRS
  RGB(128, 128, 0),         //CD_MEMSTK
  SR_TRANSPARENT,           //CD_MEMOTH
  SR_TRANSPARENT,           //CD_IMM
  SR_TRANSPARENT,           //CD_IMMADDR
  SR_TRANSPARENT,           //CD_DECIMAL
  SR_TRANSPARENT,           //CD_DISTANCE
  SR_TRANSPARENT,           //CD_SIZE
  SR_TRANSPARENT,           //CD_CHANGED
};

HANDLE ht_main = NULL;
UINT tid_main = NULL;
