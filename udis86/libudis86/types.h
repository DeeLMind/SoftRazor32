/* udis86 - libudis86/types.h
 *
 * Copyright (c) 2002-2013 Vivek Thampi
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright notice, 
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, 
 *       this list of conditions and the following disclaimer in the documentation 
 *       and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef UD_TYPES_H
#define UD_TYPES_H

#ifdef __KERNEL__
  /* 
   * -D__KERNEL__ is automatically passed on the command line when
   * building something as part of the Linux kernel. Assume standalone
   * mode.
   */
# include <linux/kernel.h>
# include <linux/string.h>
# ifndef __UD_STANDALONE__
#  define __UD_STANDALONE__ 1
# endif
#endif /* __KERNEL__ */

#if !defined(__UD_STANDALONE__)
# include <stdint.h>
# include <stdio.h>
#endif

/* gcc specific extensions */
#ifdef __GNUC__
# define UD_ATTR_PACKED __attribute__((packed))
#else
# define UD_ATTR_PACKED
#endif /* UD_ATTR_PACKED */

/* define macro */
#define UD_AMODE_PRINT_FAR          0x00000001
#define UD_AMODE_PRINT_NEAR         0x00000002
#define UD_AMODE_NOT_PRINT_ADDR     0x00000004 

#define UD_HEX                      "X"
#define UD_PTR_BYTE                 "byte ptr"
#define UD_PTR_WORD                 "word ptr"
#define UD_PTR_DWORD                "dword ptr"
#define UD_PTR_QWORD                "qword ptr"
#define UD_PTR_TWORD                "tword ptr"
#define UD_PTR_OWORD                "oword ptr"
#define UD_PTR_YWORD                "yword ptr"
#define UD_CHK_FLAG(val, flg)       (((val) & (flg)) == (flg))
#define UD_NOT_FLAG(val, flg)       (((val) & (flg)) != (flg))
#define UD_NOT_EMPTY(val, flg)      ((val) & (flg))

/* -----------------------------------------------------------------------------
 * All possible "types" of objects in udis86. Order is Important!
 * -----------------------------------------------------------------------------
 */
enum ud_type
{
  UD_NONE,

  /* 8 bit GPRs */
  UD_R_AL,  UD_R_CL,  UD_R_DL,  UD_R_BL,
  UD_R_AH,  UD_R_CH,  UD_R_DH,  UD_R_BH,
  UD_R_SPL, UD_R_BPL, UD_R_SIL, UD_R_DIL,
  UD_R_R8B, UD_R_R9B, UD_R_R10B,  UD_R_R11B,
  UD_R_R12B,  UD_R_R13B,  UD_R_R14B,  UD_R_R15B,

  /* 16 bit GPRs */
  UD_R_AX,  UD_R_CX,  UD_R_DX,  UD_R_BX,
  UD_R_SP,  UD_R_BP,  UD_R_SI,  UD_R_DI,
  UD_R_R8W, UD_R_R9W, UD_R_R10W,  UD_R_R11W,
  UD_R_R12W,  UD_R_R13W,  UD_R_R14W,  UD_R_R15W,
  
  /* 32 bit GPRs */
  UD_R_EAX, UD_R_ECX, UD_R_EDX, UD_R_EBX,
  UD_R_ESP, UD_R_EBP, UD_R_ESI, UD_R_EDI,
  UD_R_R8D, UD_R_R9D, UD_R_R10D,  UD_R_R11D,
  UD_R_R12D,  UD_R_R13D,  UD_R_R14D,  UD_R_R15D,
  
  /* 64 bit GPRs */
  UD_R_RAX, UD_R_RCX, UD_R_RDX, UD_R_RBX,
  UD_R_RSP, UD_R_RBP, UD_R_RSI, UD_R_RDI,
  UD_R_R8,  UD_R_R9,  UD_R_R10, UD_R_R11,
  UD_R_R12, UD_R_R13, UD_R_R14, UD_R_R15,

  /* segment registers */
  UD_R_ES,  UD_R_CS,  UD_R_SS,  UD_R_DS,
  UD_R_FS,  UD_R_GS,  

