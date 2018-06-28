#include "hmm.h"

#define HEAP_STANDARD 0
#define HEAP_LAL 1
#define HEAP_LFH 2

#define HMMCHK_ENABLE_COMPACT_ALLOC(dwfl)   ((InitFlag & HMMF_ENABLE_COMPACT) && (InitFlag & HMMF_COMPACT_ALLOC))
#define HMMCHK_ENABLE_COMPACT_FREE(dwfl)    ((InitFlag & HMMF_ENABLE_COMPACT) && ((InitFlag & HMMF_COMPACT_ALLOC) == 0))

HeapMemoryManager::HeapMemoryManager(DWORD dwFlag, SIZE_T InitSize) : h_heap(0), b_lfh(FALSE), InitFlag(0), ExceptionCode(0)
{
  if (InitSize) hreset(dwFlag, InitSize);
}

HeapMemoryManager::~HeapMemoryManager()
{
  if (h_heap) HeapDestroy(h_heap);
}

void * WINAPI HeapMemoryManager::hmalloc(SIZE_T bsize)
{
  LPVOID ret;

  if (!h_heap) return NULL;
  if (bsize == 0 || bsize > _HEAP_MAXREQ) return NULL;

  if (HMMCHK_ENABLE_COMPACT_ALLOC(InitFlag))
    HeapCompact(h_heap, 0);

  ret = HeapAlloc(h_heap, 0, bsize);

  if (ret)    //不为NULL
  {
    if ((DWORD)ret < 0xC0000000U)
      return ret;
    else
      ExceptionCode = (DWORD)ret;
  }

  return NULL;
}

void * WINAPI HeapMemoryManager::hcalloc(SIZE_T bsize)
{
  LPVOID ret;
  if (!h_heap) return NULL;
  if (bsize == 0 || bsize > _HEAP_MAXREQ) return NULL;

  if (HMMCHK_ENABLE_COMPACT_ALLOC(InitFlag))
    HeapCompact(h_heap, 0);

  ret = HeapAlloc(h_heap, HEAP_ZERO_MEMORY, bsize);

  if (ret)    //不为NULL
  {
    if ((DWORD)ret < 0xC0000000U)
      return ret;
    else
      ExceptionCode = (DWORD)ret;
  }

  return NULL;
}

void * WINAPI HeapMemoryManager::hcalloc(SIZE_T bnum, SIZE_T usize)
{
  UINT64 bsize64;
  LPVOID ret;

  if (!h_heap) return NULL;
  bsize64 = bnum * usize;
  if (bsize64 == 0 || bsize64 > _HEAP_MAXREQ) return NULL;

  if (HMMCHK_ENABLE_COMPACT_ALLOC(InitFlag))
    HeapCompact(h_heap, 0);

  ret = HeapAlloc(h_heap, HEAP_ZERO_MEMORY, (SIZE_T)bsize64);

  if (ret)    //不为NULL
  {
    if ((DWORD)ret < 0xC0000000U)
      return ret;
    else
      ExceptionCode = (DWORD)ret;
  }

  return NULL;
}

void * WINAPI HeapMemoryManager::hrealloc(void * pmemblk, SIZE_T newsize)
{
  LPVOID ret;

  if (!h_heap) return NULL;
  if (pmemblk == NULL || newsize > _HEAP_MAXREQ) return NULL;

  if (newsize == 0)
  {
    HeapFree(h_heap, 0, pmemblk);
    if (HMMCHK_ENABLE_COMPACT_FREE(InitFlag))
      HeapCompact(h_heap, 0);
    return NULL;
  }

  if (HMMCHK_ENABLE_COMPACT_ALLOC(InitFlag))
    HeapCompact(h_heap, 0);

  ret = HeapReAlloc(h_heap, 0, pmemblk, newsize);

  if (ret)    //不为NULL
  {
    if ((DWORD)ret < 0xC0000000U)
      return ret;
    else
      ExceptionCode = (DWORD)ret;
  }

  return NULL;
}

