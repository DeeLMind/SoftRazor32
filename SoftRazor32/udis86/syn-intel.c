/* udis86 - udis86/syn-intel.c
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
#include "types.h"
#include "extern.h"
#include "decode.h"
#include "itab.h"
#include "syn.h"
#include "udint.h"

/* -----------------------------------------------------------------------------
 * opr_cast() - 打印操作数类型.
 * -----------------------------------------------------------------------------
 */
static void 
opr_cast(struct ud *u, struct ud_operand* op,wchar_t *wtmp,size_t wcount)
{
  if (u->br_far) {
    wprintfcat(wtmp,wcount,L"far ");
  }
  switch(op->size) {
  case  8: wprintfcat(wtmp,wcount,L"byte ptr "); break;
  case 16: wprintfcat(wtmp,wcount,L"word ptr "); break;
  case 32: wprintfcat(wtmp,wcount,L"dword ptr "); break;
  case 64: wprintfcat(wtmp,wcount,L"qword ptr "); break;
  case 80: wprintfcat(wtmp,wcount,L"tword ptr "); break;
  default: break;
  }
}

/* -----------------------------------------------------------------------------
 * gen_operand() - 为每个操作数生成汇编输出.
 * -----------------------------------------------------------------------------
 */
static void gen_operand(struct ud *u,struct ud_operand *op,int syn_cast,wchar_t *wtmp,size_t wcount)
{

  switch(op->type)
  {
  case UD_OP_REG:
    wprintfcat(wtmp,wcount,L"%s",ud_reg_tab[op->base - UD_R_AL]);
    ud_casmprint(u,UDCA_GPRs,3,wtmp);
    break;

  case UD_OP_MEM:
    if (syn_cast)
      opr_cast(u, op,wtmp,wcount); //操作数类型 dword ptr

    if (u->pfx_seg)
      wprintfcat(wtmp,wcount,L"%s:", ud_reg_tab[u->pfx_seg - UD_R_AL]);  //段寄存器

    wprintfcat(wtmp,wcount,L"[");

    if (op->base)
      wprintfcat(wtmp,wcount,L"%s", ud_reg_tab[op->base - UD_R_AL]);

    if (op->index)
    {
      wprintfcat(wtmp,wcount,L"%s%s", op->base != UD_NONE? L"+" : L"",ud_reg_tab[op->index - UD_R_AL]);
      if (op->scale)
        wprintfcat(wtmp,wcount,L"*%d", op->scale);
    }
    if (op->offset != 0)
    {
      ud_syn_print_mem_disp(u, op, (op->base != UD_NONE || op->index != UD_NONE) ? 1 : 0,wtmp,wcount);
    }
    wprintfcat(wtmp,wcount,L"]");
    ud_casmprint(u,UDCA_MOther,3,wtmp);
    break;
      
  case UD_OP_IMM:
    ud_syn_print_imm(u, op,wtmp,wcount);
    ud_casmprint(u,UDCA_IMM,3,wtmp);
    break;

  case UD_OP_JIMM:
    ud_syn_print_addr(u, ud_syn_rel_target(u, op),wtmp,wcount);
    ud_casmprint(u,UDCA_IMMAddr,3,wtmp);
    break;

  case UD_OP_PTR:
    switch (op->size)
    {
      case 32:
        wprintfcat(wtmp,wcount,L"word ptr 0x%X:0x%X", op->lval.ptr.seg,op->lval.ptr.off & 0xFFFF);
        break;
      case 48:
        wprintfcat(wtmp,wcount,L"dword ptr 0x%X:0x%X", op->lval.ptr.seg,op->lval.ptr.off);
        break;
    }
    ud_casmprint(u,UDCA_IMM,3,wtmp);
    break;

  case UD_OP_CONST:
    if (syn_cast)
      opr_cast(u, op,wtmp,wcount);
    wprintfcat(wtmp,wcount,L"%d", op->lval.udword);
    ud_casmprint(u,UDCA_IMM,3,wtmp);
    break;

  default:
    return;
  }
}

/* =============================================================================
 * 翻译为Intel语法
 * =============================================================================
 */