  /* control registers*/
  UD_R_CR0, UD_R_CR1, UD_R_CR2, UD_R_CR3,
  UD_R_CR4, UD_R_CR5, UD_R_CR6, UD_R_CR7,
  UD_R_CR8, UD_R_CR9, UD_R_CR10,  UD_R_CR11,
  UD_R_CR12,  UD_R_CR13,  UD_R_CR14,  UD_R_CR15,
  
  /* debug registers */
  UD_R_DR0, UD_R_DR1, UD_R_DR2, UD_R_DR3,
  UD_R_DR4, UD_R_DR5, UD_R_DR6, UD_R_DR7,
  UD_R_DR8, UD_R_DR9, UD_R_DR10,  UD_R_DR11,
  UD_R_DR12,  UD_R_DR13,  UD_R_DR14,  UD_R_DR15,

  /* mmx registers */
  UD_R_MM0, UD_R_MM1, UD_R_MM2, UD_R_MM3,
  UD_R_MM4, UD_R_MM5, UD_R_MM6, UD_R_MM7,

  /* x87 registers */
  UD_R_ST0, UD_R_ST1, UD_R_ST2, UD_R_ST3,
  UD_R_ST4, UD_R_ST5, UD_R_ST6, UD_R_ST7, 

  /* extended multimedia registers */
  UD_R_XMM0,  UD_R_XMM1,  UD_R_XMM2,  UD_R_XMM3,
  UD_R_XMM4,  UD_R_XMM5,  UD_R_XMM6,  UD_R_XMM7,
  UD_R_XMM8,  UD_R_XMM9,  UD_R_XMM10, UD_R_XMM11,
  UD_R_XMM12, UD_R_XMM13, UD_R_XMM14, UD_R_XMM15,

  /* 256B multimedia registers */
  UD_R_YMM0,  UD_R_YMM1,  UD_R_YMM2,  UD_R_YMM3,
  UD_R_YMM4,  UD_R_YMM5,  UD_R_YMM6,  UD_R_YMM7,
  UD_R_YMM8,  UD_R_YMM9,  UD_R_YMM10, UD_R_YMM11,
  UD_R_YMM12, UD_R_YMM13, UD_R_YMM14, UD_R_YMM15,

  UD_R_RIP,

  /* Operand Types */
  UD_OP_REG,  UD_OP_MEM,  UD_OP_PTR,  UD_OP_IMM,  
  UD_OP_JIMM, UD_OP_CONST
};

#include "itab.h"

union ud_lval {
  int8_t     sbyte;
  uint8_t    ubyte;
  int16_t    sword;
  uint16_t   uword;
  int32_t    sdword;
  uint32_t   udword;
  int64_t    sqword;
  uint64_t   uqword;
  struct {
    uint16_t seg;
    uint32_t off;
  } ptr;
};

/* -----------------------------------------------------------------------------
 * struct ud_operand - Disassembled instruction Operand.
 * -----------------------------------------------------------------------------
 */
struct ud_operand {
  enum ud_type    type;
  uint16_t        size;
  enum ud_type    base;
  enum ud_type    index;
  uint8_t         scale;  
  uint8_t         offset;
  union ud_lval   lval;
  /*
   * internal use only
   */
  uint64_t        _legacy; /* this will be removed in 1.8 */
  uint8_t         _oprcode;
};

/* -----------------------------------------------------------------------------
 * struct ud - The udis86 object.
 * -----------------------------------------------------------------------------
 */
struct ud
{
  /*
   * input buffering
   */
  int       (*inp_hook) (struct ud*);
#ifndef __UD_STANDALONE__
  FILE*     inp_file;
#endif
  const uint8_t* inp_buf;
  size_t    inp_buf_size;
  size_t    inp_buf_index;
  uint8_t   inp_curr;
  size_t    inp_ctr;
  uint8_t   inp_sess[64];
  int       inp_end;
  int       inp_peek;

  void      (*translator)(struct ud*);
  uint64_t  insn_offset;
  char      insn_hexcode[64];

  /*
   * Assembly output buffer
   */
  int       asm_mode;
  char     *asm_buf;
  size_t    asm_buf_size;
  size_t    asm_index;
  char      asm_buf_int[128];

