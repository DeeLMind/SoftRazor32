#include "../sr_pch.h"
#include "../resource.h"

MainForm  * mf = NULL;

UINT WINAPI DbgWndThread(PVOID lParam)
{
  BOOL gmret;
  MSG dwmsg;

  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
  InitCommonControls();
  SR_SetHandler();

  if (mf == NULL)
  {
    mf = MainForm::GetInstance(DllBaseAddr);

    if (mf == NULL)
    {
      SR_RemoveHandler();
      _endthreadex(1);
      return 1;
    }
  }

  mf->CreateForm(WN_MAIN, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_MAXIMIZE, DllBaseAddr);

  if (mf->GetMainWindow() == NULL)
  {
    SR_RemoveHandler();
    _endthreadex(1);
    return 1;
  }

  while ((gmret = GetMessage(&dwmsg, 0, 0, 0)) != 0)
  {
    if (gmret == -1)
    {
      SR_RemoveHandler();
      _endthreadex(1);
      return 1;
    }

    if (mf->GetDbgState() != DS_Idle)
    {
      switch (dwmsg.message)
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
    TranslateMessage(&dwmsg);
    DispatchMessage(&dwmsg);
  }

  SR_RemoveHandler();
  _endthreadex(0);
  return 0;
}

BOOL WINAPI LaunchDebugger(ULONG LaunchParam)
{
  if (ht_main != NULL) return FALSE;
  ht_main = (HANDLE)_beginthreadex(0, 4194304, &DbgWndThread, NULL, 0, &tid_main);
  return (ht_main == NULL) ? FALSE : TRUE;
}

BOOL WINAPI TerminateDebugger(BOOL blEnforce)
{
  if (ht_main == 0) return FALSE;
  if (mf) mf->Release();
  /*
  
  
  */
  ht_main = NULL;
  tid_main = 0;
  return TRUE;
}

