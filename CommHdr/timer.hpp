#pragma once

#ifndef _WINDOWS_
#include <Windows.h>
#endif

#ifndef _INC_MALLOC
#include <malloc.h>
#endif

#ifndef __cplusplus
#define inline    __inline
#endif

//创建计时器
inline HANDLE timer_Create()
{
  return calloc(4, sizeof(LARGE_INTEGER));
}

//记录起始时间戳
inline INT_PTR timer_Begin(HANDLE hTimer)
{
  if (hTimer == NULL)
    return 0;

  QueryPerformanceCounter((PLARGE_INTEGER)hTimer);
  return (INT_PTR)(((PLARGE_INTEGER)hTimer)->QuadPart);
}

//记录时间
inline INT_PTR timer_Record(HANDLE hTimer, UINT idx /* 1 ~ 3 */)
{
  LARGE_INTEGER tCurrent;

  QueryPerformanceCounter(&tCurrent);
  if (idx < 1 || idx > 3) idx = 1;
  ((PLARGE_INTEGER)hTimer)[idx] = tCurrent;
  return (INT_PTR)tCurrent.QuadPart;
}

//求记录的绝对差距(s),返回32位无符号
inline UINT32 timer_RecordDiff_s32(HANDLE hTimer, UINT idx1, UINT idx2)
{
  LARGE_INTEGER tFrequency, tAbsDiff;

  if (hTimer == NULL) return 0;
  if (idx1 > 3) idx1 = 0;
  if (idx2 > 3) idx2 = 1;
  if (idx1 == idx2) return 0;

  QueryPerformanceFrequency(&tFrequency);
  tAbsDiff.QuadPart = llabs(((PLARGE_INTEGER)hTimer)[idx2].QuadPart - ((PLARGE_INTEGER)hTimer)[idx1].QuadPart);
  return (UINT32)(tAbsDiff.QuadPart / tFrequency.QuadPart);
}

//求记录的绝对差距(s),返回64位无符号
inline UINT64 timer_RecordDiff_s64(HANDLE hTimer, UINT idx1, UINT idx2)
{
  LARGE_INTEGER tFrequency, tAbsDiff;

  if (hTimer == NULL) return 0;
  if (idx1 > 3) idx1 = 0;
  if (idx2 > 3) idx2 = 1;
  if (idx1 == idx2) return 0;

  QueryPerformanceFrequency(&tFrequency);
  tAbsDiff.QuadPart = llabs(((PLARGE_INTEGER)hTimer)[idx2].QuadPart - ((PLARGE_INTEGER)hTimer)[idx1].QuadPart);
  return (UINT64)(tAbsDiff.QuadPart / tFrequency.QuadPart);
}

//求记录的绝对差距(ms),返回32位无符号
inline UINT32 timer_RecordDiff_ms32(HANDLE hTimer, UINT idx1, UINT idx2)
{
  LARGE_INTEGER tFrequency;
  long double ldFreq, ldAbsDiff;

  if (hTimer == NULL) return 0;
  if (idx1 > 3) idx1 = 0;
  if (idx2 > 3) idx2 = 1;
  if (idx1 == idx2) return 0;

  QueryPerformanceFrequency(&tFrequency);

  ldFreq = (long double)tFrequency.QuadPart;
  ldFreq /= 1000.0f; //ms级精度
  ldAbsDiff = (long double)llabs(((PLARGE_INTEGER)hTimer)[idx2].QuadPart - ((PLARGE_INTEGER)hTimer)[idx1].QuadPart);

  return (UINT32)(ldAbsDiff / ldFreq);
}

//求记录的绝对差距(ms),返回64位无符号
inline UINT64 timer_RecordDiff_ms64(HANDLE hTimer, UINT idx1, UINT idx2)
{
  LARGE_INTEGER tFrequency;
  long double ldFreq, ldAbsDiff;

  if (hTimer == NULL) return 0;
  if (idx1 > 3) idx1 = 0;
  if (idx2 > 3) idx2 = 1;
  if (idx1 == idx2) return 0;

  QueryPerformanceFrequency(&tFrequency);

  ldFreq = (long double)tFrequency.QuadPart;
  ldFreq /= 1000.0f; //ms级精度
  ldAbsDiff = (long double)llabs(((PLARGE_INTEGER)hTimer)[idx2].QuadPart - ((PLARGE_INTEGER)hTimer)[idx1].QuadPart);

  return (UINT64)(ldAbsDiff / ldFreq);
}

//求记录的绝对差距(us),返回32位无符号
inline UINT32 timer_RecordDiff_us32(HANDLE hTimer, UINT idx1, UINT idx2)
{
  LARGE_INTEGER tFrequency;
  long double ldFreq, ldAbsDiff;

  if (hTimer == NULL) return 0;
  if (idx1 > 3) idx1 = 0;
  if (idx2 > 3) idx2 = 1;
  if (idx1 == idx2) return 0;

  QueryPerformanceFrequency(&tFrequency);

  ldFreq = (long double)tFrequency.QuadPart;
  ldFreq /= 1000000.0f; //us级精度
  ldAbsDiff = (long double)llabs(((PLARGE_INTEGER)hTimer)[idx2].QuadPart - ((PLARGE_INTEGER)hTimer)[idx1].QuadPart);

  return (UINT32)(ldAbsDiff / ldFreq);
}

