#include "stdafx.h"
#include "fuku_mutation_x86_rules.h"

#define IsAllowedStackOperations (!HAS_FULL_MASK(ctx.instruction_flags, FUKU_INST_BAD_STACK))

static uint64_t di_fl_jcc[] = {
    EFLAGS_MOD_OF , EFLAGS_MOD_OF, //jo   / jno
    EFLAGS_MOD_CF , EFLAGS_MOD_CF, //jb   / jae
    EFLAGS_MOD_ZF , EFLAGS_MOD_ZF, //je   / jne
    EFLAGS_MOD_ZF | EFLAGS_MOD_CF, EFLAGS_MOD_ZF | EFLAGS_MOD_CF, //jbe / jnbe
    EFLAGS_MOD_SF , EFLAGS_MOD_SF, //js   / jns
    EFLAGS_MOD_PF , EFLAGS_MOD_PF, //jp   / jnp
    EFLAGS_MOD_OF | EFLAGS_MOD_SF, EFLAGS_MOD_OF | EFLAGS_MOD_SF, //jnge / jge
    EFLAGS_MOD_OF | EFLAGS_MOD_SF | EFLAGS_MOD_ZF, EFLAGS_MOD_OF | EFLAGS_MOD_SF | EFLAGS_MOD_ZF //jng / jnle
};


#define restore_disp_relocate(op) \
           if (op.get_type() == FUKU_T0_OPERAND && relocate_disp != -1) {\
                ctx.f_asm->get_context().inst->\
                set_relocation_disp_idx(relocate_disp);\
                ctx.code_holder->get_relocations()[relocate_disp].offset = ctx.f_asm->get_context().displacment_offset;\
           }

#define restore_imm_relocate(op) \
           if (inst_size == 4 && op.get_type() == FUKU_T0_IMMEDIATE && relocate_imm != -1) {\
                ctx.f_asm->get_context().inst->\
                set_relocation_imm_idx(relocate_imm);\
                ctx.code_holder->get_relocations()[relocate_imm].offset = ctx.f_asm->get_context().immediate_offset;\
           }

#define restore_rip_relocate_imm(op) \
           if (inst_size == 4 && op.get_type() == FUKU_T0_IMMEDIATE &&relocate_rip != -1) {\
                ctx.f_asm->get_context().inst->\
                set_rip_relocation_idx(relocate_rip);\
                ctx.code_holder->get_rip_relocations()[relocate_rip].offset = ctx.f_asm->get_context().immediate_offset;\
           }

#define restore_rip_relocate_disp(op) \
           if (inst_size == 4 && op.get_type() == FUKU_T0_OPERAND &&relocate_rip != -1) {\
                ctx.f_asm->get_context().inst->\
                set_rip_relocation_idx(relocate_rip);\
                ctx.code_holder->get_rip_relocations()[relocate_rip].offset = ctx.f_asm->get_context().displacment_offset;\
           }

#define restore_rip_to_imm_relocate(op) \
           if (op.get_type() == FUKU_T0_IMMEDIATE && relocate_rip != -1) {\
                size_t rip_label_idx = ctx.code_holder->get_rip_relocations()[relocate_rip].label_idx;\
                ctx.f_asm->get_context().inst->\
                set_relocation_imm_idx(\
                    ctx.code_holder->create_relocation_lb(\
                        ctx.f_asm->get_context().immediate_offset, rip_label_idx, 0\
                    )\
                );\
                ctx.code_holder->delete_rip_relocation(relocate_rip);\
           }

#define restore_rip_to_disp_relocate(op) \
           if (op.get_type() == FUKU_T0_OPERAND && relocate_rip != -1) {\
                size_t rip_label_idx = ctx.code_holder->get_rip_relocations()[relocate_rip].label_idx;\
                ctx.f_asm->get_context().inst->\
                set_relocation_disp_idx(\
                    ctx.code_holder->create_relocation_lb(\
                        ctx.f_asm->get_context().displacment_offset, rip_label_idx, 0\
                    )\
                );\
                ctx.code_holder->delete_rip_relocation(relocate_rip);\
           }

#define restore_imm_or_disp(op)         restore_disp_relocate(op) else restore_imm_relocate(op)
#define restore_rip_imm_or_disp(op)     restore_rip_relocate_imm(op) else restore_disp_relocate(op)
#define restore_rip_to_imm_or_disp(op)  restore_rip_to_imm_relocate(op) else restore_rip_to_disp_relocate(op)

