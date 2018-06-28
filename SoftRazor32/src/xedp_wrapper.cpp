#include "../sr_pch.h"

UINT XEDP_Assemble(struct XEDPARSE * pXEDP)
{
  if (XEDParseAssemble(pXEDP) == XEDPARSE_OK)
    return pXEDP->dest_size;
  return 0;
}