#pragma once

#ifndef _SR_INC_COMMON_H_
#define _SR_INC_COMMON_H_           100

#define ONLY_ASM                    __declspec(naked)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

#ifndef ISOLATION_AWARE_ENABLED
#define ISOLATION_AWARE_ENABLED     1
#endif

#ifndef _INC_SDKDDKVER
#include <SDKDDKVer.h>
#endif

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT                _WIN32_WINNT_WIN7

#ifndef __AFXWIN_H__
#include <afxwin.h>       // MFC 核心组件和标准组件
#endif

#ifndef __AFXEXT_H__
#include <afxext.h>       // MFC 扩展
#endif

#ifndef __AFXCVIEW_H__
#include <afxcview.h> 
#endif

#ifndef __AFXDISP_H__
#include <afxdisp.h>      // MFC 自动化类
#endif

#ifndef __AFXDTCTL_H__
#include <afxdtctl.h>     // MFC 对 Internet Explorer 4 公共控件的支持
#endif

#if !defined(_AFX_NO_AFXCMN_SUPPORT) && !defined(__AFXCMN_H__)
#include <afxcmn.h>       // MFC 对 Windows 公共控件的支持
#endif // _AFX_NO_AFXCMN_SUPPORT && __AFXCMN_H__

#ifndef _INC_WINDOWS
#include <Windows.h>
#endif

#ifndef _INC_COMMCTRL
#include <commctrl.h>
#endif

#if !defined( _OLEAUTO_H_ )
#include <OleAuto.h>
#endif

#ifndef _INC_TCHAR
#include <tchar.h>
#endif

#ifndef _WIN32_WINNT_WIN8
#define _WIN32_WINNT_WIN8                       0x0602
#endif

#ifndef SEC_IMAGE_NO_EXECUTE
#define SEC_IMAGE_NO_EXECUTE                    0x11000000
#endif

#endif //_SR_INC_COMMON_H_