#include "obf_templates/x86/obf_x86_mov_templates.h"
#include "obf_templates/x86/obf_x86_xchg_templates.h"
#include "obf_templates/x86/obf_x86_push_templates.h"
#include "obf_templates/x86/obf_x86_pop_templates.h"
#include "obf_templates/x86/obf_x86_lea_templates.h"

#include "obf_templates/x86/obf_x86_jcc_templates.h"
#include "obf_templates/x86/obf_x86_jmp_templates.h"
#include "obf_templates/x86/obf_x86_call_templates.h"
#include "obf_templates/x86/obf_x86_ret_templates.h"

#include "obf_templates/x86/obf_x86_add_templates.h"
#include "obf_templates/x86/obf_x86_sub_templates.h"
#include "obf_templates/x86/obf_x86_adc_templates.h"
#include "obf_templates/x86/obf_x86_sbb_templates.h"
#include "obf_templates/x86/obf_x86_cmp_templates.h"
#include "obf_templates/x86/obf_x86_neg_templates.h"
#include "obf_templates/x86/obf_x86_inc_templates.h"
#include "obf_templates/x86/obf_x86_dec_templates.h"
#include "obf_templates/x86/obf_x86_mul_templates.h"
#include "obf_templates/x86/obf_x86_imul_templates.h"
#include "obf_templates/x86/obf_x86_div_templates.h"
#include "obf_templates/x86/obf_x86_idiv_templates.h"

#include "obf_templates/x86/obf_x86_and_templates.h"
#include "obf_templates/x86/obf_x86_or_templates.h"
#include "obf_templates/x86/obf_x86_xor_templates.h"
#include "obf_templates/x86/obf_x86_test_templates.h"
#include "obf_templates/x86/obf_x86_not_templates.h"

#include "obf_templates/x86/obf_x86_ror_templates.h"
#include "obf_templates/x86/obf_x86_rol_templates.h"
#include "obf_templates/x86/obf_x86_rcl_templates.h"
#include "obf_templates/x86/obf_x86_rcr_templates.h"
#include "obf_templates/x86/obf_x86_shl_templates.h"
#include "obf_templates/x86/obf_x86_shr_templates.h"
#include "obf_templates/x86/obf_x86_sar_templates.h"

#include "obf_templates/x86/obf_x86_bt_templates.h"
#include "obf_templates/x86/obf_x86_bts_templates.h"
#include "obf_templates/x86/obf_x86_btr_templates.h"
#include "obf_templates/x86/obf_x86_btc_templates.h"
#include "obf_templates/x86/obf_x86_bsf_templates.h"
#include "obf_templates/x86/obf_x86_bsr_templates.h"


void fukutate_86_jcc(mutation_context& ctx) {
    ctx.was_mutated = _jcc_86_imm_tmpl(ctx); //jcc imm
}

void fukutate_86_jmp(mutation_context& ctx) {

    auto detail = ctx.instruction->detail->x86;

    if (detail.operands[0].type == X86_OP_REG) { //jmp reg
        ctx.was_mutated = _jmp_86_reg_tmpl(ctx);
    }
    else if (detail.operands[0].type == X86_OP_MEM) { //jmp [op]
        ctx.was_mutated = _jmp_86_op_tmpl(ctx);
    }
    else if (detail.operands[0].type == X86_OP_IMM) { //jmp imm
        ctx.was_mutated = _jmp_86_imm_tmpl(ctx);
    }
}

void fukutate_86_call(mutation_context& ctx) {

    auto detail = ctx.instruction->detail->x86;

    if (detail.operands[0].type == X86_OP_MEM) { //call [op]
        ctx.was_mutated = _call_86_op_tmpl(ctx);
    }
    else if (detail.operands[0].type == X86_OP_REG) {//call reg
        ctx.was_mutated = _call_86_reg_tmpl(ctx);
    }
    else if (detail.operands[0].type == X86_OP_IMM) {
        ctx.was_mutated = _call_86_imm_tmpl(ctx);
    }

}

void fukutate_86_ret(mutation_context& ctx) {
    ctx.was_mutated = _ret_86_imm_tmpl(ctx); //ret \ ret 0xXXXX
}

void fukutate_86_push(mutation_context& ctx) {

    auto detail = ctx.instruction->detail->x86;

    if (detail.operands[0].type == X86_OP_REG) {  //push reg
        ctx.was_mutated = _push_86_reg_tmpl(ctx);
    }
    else if (detail.operands[0].type == X86_OP_MEM) { //push [op]
        ctx.was_mutated = _push_86_op_tmpl(ctx);
    }
    else if (detail.operands[0].type == X86_OP_IMM) { //push imm8/imm32
        ctx.was_mutated = _push_86_imm_tmpl(ctx);
    }
}

