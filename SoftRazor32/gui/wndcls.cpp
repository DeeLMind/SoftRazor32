#include "../sr_pch.h"

static PCLASS_TABLE ClassTableHeader = NULL;
static PCLASS_TABLE LastClassTable = NULL;

BOOL AddClassTable(HWND hWnd, PVOID pClass)
{
  if (!pClass) return FALSE;

  PCLASS_TABLE ptmp = (PCLASS_TABLE)malloc(sizeof(CLASS_TABLE));

  if (!ptmp) return FALSE;

  ptmp->hWindow = hWnd;
  ptmp->pClass = pClass;
  ptmp->Next = NULL;

  if (LastClassTable)
  {
    LastClassTable->Next = ptmp;
    LastClassTable = ptmp;
  }
  else
  {
    ClassTableHeader = ptmp;
    LastClassTable = ptmp;
  }

  return TRUE;
}

PVOID GetClassPointer(HWND hWnd)
{
  PCLASS_TABLE PCT = ClassTableHeader;

  while (PCT)
  {
    if (PCT->hWindow == hWnd) return PCT->pClass;
    PCT = PCT->Next;
  }

  return NULL;
}

BOOL DelClassTable(HWND hWnd)
{
  PCLASS_TABLE PCT = ClassTableHeader;
  PCLASS_TABLE PCTL = ClassTableHeader; // 0

  if (!PCT) return FALSE;

  if (PCT->hWindow == hWnd) //Header
  {
    /* 如果最后指针等于表头 */
    if (LastClassTable == ClassTableHeader)
    {
      ClassTableHeader = ClassTableHeader->Next;
      LastClassTable = LastClassTable->Next;
    }
    else
    {
      ClassTableHeader = ClassTableHeader->Next;
    }

    free(PCT);
    return TRUE;
  }

  PCT = PCT->Next;  // 下一个结构 1

  while (PCT)
  {
    if (PCT->hWindow == hWnd)
    {
      PCTL->Next = PCT->Next;

      if (LastClassTable == PCT) //if LastClassTable == PCT , PCT->Next == NULL
        LastClassTable = PCTL;

      free(PCT);
      return TRUE;
    }

    PCTL = PCT;
    PCT = PCT->Next;
  }

  return FALSE;
}

MDIForm::~MDIForm()
{
  if (this->m_hWnd)
  {
    DestroyWindow(this->m_hWnd);
    this->m_hWnd = NULL;
  }
}

inline COLORREF QueryForeColor(signed char chType)
{
  if (chType < 0)
    return (COLORREF)0;
  else
    return crAsmFGC[chType];
}

inline COLORREF QueryBackColor(signed char chType)
{
  if (chType < 0)
    return (COLORREF)0;
  else
    return crAsmBGC[chType];
}

int DrawCasmA(HDC hDC, PCHAR lpAsmText, size_t bufmax, LONG sspace, LPCRECT lprc, UINT format)
{
  COLORREF crOrigFGC = GetTextColor(hDC);
  COLORREF crOrigBGC = GetBkColor(hDC);
  COLORREF crBGC;
  PCHAR lpText = lpAsmText;
  int count = 0;
  size_t i, len;
  SIZE ssize;
  RECT rect = *lprc;

  if (lpText == NULL) return 0;

  for (i = 0; i < bufmax; i++)
  {
    if (lpText[i] == CD_NULL) break;
    //获取并设置前景色
    SetTextColor(hDC, QueryForeColor(lpText[i]));  
    crBGC = QueryBackColor(lpText[i]);

    if (CHK_NOTEMPTY(crBGC, 0xFF000000))
      SetBkMode(hDC, TRANSPARENT);  //设置透明背景样式
    else
    {
      SetBkMode(hDC, OPAQUE);//设置非透明背景样式
      SetBkColor(hDC, crBGC);  //设置背景色
    }

    i++;    //移动索引到字符开始
    if ((len = strnlen(&lpText[i], bufmax - i)) == 0) break;  //取字符长度
    if (i + len >= bufmax) break;  //判断是否超出最大值
    GetTextExtentPointA(hDC, &lpText[i], len, &ssize);
    rect.right = rect.left + ssize.cx /*+ wspace*/;
    if (rect.right > lprc->right) rect.right = lprc->right;
    DrawTextA(hDC, &lpText[i], len, &rect, format);
    count++;

    if (rect.right >= lprc->right) break;
    rect.left = rect.right;
    i += len;
  }
  SetTextColor(hDC, crOrigFGC);
  SetBkColor(hDC, crOrigBGC);
  return count;
}

int DrawCasmW(HDC hdc, PWCHAR lpcasm, size_t bufmax, LONG wspace, LPCRECT lprc, UINT format)
{
  COLORREF fc = GetTextColor(hdc);
  COLORREF bc = GetBkColor(hdc);
  COLORREF cbc;
  PWCHAR pwcs = lpcasm;
  int dcount = 0;
  size_t i;
  size_t wlen;
  SIZE wsize;
  RECT rect = *lprc;

  if (!pwcs) return 0;

  for (i = 0; i < bufmax; i++)
  {
    if (pwcs[i] == 0) break;
    SetTextColor(hdc, QueryForeColor(pwcs[i]));  //获取并设置前景色
    cbc = QueryBackColor(pwcs[i]);

    if (CHK_NOTEMPTY(bc, 0xFF000000))
      SetBkMode(hdc, TRANSPARENT);//设置背景样式
    else
    {
      SetBkMode(hdc, OPAQUE);//设置背景样式
      SetBkColor(hdc, cbc);  //设置背景色
    }

    i++;    //移动索引到字符开始
    if (!(wlen = wcslen(&pwcs[i]))) break;  //取字符长度
    if (i + wlen >= bufmax) break;  //判断是否超出最大值
    GetTextExtentPointW(hdc, &pwcs[i], wlen, &wsize);
    rect.right = rect.left + wsize.cx /*+ wspace*/;
    if (rect.right > lprc->right) rect.right = lprc->right;
    DrawTextW(hdc, &pwcs[i], wlen, &rect, format);
    dcount++;

    if (rect.right >= lprc->right) break;
    rect.left = rect.right;
    i += wlen;
  }
  SetTextColor(hdc, fc);
  SetBkColor(hdc, bc);
  return dcount;
}

HWND MDIForm::GetMainhWnd()
{
  return this->m_hWnd;
}

BOOL MDIForm::SetTop()
{
  return SetWindowPos(this->m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

BOOL MDIForm::DestroyForm()
{
  BOOL ret;

  if (!this->m_hWnd) return FALSE;
  ret = DestroyWindow(this->m_hWnd);
  this->m_hWnd = NULL;
  return ret;
}