void * WINAPI HeapMemoryManager::hrealloc_nom(void * pmemblk, SIZE_T newsize)
{
  LPVOID ret;

  if (!h_heap) return NULL;
  if (pmemblk == NULL || newsize > _HEAP_MAXREQ) return NULL;

  if (newsize == 0)
  {
    HeapFree(h_heap, 0, pmemblk);
    if (HMMCHK_ENABLE_COMPACT_FREE(InitFlag))
      HeapCompact(h_heap, 0);
    return NULL;
  }

  if (HMMCHK_ENABLE_COMPACT_ALLOC(InitFlag))
    HeapCompact(h_heap, 0);

  ret = HeapReAlloc(h_heap, HEAP_REALLOC_IN_PLACE_ONLY, pmemblk, newsize);

  if (ret)    //不为NULL
  {
    if ((DWORD)ret < 0xC0000000U)
      return ret;
    else
      ExceptionCode = (DWORD)ret;
  }

  return NULL;
}

void * WINAPI HeapMemoryManager::hrecalloc(void * pmemblk, SIZE_T newsize)
{
  LPVOID ret;

  if (!h_heap) return NULL;
  if (pmemblk == NULL || newsize > _HEAP_MAXREQ) return NULL;
  
  if (newsize == 0)
  {
    HeapFree(h_heap, 0, pmemblk);
    if (HMMCHK_ENABLE_COMPACT_FREE(InitFlag))
      HeapCompact(h_heap, 0);
    return NULL;
  }

  if (HMMCHK_ENABLE_COMPACT_ALLOC(InitFlag))
    HeapCompact(h_heap, 0);

  ret = HeapReAlloc(h_heap, HEAP_ZERO_MEMORY, pmemblk, newsize);

  if (ret)    //不为NULL
  {
    if ((DWORD)ret < 0xC0000000U)
      return ret;
    else
      ExceptionCode = (DWORD)ret;
  }

  return NULL;
}

void * WINAPI HeapMemoryManager::hrecalloc(void * pmemblk, SIZE_T newnum, SIZE_T usize)
{
  UINT64 bsize64;
  LPVOID ret;

  if (!h_heap) return NULL;
  bsize64 = newnum * usize;
  if (pmemblk == NULL || bsize64 > _HEAP_MAXREQ) return NULL;

  if (bsize64 == 0)
  {
    HeapFree(h_heap, 0, pmemblk);
    if (HMMCHK_ENABLE_COMPACT_FREE(InitFlag))
      HeapCompact(h_heap, 0);
    return NULL;
  }

  if (HMMCHK_ENABLE_COMPACT_ALLOC(InitFlag))
    HeapCompact(h_heap, 0);

  ret = HeapReAlloc(h_heap, HEAP_ZERO_MEMORY, pmemblk, (SIZE_T)bsize64);

  if (ret)    //不为NULL
  {
    if ((DWORD)ret < 0xC0000000U)
      return ret;
    else
      ExceptionCode = (DWORD)ret;
  }

  return NULL;
}

void * WINAPI HeapMemoryManager::hrecalloc_nom(void * pmemblk, SIZE_T newsize)
{
  LPVOID ret;

  if (!h_heap) return NULL;
  if (pmemblk == NULL || newsize > _HEAP_MAXREQ) return NULL;

  if (newsize == 0)
  {
    HeapFree(h_heap, 0, pmemblk);
    if (HMMCHK_ENABLE_COMPACT_FREE(InitFlag))
      HeapCompact(h_heap, 0);
    return NULL;
  }

  if (HMMCHK_ENABLE_COMPACT_ALLOC(InitFlag))
    HeapCompact(h_heap, 0);

  ret = HeapReAlloc(h_heap, HEAP_ZERO_MEMORY | HEAP_REALLOC_IN_PLACE_ONLY, pmemblk, newsize);

  if (ret)    //不为NULL
  {
    if ((DWORD)ret < 0xC0000000U)
      return ret;
    else
      ExceptionCode = (DWORD)ret;
  }

  return NULL;
}