void fukutate_86_pop(mutation_context& ctx) {

    auto detail = ctx.instruction->detail->x86;

    if (detail.operands[0].type == X86_OP_REG) { //pop reg
        ctx.was_mutated = _pop_86_reg_tmpl(ctx);
    }
    else if (detail.operands[0].type == X86_OP_MEM) { //pop [op]
        ctx.was_mutated = _pop_86_op_tmpl(ctx);
    }
}

void fukutate_86_mov(mutation_context& ctx) {

    auto detail = ctx.instruction->detail->x86;

    if (detail.operands[0].type == X86_OP_REG) {

        if (detail.operands[1].type == X86_OP_REG) { //mov reg, reg
            ctx.was_mutated = _mov_86_reg_reg_tmpl(ctx);
        }
        else if (detail.operands[1].type == X86_OP_IMM) {//mov reg, imm
            ctx.was_mutated = _mov_86_reg_imm_tmpl(ctx);
        }
        else if (detail.operands[1].type == X86_OP_MEM) {//mov reg, [op]
            ctx.was_mutated = _mov_86_reg_op_tmpl(ctx);
        }
    }
    else if (detail.operands[0].type == X86_OP_MEM) {

        if (detail.operands[1].type == X86_OP_REG) { //mov [op], reg
            ctx.was_mutated = _mov_86_op_reg_tmpl(ctx);
        }
        else if (detail.operands[1].type == X86_OP_IMM) {//mov [op], imm
            ctx.was_mutated = _mov_86_op_imm_tmpl(ctx);
        }
    }
}

void fukutate_86_xchg(mutation_context& ctx) {

    auto detail = ctx.instruction->detail->x86;

    if (detail.operands[0].type == X86_OP_MEM ||
        detail.operands[1].type == X86_OP_MEM ) { //xchg [op], reg
        ctx.was_mutated = _xchg_86_op_reg_tmpl(ctx);
    } 
    else { //xchg reg, reg
        ctx.was_mutated = _xchg_86_reg_reg_tmpl(ctx);
    }
}

void fukutate_86_lea(mutation_context& ctx) {

    
}

void fukutate_86_add(mutation_context& ctx) {

    auto detail = ctx.instruction->detail->x86;

    if (detail.operands[0].type == X86_OP_REG) {

        if (detail.operands[1].type == X86_OP_REG) { //add reg, reg
            ctx.was_mutated = _add_86_reg_reg_tmpl(ctx);
        }
        else if (detail.operands[1].type == X86_OP_IMM) {//add reg, imm
            ctx.was_mutated = _add_86_reg_imm_tmpl(ctx);
        }
        else if (detail.operands[1].type == X86_OP_MEM) {//add reg, [op]
            ctx.was_mutated = _add_86_reg_op_tmpl(ctx);
        }
    }
    else if (detail.operands[0].type == X86_OP_MEM) {

        if (detail.operands[1].type == X86_OP_REG) { //add [op], reg
            ctx.was_mutated = _add_86_op_reg_tmpl(ctx);
        }
        else if (detail.operands[1].type == X86_OP_IMM) {//add [op], imm
            ctx.was_mutated = _add_86_op_imm_tmpl(ctx);
        }
    }

}
void fukutate_86_or(mutation_context& ctx) {

    auto detail = ctx.instruction->detail->x86;

    if (detail.operands[0].type == X86_OP_REG) {

        if (detail.operands[1].type == X86_OP_REG) { //or reg, reg
            ctx.was_mutated = _add_86_reg_reg_tmpl(ctx);
        }
        else if (detail.operands[1].type == X86_OP_IMM) {//or reg, imm
            ctx.was_mutated = _add_86_reg_imm_tmpl(ctx);
        }
        else if (detail.operands[1].type == X86_OP_MEM) {//or reg, [op]
            ctx.was_mutated = _add_86_reg_op_tmpl(ctx);
        }
    }
    else if (detail.operands[0].type == X86_OP_MEM) {

        if (detail.operands[1].type == X86_OP_REG) { //or [op], reg
            ctx.was_mutated = _add_86_op_reg_tmpl(ctx);
        }
        else if (detail.operands[1].type == X86_OP_IMM) {//or [op], imm
            ctx.was_mutated = _add_86_op_imm_tmpl(ctx);
        }
    }
    
}
void fukutate_86_adc(mutation_context& ctx) {

}
void fukutate_86_sbb(mutation_context& ctx) {

}

