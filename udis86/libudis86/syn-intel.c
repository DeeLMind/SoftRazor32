/* udis86 - libudis86/syn-intel.c
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
 * opr_cast() - Prints an operand cast.
 * -----------------------------------------------------------------------------
 */
static void
opr_cast(struct ud* u, struct ud_operand* op)
{
  if ((u->br_far) && UD_NOT_EMPTY(u->asm_mode, UD_AMODE_PRINT_FAR))
  {
    ud_casmprintf(u, 0x01, CD_DISTANCE, "far");
    ud_casmprintf(u, 0x01, CD_DEFCLR, " ");
  }

  if ((u->br_near) && UD_NOT_EMPTY(u->asm_mode, UD_AMODE_PRINT_NEAR))
  {
    ud_casmprintf(u, 0x01, CD_DISTANCE, "near");
    ud_casmprintf(u, 0x01, CD_DEFCLR, " ");
  }

  switch (op->size)
  {
  case  8:
    ud_casmprintf(u, 0x01, CD_SIZE, UD_PTR_BYTE);
    ud_casmprintf(u, 0x01, CD_DEFCLR, " ");
    break;
  case 16:
    ud_casmprintf(u, 0x01, CD_SIZE, UD_PTR_WORD);
    ud_casmprintf(u, 0x01, CD_DEFCLR, " ");
    break;
  case 32:
    ud_casmprintf(u, 0x01, CD_SIZE, UD_PTR_DWORD);
    ud_casmprintf(u, 0x01, CD_DEFCLR, " ");
    break;
  case 64:
    ud_casmprintf(u, 0x01, CD_SIZE, UD_PTR_QWORD);
    ud_casmprintf(u, 0x01, CD_DEFCLR, " ");
    break;
  case 80:
    ud_casmprintf(u, 0x01, CD_SIZE, UD_PTR_TWORD);
    ud_casmprintf(u, 0x01, CD_DEFCLR, " ");
    break;
  case 128:
    ud_casmprintf(u, 0x01, CD_SIZE, UD_PTR_OWORD);
    ud_casmprintf(u, 0x01, CD_DEFCLR, " ");
    break;
  case 256:
    ud_casmprintf(u, 0x01, CD_SIZE, UD_PTR_YWORD);
    ud_casmprintf(u, 0x01, CD_DEFCLR, " ");
    break;
  default: break;
  }
}

/* -----------------------------------------------------------------------------
 * gen_operand() - Generates assembly output for each operand.
 * -----------------------------------------------------------------------------
 */