void * WINAPI HeapMemoryManager::hrecalloc_nom(void * pmemblk, SIZE_T newnum, SIZE_T usize)
{
  UINT64 bsize64;
  LPVOID ret;

  if (!h_heap) return NULL;
  bsize64 = newnum * usize;
  if (pmemblk == NULL || bsize64 > _HEAP_MAXREQ) return NULL;

  if (bsize64 == 0)
  {
    HeapFree(h_heap, 0, pmemblk);
    if (HMMCHK_ENABLE_COMPACT_FREE(InitFlag))
      HeapCompact(h_heap, 0);
    return NULL;
  }

  if (HMMCHK_ENABLE_COMPACT_ALLOC(InitFlag))
    HeapCompact(h_heap, 0);

  ret = HeapReAlloc(h_heap, HEAP_ZERO_MEMORY | HEAP_REALLOC_IN_PLACE_ONLY, pmemblk, (SIZE_T)bsize64);

  if (ret)    //不为NULL
  {
    if ((DWORD)ret < 0xC0000000U)
      return ret;
    else
      ExceptionCode = (DWORD)ret;
  }

  return NULL;
}

DWORD WINAPI HeapMemoryManager::hsize(void * pmemblk)
{
  if (!h_heap) return (DWORD)-1;
  return HeapSize(h_heap, 0, pmemblk);
}

BOOL WINAPI HeapMemoryManager::hvalidate(void * pmemblk)
{
  if (!h_heap) return FALSE;
  return HeapValidate(h_heap, 0, pmemblk);
}

BOOL WINAPI HeapMemoryManager::hfree(void * pmemblk)
{
  BOOL ret;

  if (!h_heap) return FALSE;
  if (pmemblk == NULL)  return FALSE;
  ret = HeapFree(h_heap, 0, pmemblk);
  if (HMMCHK_ENABLE_COMPACT_FREE(InitFlag)) HeapCompact(h_heap, 0);
  return ret;
}

DWORD WINAPI HeapMemoryManager::hgeterrorcode()
{
  return ExceptionCode;
}

SIZE_T WINAPI HeapMemoryManager::hcompact()
{
  if (!h_heap) return 0;
  return HeapCompact(h_heap, 0);
}

BOOL WINAPI HeapMemoryManager::hreset(DWORD dwFlag, SIZE_T InitSize)
{
  DWORD hcFlag = 0;

  if (h_heap) HeapDestroy(h_heap);
  if (dwFlag & HMMF_ENABLE_EXECUTE) hcFlag |= HEAP_CREATE_ENABLE_EXECUTE;
  if (dwFlag & HMMF_ENABLE_EXCEPTIONS) hcFlag |= HEAP_GENERATE_EXCEPTIONS;

  h_heap = HeapCreate(hcFlag, InitSize, 0);
  InitFlag = 0;
  b_lfh = FALSE;
  ExceptionCode = 0;

  if (!h_heap) return FALSE;
  InitFlag = dwFlag;

  if (dwFlag & HMMF_ENABLE_LFH)
  {
    ULONG hInfo = HEAP_LFH;
    b_lfh = HeapSetInformation(h_heap, HeapCompatibilityInformation, &hInfo, sizeof(ULONG));
  }

  return TRUE;
}

BOOL WINAPI HeapMemoryManager::hdestroy()
{
  if (!h_heap) return FALSE;

  BOOL ret;
  ret = HeapDestroy(h_heap);
  h_heap = NULL;
  b_lfh = FALSE;
  InitFlag = 0;
  return ret;
}

BOOL WINAPI HeapMemoryManager::IsLowFragmentationHeap()
{
  if (!h_heap) return FALSE;
  return b_lfh;
}