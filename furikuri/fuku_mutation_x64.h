#pragma once

#pragma once

class fuku_mutation_x64 :
    public fuku_mutation {

    csh cap_handle;
    fuku_assambler f_asm;
    fuku_settings_obfuscation settings;


    void fuku_mutation_x64::fuku_junk(mutation_context& ctx);

    void fuku_mutation_x64::fukutation(fuku_code_holder& code_holder, linestorage::iterator& lines_iter);
    void fuku_mutation_x64::obfuscate_lines(fuku_code_holder& code_holder, linestorage::iterator lines_iter_begin, linestorage::iterator lines_iter_end, unsigned int recurse_idx);

public:
    fuku_mutation_x64::fuku_mutation_x64(const fuku_settings_obfuscation& settings);
    fuku_mutation_x64::~fuku_mutation_x64();

    void fuku_mutation_x64::obfuscate(fuku_code_holder& code_holder);

    void fuku_mutation_x64::get_junk(fuku_code_holder& code_holder, size_t junk_size, bool unstable_stack,
        uint64_t eflags_changes, uint64_t regs_changes);
};