void fukutate_86_and(mutation_context& ctx) {

    auto detail = ctx.instruction->detail->x86;

    if (detail.operands[0].type == X86_OP_REG) {

        if (detail.operands[1].type == X86_OP_REG) { //and reg, reg
            ctx.was_mutated = _and_86_reg_reg_tmpl(ctx);
        }
        else if (detail.operands[1].type == X86_OP_IMM) {//and reg, imm
            ctx.was_mutated = _and_86_reg_imm_tmpl(ctx);
        }
        else if (detail.operands[1].type == X86_OP_MEM) {//and reg, [op]
            ctx.was_mutated = _and_86_reg_op_tmpl(ctx);
        }
    }
    else if (detail.operands[0].type == X86_OP_MEM) {

        if (detail.operands[1].type == X86_OP_REG) { //and [op], reg
            ctx.was_mutated = _and_86_op_reg_tmpl(ctx);
        }
        else if (detail.operands[1].type == X86_OP_IMM) {//and [op], imm
            ctx.was_mutated = _and_86_op_imm_tmpl(ctx);
        }
    }
    
}

void fukutate_86_sub(mutation_context& ctx) {

    auto detail = ctx.instruction->detail->x86;

    if (detail.operands[0].type == X86_OP_REG) {

        if (detail.operands[1].type == X86_OP_REG) { //sub reg, reg
            ctx.was_mutated = _sub_86_reg_reg_tmpl(ctx);
        }
        else if (detail.operands[1].type == X86_OP_IMM) {//sub reg, imm
            ctx.was_mutated = _sub_86_reg_imm_tmpl(ctx);
        }
        else if (detail.operands[1].type == X86_OP_MEM) {//sub reg, [op]
            ctx.was_mutated = _sub_86_reg_op_tmpl(ctx);
        }
    }
    else if (detail.operands[0].type == X86_OP_MEM) {

        if (detail.operands[1].type == X86_OP_REG) { //sub [op], reg
            ctx.was_mutated = _sub_86_op_reg_tmpl(ctx);
        }
        else if (detail.operands[1].type == X86_OP_IMM) {//sub [op], imm
            ctx.was_mutated = _sub_86_op_imm_tmpl(ctx);
        }
    }
    
}
void fukutate_86_xor(mutation_context& ctx) {

    auto detail = ctx.instruction->detail->x86;

    if (detail.operands[0].type == X86_OP_REG) {

        if (detail.operands[1].type == X86_OP_REG) { //xor reg, reg
            ctx.was_mutated = _xor_86_reg_reg_tmpl(ctx);
        }
        else if (detail.operands[1].type == X86_OP_IMM) {//xor reg, imm
            ctx.was_mutated = _xor_86_reg_imm_tmpl(ctx);
        }
        else if (detail.operands[1].type == X86_OP_MEM) {//xor reg, [op]
            ctx.was_mutated = _xor_86_reg_op_tmpl(ctx);
        }
    }
    else if (detail.operands[0].type == X86_OP_MEM) {

        if (detail.operands[1].type == X86_OP_REG) { //xor [op], reg
            ctx.was_mutated = _xor_86_op_reg_tmpl(ctx);
        }
        else if (detail.operands[1].type == X86_OP_IMM) {//xor [op], imm
            ctx.was_mutated = _xor_86_op_imm_tmpl(ctx);
        }
    }
    
}

void fukutate_86_cmp(mutation_context& ctx) {

}

void fukutate_86_test(mutation_context& ctx) {

}


void fukutate_86_inc(mutation_context& ctx) {

}

void fukutate_86_dec(mutation_context& ctx) {
    
}

void fukutate_86_not(mutation_context& ctx) {

}

void fukutate_86_neg(mutation_context& ctx) {

}

void fukutate_86_mul(mutation_context& ctx) {

}

void fukutate_86_imul(mutation_context& ctx) {
    
}

void fukutate_86_div(mutation_context& ctx) {

}

void fukutate_86_idiv(mutation_context& ctx) {

}


void fukutate_86_rol(mutation_context& ctx) {

}

void fukutate_86_ror(mutation_context& ctx) {

}


void fukutate_86_rcl(mutation_context& ctx) {

}


void fukutate_86_rcr(mutation_context& ctx) {

}


void fukutate_86_shl(mutation_context& ctx) {

}


void fukutate_86_shr(mutation_context& ctx) {

}


void fukutate_86_sar(mutation_context& ctx) {

}


void fukutate_86_bt(mutation_context& ctx) {

}

void fukutate_86_bts(mutation_context& ctx) {

}

void fukutate_86_btr(mutation_context& ctx) {

}

void fukutate_86_btc(mutation_context& ctx) {

}

void fukutate_86_bsf(mutation_context& ctx) {

}

void fukutate_86_bsr(mutation_context& ctx) {

}