extern void
ud_translate_intel(struct ud *u)
{
  uint16_t st;
  wchar_t stmp[128] = {0};

  /* 检查如果 P_OSO 前缀被使用 */
  if (!P_OSO(u->itab_entry->prefix) && u->pfx_opr) {
    switch (u->dis_mode) {
    case 16: wprintfcat(stmp,128,L"o32 "); break;
    case 32:
    case 64: wprintfcat(stmp,128,L"o16 "); break;
    }
  }

  /* 检查如果 P_ASO 前缀被使用 */
  if (!P_ASO(u->itab_entry->prefix) && u->pfx_adr) {
    switch (u->dis_mode) {
    case 16: wprintfcat(stmp,128,L"a32 "); break;
    case 32: wprintfcat(stmp,128,L"a16 "); break;
    case 64: wprintfcat(stmp,128,L"a32 "); break;
    }
  }

  /* 检查段寄存器 */
  if (u->pfx_seg && u->operand[0].type != UD_OP_MEM && u->operand[1].type != UD_OP_MEM)
    wprintfcat(stmp,128,L"%s ", ud_reg_tab[u->pfx_seg - UD_R_AL]);

  /* 检查 lock 前缀 */
  if (u->pfx_lock)
    wprintfcat(stmp,128,L"lock ");

  if (u->pfx_rep)
    wprintfcat(stmp,128,L"rep ");
  else if (u->pfx_repe)
    wprintfcat(stmp,128,L"repe ");
  else if (u->pfx_repne)
    wprintfcat(stmp,128,L"repne ");

  /* 打印指令助记符 */
  ud_casmprint(u,UDCA_Prefix,3,stmp);

  switch (u->mnemonic)
  {
  case UD_Ipush:                      //栈操作指令
  case UD_Ipop:
    st = UDCA_StackOp;
    break;
  case UD_Ijmp:                       //跳转
    st = UDCA_Jump;
    break;
  case UD_Icall:                      //调用
    st = UDCA_Call;
    break;
  case UD_Iretn:                      //返回
  case UD_Iretf:
    st = UDCA_Ret;
    break;
  case UD_Iint:
  case UD_Iint1:
  case UD_Iint3:
    st = UDCA_Int;
    break;
  default:
    if (u->mnemonic >= UD_Ijo && u->mnemonic <= UD_Ijrcxz)  //条件跳转
      st = UDCA_CJump;
    else if ( (u->mnemonic >= UD_Iemms && u->mnemonic <= UD_Ifyl2xp1) ||
              (u->mnemonic >= UD_Ipacksswb && u->mnemonic <= UD_Ipextrw) ||
              (u->mnemonic == UD_Ipor) ||
              (u->mnemonic >= UD_Ipsadbw && u->mnemonic <= UD_Ipunpckldq) ||
              (u->mnemonic == UD_Ipxor) )         //FPU MMX SSE 寄存器相关指令
      st = UDCA_FMSOp;
    else                                          //普通指令
      st = UDCA_Inst;
  }
  wprintfcat(stmp,128,L"%s", ud_lookup_mnemonic(u->mnemonic));
  ud_casmprint(u,st,3,stmp);

  if (u->operand[0].type != UD_NONE) {
    int cast = 0;
    wprintfcat(stmp,128,L" ");
    ud_casmprint(u,UDCA_Inst,3,stmp);
    if (u->operand[0].type == UD_OP_MEM)
    {
      if (u->operand[1].type == UD_OP_IMM   ||
          u->operand[1].type == UD_OP_CONST ||
          u->operand[1].type == UD_NONE     ||
          (u->operand[0].size != u->operand[1].size && 
           u->operand[1].type != UD_OP_REG))
      {
          cast = 1;
      } else if (u->operand[1].type == UD_OP_REG &&
                 u->operand[1].base == UD_R_CL)
      {
          switch (u->mnemonic)
          {
          case UD_Ircl:
          case UD_Irol:
          case UD_Iror:
          case UD_Ircr:
          case UD_Ishl:
          case UD_Ishr:
          case UD_Isar:
              cast = 1;
              break;
          default: break;
          }
      }
    }
    //默认生成操作数
    gen_operand(u, &u->operand[0], cast,stmp,128);
  }

  if (u->operand[1].type != UD_NONE)
  {
    int cast = 0;
    wprintfcat(stmp,128,L",");
    ud_casmprint(u,UDCA_Inst,3,stmp);
    if (u->operand[1].type == UD_OP_MEM &&
        u->operand[0].size != u->operand[1].size && !ud_opr_is_sreg(&u->operand[0])) {
      cast = 1;
    }
    gen_operand(u, &u->operand[1], cast,stmp,128);
  }

  if (u->operand[2].type != UD_NONE)
  {
    wprintfcat(stmp,128,L",");
    ud_casmprint(u,UDCA_Inst,3,stmp);
    gen_operand(u, &u->operand[2], 0,stmp,128);
  }
  ud_casmprint(u,UDCA_Inst,3,stmp);
}

/*
vim: set ts=2 sw=2 expandtab
*/