  char     *casm_buf;             //指向颜色反汇编buffer
  size_t    casm_buf_size;        //颜色反汇编buffer大小
  size_t    casm_index;
  int       casm_error;           //colorasm错误信息,0即没有错误
  char      casm_buf_int[256];    //默认内部buffer

  /*
   * Symbol resolver for use in the translation phase.
   */
  const char* (*sym_resolver)(struct ud*, uint64_t addr, int64_t *offset);

  uint8_t   dis_mode;
  uint64_t  pc;
  uint8_t   vendor;
  enum ud_mnemonic_code mnemonic;
  struct ud_operand operand[4];
  uint8_t   error;
  uint8_t   _rex;
  uint8_t   pfx_rex;
  uint8_t   pfx_seg;
  uint8_t   pfx_opr;
  uint8_t   pfx_adr;
  uint8_t   pfx_lock;
  uint8_t   pfx_str;
  uint8_t   pfx_rep;
  uint8_t   pfx_repe;
  uint8_t   pfx_repne;
  uint8_t   opr_mode;
  uint8_t   adr_mode;
  uint8_t   br_far;
  uint8_t   br_near;
  uint8_t   have_modrm;
  uint8_t   modrm;
  uint8_t   modrm_offset;
  uint8_t   vex_op;
  uint8_t   vex_b1;
  uint8_t   vex_b2;
  uint8_t   primary_opcode;
  void *    user_opaque_data;
  struct ud_itab_entry * itab_entry;
  struct ud_lookup_table_list_entry *le;
};

/* -----------------------------------------------------------------------------
 * Type-definitions
 * -----------------------------------------------------------------------------
 */
typedef enum ud_type          ud_type_t;
typedef enum ud_mnemonic_code ud_mnemonic_code_t;

typedef struct ud             ud_t;
typedef struct ud_operand     ud_operand_t;

#define UD_SYN_INTEL          ud_translate_intel
#define UD_SYN_ATT            ud_translate_att
#define UD_EOI                (-1)
#define UD_INP_CACHE_SZ       32
#define UD_VENDOR_AMD         0
#define UD_VENDOR_INTEL       1
#define UD_VENDOR_ANY         2

/*定义指令类型*/
#define CD_NULL               0x00    //无
#define CD_IGNORE             0x01    //被忽略的
#define CD_INST               0x02    //一般指令 (mov add xor)
#define CD_STACK              0x03    //栈操作指令 (push pop)
#define CD_FMS                0x04    //FPU MMX SSE 寄存器相关指令
#define CD_CJMP               0x05    //条件跳转
#define CD_JMP                0x06    //跳转
#define CD_CALL               0x07    //调用
#define CD_RET                0x08    //返回
#define CD_PRIVINST           0x09    //特权指令
#define CD_PF                 0x0A    //前缀
#define CD_PFLOCK             0x0B    //lock前缀
#define CD_PFREP              0x0C    //rep前缀
#define CD_DEFCLR             0x0D    //默认颜色
#define CD_INT                0x0E    //INT中断
#define CD_SYS                0x0F    //sys* (sysenter,syscall)
#define CD_DATA               0x10    //数据
#define CD_TEXT               0x11    //文本
#define CD_SYMBOL             0x12    //符号名称
#define CD_ERROR              0x13    //非法命令、错误指令
#define CD_GPRS               0x14    //通用寄存器
#define CD_FSRS               0x15    //FPU SSE 寄存器
#define CD_SEG                0x16    //段寄存器
#define CD_SSRS               0x17    //系统寄存器
#define CD_MEMSTK             0x18    //栈内存地址
#define CD_MEMOTH             0x19    //其它内存地址
#define CD_IMM                0x1A    //立即数
#define CD_IMMADDR            0x1B    //地址立即数
#define CD_DECIMAL            0x1C    //10进制
#define CD_DISTANCE           0x1D    //距离修饰符 (near far)
#define CD_SIZE               0x1E    //大小修饰符 (byte word dword)
#define CD_CHANGED            0x80    //被修改的

#endif

/*
vim: set ts=2 sw=2 expandtab
*/