static void gen_operand(struct ud* u, struct ud_operand* op, int syn_cast)
{
  switch (op->type)
  {
  case UD_OP_REG:
    ud_casmprintf(u, 0x01, CD_GPRS, "%s", ud_reg_tab[op->base - UD_R_AL]);
    break;

  case UD_OP_MEM:
    if (syn_cast)
    {
      opr_cast(u, op);
    }

    ud_casmprintf(u, 0x01, CD_DEFCLR, "[");

    //段寄存器
    if (u->pfx_seg)
    {
      ud_casmprintf(u, 0x01, CD_SEG, "%s", ud_reg_tab[u->pfx_seg - UD_R_AL]);
      ud_casmprintf(u, 0x01, CD_DEFCLR, ":");
    }

    //基寄存器
    if (op->base)
    {
      ud_casmprintf(u, 0x01, CD_GPRS, "%s", ud_reg_tab[op->base - UD_R_AL]);
    }

    //index
    if (op->index)
    {
      ud_casmprintf(u, 0x01, CD_DEFCLR, "%s", op->base != UD_NONE ? "+" : "");
      ud_casmprintf(u, 0x01, CD_GPRS, "%s", ud_reg_tab[op->index - UD_R_AL]);

      //倍数
      if (op->scale)
      {
        ud_casmprintf(u, 0x01, CD_DEFCLR, "*");
        ud_casmprintf(u, 0x01, CD_DECIMAL, "%u", op->scale);
      }
    }

    //偏移
    if (op->offset != 0)
    {
      ud_syn_print_mem_disp(u, op, 
        (op->base != UD_NONE || op->index != UD_NONE) ? 1 : 0);
    }
    ud_casmprintf(u, 0x01, CD_DEFCLR, "]");
    break;

  case UD_OP_IMM:
    ud_syn_print_imm(u, op);
    break;


  case UD_OP_JIMM:
    ud_syn_print_addr(u, ud_syn_rel_target(u, op));
    break;

  case UD_OP_PTR:
    switch (op->size)
    {
    case 32:
      ud_casmprintf(u, 0x01, CD_SIZE, UD_PTR_WORD);
      ud_casmprintf(u, 0x01, CD_DEFCLR, " ");
      ud_casmprintf(u, 0x01, CD_SEG, "0x" UD_HEX, op->lval.ptr.seg);
      ud_casmprintf(u, 0x01, CD_DEFCLR, ":");
      ud_casmprintf(u, 0x01, CD_IMM, "0x" UD_HEX, op->lval.ptr.off & 0xFFFF);
      break;
    case 48:
      ud_casmprintf(u, 0x01, CD_SIZE, UD_PTR_DWORD);
      ud_casmprintf(u, 0x01, CD_DEFCLR, " ");
      ud_casmprintf(u, 0x01, CD_SEG, "0x" UD_HEX, op->lval.ptr.seg);
      ud_casmprintf(u, 0x01, CD_DEFCLR, ":");
      ud_casmprintf(u, 0x01, CD_IMM, "0x" UD_HEX, op->lval.ptr.off);
      break;
    }
    break;

  case UD_OP_CONST:
    if (syn_cast) opr_cast(u, op);
    ud_casmprintf(u, 0x01, CD_DECIMAL, "%d", op->lval.udword);
    break;

  default: return;
  }
}

/* =============================================================================
 * translates to intel syntax
 * =============================================================================
 */
