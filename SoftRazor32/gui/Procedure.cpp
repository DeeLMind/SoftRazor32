#include "../sr_pch.h"

BOOL CheckMessageQueue()
{
  MSG msg;

  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
  {
    if (msg.message == WM_QUIT)
      return FALSE;
    
    if (mf->GetDbgState() != DS_Idle)
    {
      switch (msg.message)
      {
      /* 不处理的消息 */
      case WM_KEYDOWN:
      case WM_KEYUP:
      case WM_LBUTTONDOWN:
      case WM_LBUTTONUP:
      case WM_RBUTTONDOWN:
      case WM_RBUTTONUP:
      case WM_CLOSE:
        continue;
      default:
        break;
      }
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  if (mf || mf->hw_main)
    UpdateWindow(mf->hw_main);
  return TRUE;
}

COLORREF scGetForeColor(DWORD sc)
{
  COLORREF ret = 0;

  if ((sc & IMAGE_SCN_MEM_WRITE) == IMAGE_SCN_MEM_WRITE) 
    ret |= RGB(0,200,0);

  if ((sc & IMAGE_SCN_MEM_SHARED) == IMAGE_SCN_MEM_SHARED)
    ret = RGB(255,0,255);

  if ((sc & IMAGE_SCN_MEM_DISCARDABLE) == IMAGE_SCN_MEM_DISCARDABLE)
    ret = RGB(255,0,0);
  
  return ret;
}

COLORREF scGetBackColor(DWORD sc)
{
  COLORREF ret = RGB(255,255,255);

  if ((sc & IMAGE_SCN_CNT_CODE) == IMAGE_SCN_CNT_CODE || (sc & IMAGE_SCN_MEM_EXECUTE) == IMAGE_SCN_MEM_EXECUTE)
    ret = RGB(100,80,180);

  return ret;
}

void CommonUpdatelParam(HWND hlv)
{
  LV_ITEM lvi;
  int nCount = SendMessage(hlv,LVM_GETITEMCOUNT,0,0);

  lvi.mask = LVIF_PARAM;
  lvi.iSubItem = 0;
  lvi.iItem = 0;

  while(nCount > 0)
  {
    lvi.lParam = lvi.iItem;
    SendMessage(hlv,LVM_SETITEM,0,(LPARAM)&lvi);
    lvi.iItem++;
    nCount--;
  }
}

void ShowCurrentFocus(HWND hlv)
{
  LV_ITEM lvi;
  WCHAR buffer[256];
  int nIndex = SendMessage(hlv,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
  lvi.iItem = nIndex;
  lvi.iSubItem = 0;
  lvi.mask = LVIF_TEXT;
  lvi.pszText = buffer;
  lvi.cchTextMax = 256;
  SendMessage(hlv,LVM_GETITEM,0,(LPARAM)&lvi);
}