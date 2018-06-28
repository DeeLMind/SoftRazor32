#include "../sr_pch.h"

FloatDialog::~FloatDialog()
{
  if (this->m_hWnd)
  {
    DestroyWindow(this->m_hWnd);
    this->m_hWnd = NULL;
  }
}

HWND FloatDialog::GetMainhWnd()
{
  return this->m_hWnd;
}

BOOL FloatDialog::SetTop()
{
  return SetWindowPos(this->m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

BOOL FloatDialog::DestroyForm()
{
  BOOL ret;

  if (!this->m_hWnd) return FALSE;
  ret = DestroyWindow(this->m_hWnd);
  this->m_hWnd = NULL;
  return ret;
}