extern void
ud_translate_intel(struct ud* u)
{
  /* check if P_OSO prefix is used */
  if (!P_OSO(u->itab_entry->prefix) && u->pfx_opr)
  {
    switch (u->dis_mode)
    {
    case 16:
      ud_casmprintf(u, 0x01, CD_INST, "o32");
      ud_casmprintf(u, 0x01, CD_DEFCLR, " ");
      break;
    case 32:
    case 64:
      ud_casmprintf(u, 0x01, CD_INST, "o16");
      ud_casmprintf(u, 0x01, CD_DEFCLR, " ");
      break;
    }
  }

  /* check if P_ASO prefix was used */
  if (!P_ASO(u->itab_entry->prefix) && u->pfx_adr)
  {
    switch (u->dis_mode)
    {
    case 16:
      ud_casmprintf(u, 0x01, CD_INST, "a32");
      ud_casmprintf(u, 0x01, CD_DEFCLR, " ");
      break;
    case 32:
      ud_casmprintf(u, 0x01, CD_INST, "a16");
      ud_casmprintf(u, 0x01, CD_DEFCLR, " ");
      break;
    case 64:
      ud_casmprintf(u, 0x01, CD_INST, "a32");
      ud_casmprintf(u, 0x01, CD_DEFCLR, " ");
      break;
    }
  }

  if (u->pfx_seg &&
      u->operand[0].type != UD_OP_MEM &&
      u->operand[1].type != UD_OP_MEM)
  {
    ud_casmprintf(u, 0x01, CD_SEG, "%s", ud_reg_tab[u->pfx_seg - UD_R_AL]);
    ud_casmprintf(u, 0x01, CD_DEFCLR, " ");
  }

  if (u->pfx_lock)
  {
    ud_casmprintf(u, 0x01, CD_PFLOCK, "lock");
    ud_casmprintf(u, 0x01, CD_DEFCLR, " ");
  }

  if (u->pfx_rep)
  {
    ud_casmprintf(u, 0x01, CD_PFREP, "rep");
    ud_casmprintf(u, 0x01, CD_DEFCLR, " ");
  }
  else if (u->pfx_repe)
  {
    ud_casmprintf(u, 0x01, CD_PFREP, "repe");
    ud_casmprintf(u, 0x01, CD_DEFCLR, " ");
  }
  else if (u->pfx_repne)
  {
    ud_casmprintf(u, 0x01, CD_PFREP, "repne");
    ud_casmprintf(u, 0x01, CD_DEFCLR, " ");
  }

  /* print the instruction mnemonic 
     打印指令助记符 */

  // ud_asmprintf(u, "%s", ud_lookup_mnemonic(u->mnemonic));

  char chIdx = CD_INST;

  switch (u->mnemonic)
  {
    //栈操作指令
  case UD_Ipush:
  case UD_Ipusha:
  case UD_Ipushad:
  case UD_Ipushfd:
  case UD_Ipushfq:
  case UD_Ipushfw:
  case UD_Ipop:
  case UD_Ipopa:
  case UD_Ipopad:
  case UD_Ipopcnt:
  case UD_Ipopfd:
  case UD_Ipopfq:
  case UD_Ipopfw:
    chIdx = CD_STACK;
    break;
    //跳转
  case UD_Ijmp:
    chIdx = CD_JMP;
    break;
    //调用
  case UD_Icall:
    chIdx = CD_CALL;
    break;
    //返回
  case UD_Iretn:
  case UD_Iretf:
    chIdx = CD_RET;
    break;
    //中断
  case UD_Iint:
  case UD_Iint1:
  case UD_Iint3:
    chIdx = CD_INT;
    break;
    //系统调用
  case UD_Isyscall:
  case UD_Isysenter:
  case UD_Isysexit:
  case UD_Isysret:
    chIdx = CD_SYS;
    break;
    //条件跳转
  case UD_Ija:
  case UD_Ijae:
  case UD_Ijb:
  case UD_Ijbe:
  case UD_Ijcxz:
  case UD_Ijecxz:
  case UD_Ijg:
  case UD_Ijge:
  case UD_Ijl:
  case UD_Ijle:
  case UD_Ijno:
  case UD_Ijnp:
  case UD_Ijns:
  case UD_Ijnz:
  case UD_Ijo:
  case UD_Ijp:
  case UD_Ijrcxz:
  case UD_Ijs:
  case UD_Ijz:
    chIdx = CD_CJMP;
    break;
    //浮点相关 ??
  case UD_Iemms:
  case UD_Iextractps:
  case UD_If2xm1:
  case UD_Ifabs:
  case UD_Ifadd:
  case UD_Ifaddp:
  case UD_Ifbld:
  case UD_Ifbstp:
  case UD_Ifchs:
  case UD_Ifclex:
  case UD_Ifcmovb:
  case UD_Ifcmovbe:
  case UD_Ifcmove:
  case UD_Ifcmovnb:
  case UD_Ifcmovnbe:
  case UD_Ifcmovne:
  case UD_Ifcmovnu:
  case UD_Ifcmovu:
  case UD_Ifcom:
  case UD_Ifcom2:
  case UD_Ifcomi:
  case UD_Ifcomip:
  case UD_Ifcomp:
  case UD_Ifcomp3:
  case UD_Ifcomp5:
  case UD_Ifcompp:
  case UD_Ifcos:
  case UD_Ifdecstp:
  case UD_Ifdiv:
  case UD_Ifdivp:
  case UD_Ifdivr:
  case UD_Ifdivrp:
  case UD_Ifemms:
  case UD_Iffree:
  case UD_Iffreep:
  case UD_Ifiadd:
  case UD_Ificom:
  case UD_Ificomp:
  case UD_Ifidiv:
  case UD_Ifidivr:
  case UD_Ifild:
  case UD_Ifimul:
  case UD_Ifincstp:
  case UD_Ifist:
  case UD_Ifistp:
  case UD_Ifisttp:
  case UD_Ifisub:
  case UD_Ifisubr:
  case UD_Ifld:
  case UD_Ifld1:
  case UD_Ifldcw:
  case UD_Ifldenv:
  case UD_Ifldl2e:
  case UD_Ifldl2t:
  case UD_Ifldlg2:
  case UD_Ifldln2:
  case UD_Ifldpi:
  case UD_Ifldz:
  case UD_Ifmul:
  case UD_Ifmulp:
  case UD_Ifndisi:
  case UD_Ifneni:
  case UD_Ifninit:
  case UD_Ifnop:
  case UD_Ifnsave:
  case UD_Ifnsetpm:
  case UD_Ifnstcw:
  case UD_Ifnstenv:
  case UD_Ifnstsw:
  case UD_Ifpatan:
  case UD_Ifprem:
  case UD_Ifprem1:
  case UD_Ifptan:
  case UD_Ifrndint:
  case UD_Ifrstor:
  case UD_Ifrstpm:
  case UD_Ifscale:
  case UD_Ifsin:
  case UD_Ifsincos:
  case UD_Ifsqrt:
  case UD_Ifst:
  case UD_Ifstp:
  case UD_Ifstp1:
  case UD_Ifstp8:
  case UD_Ifstp9:
  case UD_Ifsub:
  case UD_Ifsubp:
  case UD_Ifsubr:
  case UD_Ifsubrp:
  case UD_Iftst:
  case UD_Ifucom:
  case UD_Ifucomi:
  case UD_Ifucomip:
  case UD_Ifucomp:
  case UD_Ifucompp:
  case UD_Ifxam:
  case UD_Ifxch:
  case UD_Ifxch4:
  case UD_Ifxch7:
  case UD_Ifxrstor:
  case UD_Ifxsave:
  case UD_Ifxtract:
  case UD_Ifyl2x:
  case UD_Ifyl2xp1:
    chIdx = CD_FMS;
    break;
  }

  ud_casmprintf(u, 0x01, chIdx, "%s", ud_lookup_mnemonic(u->mnemonic));

  if (u->operand[0].type != UD_NONE)
  {
    int cast = 0;
    ud_casmprintf(u, 0x01, CD_DEFCLR, " ");

    if (u->operand[0].type == UD_OP_MEM)
    {
      if (u->operand[1].type == UD_OP_IMM ||
          u->operand[1].type == UD_OP_CONST ||
          u->operand[1].type == UD_NONE ||
          (u->operand[0].size != u->operand[1].size))
      {
        cast = 1;
      }
      else if (u->operand[1].type == UD_OP_REG &&
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
    gen_operand(u, &u->operand[0], cast);
  }

  if (u->operand[1].type != UD_NONE)
  {
    int cast = 0;
    ud_casmprintf(u, 0x01, CD_DEFCLR, ", ");
    if (u->operand[1].type == UD_OP_MEM &&
        u->operand[0].size != u->operand[1].size &&
        !ud_opr_is_sreg(&u->operand[0]))
    {
      cast = 1;
    }
    gen_operand(u, &u->operand[1], cast);
  }

  if (u->operand[2].type != UD_NONE)
  {
    int cast = 0;
    ud_casmprintf(u, 0x01, CD_DEFCLR, ", ");
    if (u->operand[2].type == UD_OP_MEM &&
        u->operand[2].size != u->operand[1].size)
    {
      cast = 1;
    }
    gen_operand(u, &u->operand[2], cast);
  }

  if (u->operand[3].type != UD_NONE)
  {
    ud_casmprintf(u, 0x01, CD_DEFCLR, ", ");
    gen_operand(u, &u->operand[3], 0);
  }
}

/*
vim: set ts=2 sw=2 expandtab
*/