//求记录的绝对差距(us),返回64位无符号
inline UINT64 timer_RecordDiff_us64(HANDLE hTimer, UINT idx1, UINT idx2)
{
  LARGE_INTEGER tFrequency;
  long double ldFreq, ldAbsDiff;

  if (hTimer == NULL) return 0;
  if (idx1 > 3) idx1 = 0;
  if (idx2 > 3) idx2 = 1;
  if (idx1 == idx2) return 0;

  QueryPerformanceFrequency(&tFrequency);

  ldFreq = (long double)tFrequency.QuadPart;
  ldFreq /= 1000000.0f; //us级精度
  ldAbsDiff = (long double)llabs(((PLARGE_INTEGER)hTimer)[idx2].QuadPart - ((PLARGE_INTEGER)hTimer)[idx1].QuadPart);

  return (UINT64)(ldAbsDiff / ldFreq);
}

//求秒级时差,返回32位无符号
inline UINT32 timer_CurrentDiff_s32(HANDLE hTimer)
{
  LARGE_INTEGER tCurrent, tFrequency;

  QueryPerformanceCounter(&tCurrent);
  QueryPerformanceFrequency(&tFrequency);

  if (hTimer == NULL) return 0;

  tCurrent.QuadPart -= ((PLARGE_INTEGER)hTimer)->QuadPart;
  return (UINT32)(tCurrent.QuadPart / tFrequency.QuadPart);
}

//求秒级时差,返回64位无符号
inline UINT64 timer_CurrentDiff_s64(HANDLE hTimer)
{
  LARGE_INTEGER tCurrent, tFrequency;

  QueryPerformanceCounter(&tCurrent);
  QueryPerformanceFrequency(&tFrequency);

  if (hTimer == NULL) return 0;

  tCurrent.QuadPart -= ((PLARGE_INTEGER)hTimer)->QuadPart;
  return (UINT64)(tCurrent.QuadPart / tFrequency.QuadPart);
}

//求毫秒级时差,返回32位无符号
inline UINT32 timer_CurrentDiff_ms32(HANDLE hTimer)
{
  LARGE_INTEGER tCurrent, tFrequency;
  long double ldFreq, ldDiff;

  QueryPerformanceCounter(&tCurrent);
  QueryPerformanceFrequency(&tFrequency);

  if (hTimer == NULL) return 0;

  ldFreq = (long double)tFrequency.QuadPart;
  ldFreq /= 1000.0f; //ms级精度
  ldDiff = (long double)(tCurrent.QuadPart - ((PLARGE_INTEGER)hTimer)->QuadPart);

  return (UINT32)(ldDiff / ldFreq);
}

//求毫秒级时差,返回64位无符号
inline UINT64 timer_CurrentDiff_ms64(HANDLE hTimer)
{
  LARGE_INTEGER tCurrent, tFrequency;
  long double ldFreq, ldDiff;

  QueryPerformanceCounter(&tCurrent);
  QueryPerformanceFrequency(&tFrequency);

  if (hTimer == NULL) return 0;

  ldFreq = (long double)tFrequency.QuadPart;
  ldFreq /= 1000.0f; //ms级精度
  ldDiff = (long double)(tCurrent.QuadPart - ((PLARGE_INTEGER)hTimer)->QuadPart);

  return (UINT64)(ldDiff / ldFreq);
}


//求微秒级时差,返回32位无符号
inline UINT32 timer_CurrentDiff_us32(HANDLE hTimer)
{
  LARGE_INTEGER tCurrent, tFrequency;
  long double ldFreq, ldDiff;

  QueryPerformanceCounter(&tCurrent);
  QueryPerformanceFrequency(&tFrequency);

  if (hTimer == NULL) return 0;

  ldFreq = (long double)tFrequency.QuadPart;
  ldFreq /= 1000000.0f; //us级精度
  ldDiff = (long double)(tCurrent.QuadPart - ((PLARGE_INTEGER)hTimer)->QuadPart);

  return (UINT32)(ldDiff / ldFreq);
}

//求微秒级时差,返回64位无符号
inline UINT64 timer_CurrentDiff_us64(HANDLE hTimer)
{
  LARGE_INTEGER tCurrent, tFrequency;
  long double ldFreq, ldDiff;

  QueryPerformanceCounter(&tCurrent);
  QueryPerformanceFrequency(&tFrequency);

  if (hTimer == NULL) return 0;

  ldFreq = (long double)tFrequency.QuadPart;
  ldFreq /= 1000000.0f; //us级精度
  ldDiff = (long double)(tCurrent.QuadPart - ((PLARGE_INTEGER)hTimer)->QuadPart);

  return (UINT64)(ldDiff / ldFreq);
}

inline void timer_Destroy(HANDLE hTimer)
{
  if (hTimer != NULL)
    free(hTimer);
}