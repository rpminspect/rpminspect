/*
 * Copyright (C) 2019  Red Hat, Inc.
 * Author(s):  David Shea <dshea@redhat.com>
 *             David Cantrell <dcantrell@redhat.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <fcntl.h>
#include <search.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/queue.h>
#include <sys/types.h>

#include <dlfcn.h>
#include <gnu/lib-names.h>
#include <link.h>

#include <gelf.h>
#include <libelf.h>

#include "inspect.h"
#include "readelf.h"
#include "rpminspect.h"

/* defined in inspect_elf_bits.c. See pic_bits.sh */
bool is_pic_reloc(Elf64_Half, Elf64_Xword);

/* Used by the fortified symbol checks */
static string_list_t *fortifiable = NULL;
static struct hsearch_data *fortifiable_table = NULL;

static bool is_fortified(const char *symbol);
static bool is_fortifiable(const char *symbol);

void init_elf_data(void)
{
    void *dl;
    struct link_map *info;
    char *libc_path;
    Elf *libc_elf;
    int libc_fd;
    string_list_t *libc_fortified;

    string_entry_t *entry;
    string_entry_t *iter;
    size_t symbol_len;
    size_t nentries;
    ENTRY e;
    ENTRY *eptr;

    /*
     * Use libdl to get the path to libc.so.6 so we can open it.
     * This is kind of lame, but avoids having to hardcode library paths
     * or call an external program or worry about 32 vs. 64-bit.
     */
    dl = dlopen(LIBC_SO, RTLD_LAZY);

    if (dl == NULL) {
        return;
    }

    if (dlinfo(dl, RTLD_DI_LINKMAP, &info) != 0) {
        dlclose(dl);
        return;
    }

    /* get_elf only operates on regular files, use realpath to resolve any symlinks */
    libc_path = realpath(info->l_name, NULL);

    if (libc_path == NULL) {
        dlclose(dl);
        return;
    }

    libc_elf = get_elf(libc_path, &libc_fd);
    free(libc_path);
    dlclose(dl);

    if (libc_elf == NULL) {
        return;
    }

    /* Get a list of all fortified symbols exported by glibc */
    libc_fortified = get_elf_exported_functions(libc_elf, is_fortified);

    if (libc_fortified == NULL) {
        elf_end(libc_elf);
        close(libc_fd);
        return;
    }

    fortifiable = malloc(sizeof(*fortifiable));
    assert(fortifiable != NULL);
    TAILQ_INIT(fortifiable);

    /* the symbols will be of the form, e.g., "__asprintf_chk". Turn that into "asprintf". */
    nentries = 0;
    TAILQ_FOREACH(iter, libc_fortified, items) {
        /* Skip this one */
        if (!strcmp(iter->data, "__chk_fail")) {
            continue;
        }

        symbol_len = strlen(iter->data);

        entry = calloc(1, sizeof(*entry));
        assert(entry != NULL);
        /* minus 2 underscores, minus for _chk, plus \0 */
        entry->data = calloc(symbol_len - 5, 1);
        assert(entry->data != NULL);

        /* strip off underscores, stop before _chk, \0 already present */
        strncpy(entry->data, iter->data + 2, symbol_len - 6);
        TAILQ_INSERT_TAIL(fortifiable, entry, items);
        nentries++;
    }

    list_free(libc_fortified, NULL);
    elf_end(libc_elf);
    close(libc_fd);

    /* The fortifiable tailq is to keep track of what all's been malloced.
     * Copy into a hash table for fast lookups.
     */
    fortifiable_table = calloc(1, sizeof(*fortifiable_table));
    assert(fortifiable_table != NULL);

    if (hcreate_r(nentries, fortifiable_table) == 0) {
        free(fortifiable_table);
        fortifiable_table = NULL;
        return;
    }

    TAILQ_FOREACH(iter, fortifiable, items) {
        e.key = iter->data;
        e.data = iter->data;
        hsearch_r(e, ENTER, &eptr, fortifiable_table);
    }
}

void free_elf_data(void)
{
    if (fortifiable_table != NULL) {
        hdestroy_r(fortifiable_table);
        free(fortifiable_table);
        fortifiable_table = NULL;
    }

    if (fortifiable != NULL) {
        list_free(fortifiable, free);
        fortifiable = NULL;
    }
}

/* Check whether the given object file has information about
 * the stack settings. This is contained in the GNU_STACK program
 * header for ET_EXEC and ET_DYN, and in the .note.GNU-stack
 * section for ET_REL
 */
bool is_execstack_present(Elf *elf)
{
    GElf_Phdr phdr;

    switch (get_elf_type(elf)) {
        case ET_REL:
            return have_elf_section(elf, SHT_PROGBITS, ".note.GNU-stack");
        case ET_EXEC:
        case ET_DYN:
            return (get_elf_phdr(elf, PT_GNU_STACK, &phdr) != NULL);
        default:
            return false;
    }
}

/* Return the flags value from the execstack data.
 * This is either the p_flags from the GNU_STACK program header entry
 * or the sh_flags from the .note.GNU-stack section.
 */
uint64_t get_execstack_flags(Elf *elf)
{
    GElf_Phdr phdr;
    Elf_Scn *scn;
    GElf_Shdr shdr;

    switch (get_elf_type(elf)) {
        case ET_REL:
            scn = get_elf_section(elf, SHT_PROGBITS, ".note.GNU-stack", NULL, &shdr);

            if (scn == NULL) {
                return 0;
            }

            return shdr.sh_flags;
        case ET_EXEC:
        case ET_DYN:
            if (get_elf_phdr(elf, PT_GNU_STACK, &phdr) == NULL) {
                return 0;
            }

            return phdr.p_flags;
        default:
            return 0;
    }
}

/* Return true if this object has a SHT_PROGBITS section with SHF_EXECINSTR set.
 * This is a way of filtering out the ET_REL DWARF objects in /usr/lib/debug/.dwz,
 * which have no executable code.
 */
bool has_executable_program(Elf *elf)
{
    Elf_Scn *scn = NULL;
    GElf_Shdr shdr;

    while ((scn = get_elf_section(elf, SHT_PROGBITS, NULL, scn, &shdr)) != NULL) {
        if (shdr.sh_flags & SHF_EXECINSTR) {
            return true;
        }
    }

    return false;
}

/* Check whether the given object's execstack information makes sense.
 * For ET_EXEC and ET_DYN, PF_W and PF_R must be set. For ET_REL,
 * nothing other than SHF_EXECINSTR should be set.
 * flags is the value returned by get_execstack_flags. The Elf pointer
 * is still required to figure out which flags they are.
 */
bool is_execstack_valid(Elf *elf, uint64_t flags)
{
    switch (get_elf_type(elf)) {
        case ET_REL:
            /* Mask out SHF_EXECINSTR, check that nothing else is set */
            return !(flags & ~(SHF_EXECINSTR));
        case ET_EXEC:
        case ET_DYN:
            /* PF_W and PF_R must be set, nothing besides those two and PF_X should be set */
            return ((flags & (PF_W | PF_R)) && !(flags & ~(PF_W | PF_R | PF_X)));
        default:
            return false;
    }
}

/* Like above, but return true if the relevant executable bit is set */
bool is_stack_executable(Elf *elf, uint64_t flags)
{
    switch (get_elf_type(elf)) {
        case ET_REL:
            return flags & SHF_EXECINSTR;
        case ET_EXEC:
        case ET_DYN:
            return flags & PF_X;
        default:
            return false;
    }
}

/* Return true if this object has a DT_TEXTREL entry */
bool has_textrel(Elf *elf)
{
    return have_dynamic_tag(elf, DT_TEXTREL);
}

/* true if there is a PT_GNU_RELRO phdr */
bool has_relro(Elf *elf)
{
    GElf_Phdr phdr;
    return (get_elf_phdr(elf, PT_GNU_RELRO, &phdr) != NULL);
}

/* true if there is a DT_BIND_NOW entry */
bool has_bind_now(Elf *elf)
{
    return have_dynamic_tag(elf, DT_BIND_NOW);
}

static bool is_fortified(const char *symbol)
{
    /* Besides the fortified versions of functions, look for the function
     * that gets calls on buffer overflow
     */
    if (!strcmp(symbol, "__chk_fail")) {
        return true;
    }

    return (strprefix(symbol, "__") && strsuffix(symbol, "_chk"));
}

static bool is_fortifiable(const char *symbol)
{
    ENTRY e;
    ENTRY *eptr;
    e.key = (char *) symbol;
    hsearch_r(e, FIND, &eptr, fortifiable_table);
    return eptr != NULL;
}

/* Return a list of fortified symbols found linked in the given ELF object */
string_list_t * get_fortified_symbols(Elf *elf)
{
    return get_elf_imported_functions(elf, is_fortified);
}

/* Return a list of linked symbols that could have been fortified but are not */
string_list_t * get_fortifiable_symbols(Elf *elf)
{
    return get_elf_imported_functions(elf, is_fortifiable);
}

/* Check the referenced symbol for global binding. */
static bool is_global_reloc(GElf_Shdr *symtab_shdr, Elf_Data *symtab_data, Elf_Data *symtab_xdata, uint64_t r_sym)
{
    GElf_Sym sym;
    size_t num_syms;

    /* Sanity check, make sure the symbol index isn't bigger than the symbol table */
    assert(symtab_shdr->sh_entsize > 0);
    num_syms = symtab_shdr->sh_size / symtab_shdr->sh_entsize;

    if (r_sym >= num_syms) {
        return false;
    }

    if (gelf_getsymshndx(symtab_data, symtab_xdata, r_sym, &sym, NULL) == NULL) {
        return false;
    }

    if (GELF_ST_BIND(sym.st_info) == STB_GLOBAL) {
        return true;
    }

    return false;
}

/* Given the ET_REL object, return whether we think it was compiled with -fPIC */
/* This is kind of iffy. Whether the relocations in a given ELF object are PIC or
 * not depend on the type of relocation encoded in r_info, and all of the relocation
 * types are processor specific. This code uses the function is_pic_reloc, which is
 * generated by pic_bits.sh, which just greps the R_<arch>_* constants for the the
 * ones that include "PLT" or "GOT" in the macro name.
 *
 * All together, the idea is:
 *   * iterate over all relocations
 *   * if the relocation is for a symbol of binding other than STB_GLOBAL, it's probably fine
 *   * otherwise, if the relocation type doesn't pass is_pic_reloc, return false.
 */
bool is_pic_ok(Elf *elf)
{
    GElf_Ehdr ehdr;
    Elf_Scn *rel_section;
    GElf_Shdr rel_shdr;
    Elf_Data *rel_data = NULL;

    Elf_Scn *symtab_section;
    GElf_Shdr symtab_shdr;
    Elf_Data *symtab_data;

    Elf_Scn *xndxscn = NULL;
    Elf_Data *xndxdata = NULL;

    GElf_Rel rel;
    GElf_Rela rela;

    size_t entry_size;
    size_t i;

    if (gelf_getehdr(elf, &ehdr) == NULL) {
        return true;
    }

    /* Fetch the symtab data */
    if ((symtab_section = get_elf_section(elf, SHT_SYMTAB, NULL, NULL, &symtab_shdr)) == NULL) {
        return true;
    }

    if ((symtab_data = elf_getdata(symtab_section, NULL)) == NULL) {
        return true;
    }

    if ((xndxscn = get_elf_extended_section(elf, symtab_section, NULL)) != NULL) {
        if ((xndxdata = elf_getdata(xndxscn, NULL)) == NULL) {
            return true;
        }
    }

    /* Look for a SHT_RELA section first */
    rel_section = get_elf_section(elf, SHT_RELA, ".rela.text", NULL, &rel_shdr);

    if (rel_section != NULL) {
        while ((rel_data = elf_getdata(rel_section, rel_data)) != NULL) {
            if (!(entry_size = gelf_fsize(elf, rel_data->d_type, 1, EV_CURRENT))) {
                continue;
            }

            for (i = 0; i < rel_shdr.sh_size / entry_size; i++) {
                if (gelf_getrela(rel_data, i, &rela) == NULL) {
                    continue;
                }

                if (is_global_reloc(&symtab_shdr, symtab_data, xndxdata, GELF_R_SYM(rela.r_info)) &&
                        !is_pic_reloc(ehdr.e_machine, GELF_R_TYPE(rela.r_info))) {
                    return false;
                }
            }
        }
    }

    /* Try again with SHT_REL */
    rel_section = get_elf_section(elf, SHT_REL, ".rel.text", NULL, &rel_shdr);
    rel_data = NULL;

    if (rel_section != NULL) {
        while ((rel_data = elf_getdata(rel_section, rel_data)) != NULL) {
            if (!(entry_size = gelf_fsize(elf, rel_data->d_type, 1, EV_CURRENT))) {
                continue;
            }

            for (i = 0; i < rel_shdr.sh_size / entry_size; i++) {
                if (gelf_getrel(rel_data, i, &rel) == NULL) {
                    continue;
                }

                if (is_global_reloc(&symtab_shdr, symtab_data, xndxdata, GELF_R_SYM(rel.r_info)) &&
                        !is_pic_reloc(ehdr.e_machine, GELF_R_TYPE(rel.r_info))) {
                    return false;
                }
            }
        }
    }

    return true;
}

static const char * pflags_to_str(uint64_t flags)
{
    /* enough space for RWX?\0 */
    static char output[5];
    char *current = output;

    memset(output, 0, sizeof(output));

    if (flags & PF_R) {
        *current = 'R';
        current++;
    }

    if (flags & PF_W) {
        *current = 'W';
        current++;
    }

    if (flags & PF_X) {
        *current = 'X';
        current++;
    }

    if (flags & ~(PF_R|PF_W|PF_X)) {
        *current = '?';
        current++;
    }

    return output;
}

static bool inspect_elf_execstack(struct rpminspect *ri, Elf *after_elf, Elf *before_elf, const char *localpath, const char *arch)
{
    Elf64_Half elf_type;
    uint64_t execstack_flags;
    bool result = false;
    char *msg = NULL;

    bool before_execstack = false;
    severity_t severity;

    /* If there is no executable code, there is no executable stack */
    if (!has_executable_program(after_elf)) {
        return true;
    }

    elf_type = get_elf_type(after_elf);

    /* If the peer file had an executable stack, turn down the result severity */
    if (before_elf) {
        before_execstack = is_stack_executable(before_elf, get_execstack_flags(before_elf));
    }

    /* Check if execstack information is present */
    if (!is_execstack_present(after_elf)) {
        if (elf_type == ET_REL) {
            /* Missing .note.GNU-stack will result in an executable stack */
            if (before_execstack) {
                xasprintf(&msg, "Object still has executable stack (no GNU-stack note): %s on %s", localpath, arch);
                severity = RESULT_VERIFY;
            } else {
                xasprintf(&msg, "Object has executable stack (no GNU-stack note): %s on %s", localpath, arch);
                severity = RESULT_BAD;
            }
        } else {
            xasprintf(&msg, "Program built without GNU_STACK: %s on %s", localpath, arch);
            severity = RESULT_BAD;
        }

        add_result(&ri->results, severity, WAIVABLE_BY_SECURITY, HEADER_ELF, msg, NULL, REMEDY_ELF_EXECSTACK_MISSING);
        goto cleanup;
    }

    /* Check that the execstack flags make sense */
    execstack_flags = get_execstack_flags(after_elf);

    if (!is_execstack_valid(after_elf, execstack_flags)) {
        if (elf_type == ET_REL) {
            xasprintf(&msg, "File %s has invalid execstack flags %lX on %s", localpath, execstack_flags, arch);

            add_result(&ri->results, RESULT_BAD, WAIVABLE_BY_SECURITY, HEADER_ELF, msg, NULL, REMEDY_ELF_EXECSTACK_INVALID);
        } else {
            xasprintf(&msg, "File %s has unrecognized GNU_STACK '%s' (expected RW or RWE) on %s", localpath, pflags_to_str(execstack_flags), arch);

            add_result(&ri->results, RESULT_BAD, WAIVABLE_BY_SECURITY, HEADER_ELF, msg, NULL, REMEDY_ELF_EXECSTACK_INVALID);
        }

        goto cleanup;
    }

    /* Check that the stack is not marked as executable */
    if (is_stack_executable(after_elf, execstack_flags)) {
        if (elf_type == ET_REL) {
            if (before_execstack) {
                xasprintf(&msg, "Object still has executable stack (GNU-stack note = X): %s on %s", localpath, arch);
                severity = RESULT_VERIFY;
            } else {
                xasprintf(&msg, "Object has executable stack (GNU-stack note = X): %s on %s", localpath, arch);
                severity = RESULT_BAD;
            }
        } else {
            if (before_execstack) {
                xasprintf(&msg, "Stack is still executable: %s on %s", localpath, arch);
                severity = RESULT_VERIFY;
            } else {
                xasprintf(&msg, "Stack is executable: %s on %s", localpath, arch);
                severity = RESULT_BAD;
            }
        }

        add_result(&ri->results, severity, WAIVABLE_BY_SECURITY, HEADER_ELF, msg, NULL, REMEDY_ELF_EXECSTACK_EXECUTABLE);
        goto cleanup;
    }

    result = true;

cleanup:
    free(msg);

    return result;
}

static bool check_relro(struct rpminspect *ri, Elf *before_elf, Elf *after_elf, const char *localpath, const char *arch)
{
    bool before_relro = has_relro(before_elf);
    bool before_bind_now = has_bind_now(before_elf);
    bool after_relro = has_relro(after_elf);
    bool after_bind_now = has_bind_now(after_elf);
    char *msg = NULL;

    if (before_relro && before_bind_now && after_relro && !after_bind_now) {
        /* full relro in before, partial relro in after */
        xasprintf(&msg, "%s lost full GNU_RELRO security protection on %s", localpath, arch);
    } else if (before_relro && !after_relro) {
        /* partial or full relro in before, no relro in after */
        xasprintf(&msg, "%s lost GNU_RELRO security protection on %s", localpath, arch);
    }

    if (msg != NULL) {
        add_result(&ri->results, RESULT_BAD, WAIVABLE_BY_SECURITY, HEADER_ELF, msg, NULL, REMEDY_ELF_GNU_RELRO);
        free(msg);
        return false;
    }

    return true;
}

/* Check for binaries that had fortified symbols in before, and have no fortified symbols in after.
 * This could indicate a loss of hardening build flags.
 */
static bool check_fortified(struct rpminspect *ri, Elf *before_elf, Elf *after_elf, const char *localpath, const char *arch)
{
    string_list_t *before_fortifiable = NULL;
    string_list_t *before_fortified = NULL;
    string_list_t *after_fortifiable = NULL;
    string_list_t *after_fortified = NULL;
    string_list_t *sorted_list;
    string_entry_t *iter;

    FILE *output_stream;
    char *output_buffer = NULL;
    size_t output_size;
    int output_result;

    char *msg;
    bool result = true;

    /* ignore unused variable warnings if assert is disabled */
    (void) output_result;

    /* If "before" had no fortified symbols, it can't lose fortified symbols. Return. */
    before_fortified = get_fortified_symbols(before_elf);
    assert(before_fortified != NULL);

    if ((before_fortified == NULL) || TAILQ_EMPTY(before_fortified)) {
        goto cleanup;
    }

    /*
     * If "after" has any fortified symbols, then at least some of it was compiled with
     * -D_FORTIFY_SOURCE. Assume it's fine.
     */
    after_fortified = get_fortified_symbols(after_elf);
    assert(after_fortified != NULL);

    if ((after_fortified != NULL) && !TAILQ_EMPTY(after_fortified)) {
        goto cleanup;
    }

    /* If "after" has no fortifiable symbols, it's fine. */
    after_fortifiable = get_fortifiable_symbols(after_elf);
    assert(after_fortifiable != NULL);

    if ((after_fortifiable == NULL) || TAILQ_EMPTY(after_fortifiable)) {
        goto cleanup;
    }

    /*
     * At this point:
     *   - before is definitely fortified,
     *   - after shows no sign of being fortified, and
     *   - after contains symbols that we think could have been fortified.
     */
    result = false;

    /* Create a screendump listing the symbols involved. */
    output_stream = open_memstream(&output_buffer, &output_size);
    assert(output_stream != NULL);

    output_result = fprintf(output_stream, "Fortified symbols lost:\n");
    assert(output_result > 0);

    sorted_list = list_sort(before_fortified);
    assert(sorted_list != NULL);

    TAILQ_FOREACH(iter, sorted_list, items) {
        output_result = fprintf(output_stream, "\t%s\n", iter->data);
        assert(output_result > 0);
    }

    list_free(sorted_list, NULL);

    output_result = fprintf(output_stream, "Fortifiable symbols present:\n");
    assert(output_result > 0);

    sorted_list = list_sort(after_fortifiable);
    assert(sorted_list != NULL);

    TAILQ_FOREACH(iter, sorted_list, items) {
        output_result = fprintf(output_stream, "\t%s\n", iter->data);
        assert(output_result > 0);
    }

    list_free(sorted_list, NULL);

    output_result = fclose(output_stream);
    assert(output_result == 0);

    xasprintf(&msg, "%s may have lost -D_FORTIFY_SOURCE on %s", localpath, arch);
    add_result(&ri->results, RESULT_VERIFY, WAIVABLE_BY_SECURITY, HEADER_ELF, msg, output_buffer, REMEDY_ELF_FORTIFY_SOURCE);
    free(msg);

cleanup:
    list_free(before_fortifiable, NULL);
    list_free(before_fortified, NULL);
    list_free(after_fortifiable, NULL);
    list_free(after_fortified, NULL);

    free(output_buffer);

    return result;
}

/* Helper for elf_archive_tests; add the archive member to the list if compiled *without* -fPIC */
static bool find_no_pic(Elf *elf, void *user_data)
{
    string_list_t *no_pic_list = (string_list_t *) user_data;
    string_entry_t *entry;
    Elf_Arhdr *arhdr;

    if ((arhdr = elf_getarhdr(elf)) == NULL) {
        return true;
    }

    /* Skip the / entry */
    if (!strcmp(arhdr->ar_name, "/")) {
        return true;
    }

    if (!is_pic_ok(elf)) {
        entry = calloc(1, sizeof(*entry));
        assert(entry != NULL);

        entry->data = strdup(arhdr->ar_name);
        assert(entry->data != NULL);

        TAILQ_INSERT_TAIL(no_pic_list, entry, items);
    }

    return true;
}

/* Helper for elf_archive_tests, add the archive member to the list if compiled *with* -fPIC */
static bool find_pic(Elf *elf, void *user_data)
{
    string_list_t *pic_list = (string_list_t *) user_data;
    string_entry_t *entry;
    Elf_Arhdr *arhdr;

    if ((arhdr = elf_getarhdr(elf)) == NULL) {
        return true;
    }

    /* Skip the / entry */
    if (!strcmp(arhdr->ar_name, "/")) {
        return true;
    }

    if (is_pic_ok(elf)) {
        entry = calloc(1, sizeof(*entry));
        assert(entry != NULL);

        entry->data = strdup(arhdr->ar_name);
        assert(entry->data != NULL);

        TAILQ_INSERT_TAIL(pic_list, entry, items);
    }

    return true;
}

/* Helper for elf_archive_tests, get all archive member names */
static bool find_all(Elf *elf, void *user_data)
{
    string_list_t *all_list = (string_list_t *) user_data;
    string_entry_t *entry;
    Elf_Arhdr *arhdr;

    if ((arhdr = elf_getarhdr(elf)) == NULL) {
        return true;
    }

    /* Skip the / entry */
    if (!strcmp(arhdr->ar_name, "/")) {
        return true;
    }

    entry = calloc(1, sizeof(*entry));
    assert(entry != NULL);

    entry->data = strdup(arhdr->ar_name);
    assert(entry->data != NULL);

    TAILQ_INSERT_TAIL(all_list, entry, items);

    return true;
}

static bool elf_archive_tests(struct rpminspect *ri, Elf *after_elf, int after_elf_fd, Elf *before_elf, int before_elf_fd, const char *localpath, const char *arch)
{
    string_list_t *after_no_pic = NULL;
    string_list_t *before_pic = NULL;
    string_list_t *before_all = NULL;

    string_list_t *after_lost_pic = NULL;
    string_list_t *after_new = NULL;

    string_entry_t *iter;

    FILE *output_stream;
    char *screendump = NULL;
    size_t screendump_size;
    int output_result;

    char *msg = NULL;
    bool result = true;

    /* ignore unused variable warnings if assert is disabled */
    (void) output_result;

    /* comparison-only, skip if no before */
    if (!before_elf) {
        return result;
    }

    after_no_pic = calloc(1, sizeof(*after_no_pic));
    assert(after_no_pic != NULL);

    TAILQ_INIT(after_no_pic);
    elf_archive_iterate(after_elf_fd, after_elf, find_no_pic, after_no_pic);

    /* If everything in after looks ok, we're done */
    if (TAILQ_EMPTY(after_no_pic)) {
        goto cleanup;
    }

    /* Gather data for two possible messages:
     *   - Objects in after that had -fPIC in before
     *   - Objects in after that are completely new
     *
     * It's still possible for this test to pass if everything without -fPIC in after
     * also did not have -fPIC in before.
     */

    output_stream = open_memstream(&screendump, &screendump_size);
    assert(output_stream != NULL);

    before_pic = calloc(1, sizeof(*before_pic));
    assert(before_pic != NULL);

    TAILQ_INIT(before_pic);
    elf_archive_iterate(before_elf_fd, before_elf, find_pic, before_pic);

    after_lost_pic = list_intersection(after_no_pic, before_pic);
    assert(after_lost_pic != NULL);

    if (!TAILQ_EMPTY(after_lost_pic)) {
        result = false;

        output_result = fprintf(output_stream, "The following objects lost -fPIC:\n");
        assert(output_result > 0);

        TAILQ_FOREACH(iter, after_lost_pic, items) {
            output_result = fprintf(output_stream, "\t%s\n", iter->data);
            assert(output_result > 0);
        }
    }

    before_all = calloc(1, sizeof(*before_all));
    assert(before_all != NULL);

    TAILQ_INIT(before_all);
    elf_archive_iterate(before_elf_fd, before_elf, find_all, before_all);

    after_new = list_difference(after_no_pic, before_all);
    assert(after_new != NULL);

    if (!TAILQ_EMPTY(after_new)) {
        result = false;

        output_result = fprintf(output_stream, "The following new objects were built without -fPIC:\n");
        assert(output_result > 0);

        TAILQ_FOREACH(iter, after_new, items) {
            output_result = fprintf(output_stream, "\t%s\n", iter->data);
            assert(output_result > 0);
        }
    }

    output_result = fclose(output_stream);
    assert(output_result == 0);

    if (!result) {
        xasprintf(&msg, "%s lost -fPIC on %s", localpath, arch);
        add_result(&ri->results, RESULT_BAD, WAIVABLE_BY_SECURITY, HEADER_ELF, msg, screendump, REMEDY_ELF_FPIC);
    }

cleanup:
    list_free(after_lost_pic, NULL);
    list_free(after_new, NULL);

    list_free(after_no_pic, free);
    list_free(before_pic, free);
    list_free(before_all, free);

    free(screendump);
    free(msg);

    return result;
}

static bool elf_regular_tests(struct rpminspect *ri, Elf *after_elf, Elf *before_elf, const char *localpath, const char *arch)
{
    char *msg = NULL;
    bool result = true;

    if (!inspect_elf_execstack(ri, after_elf, before_elf, localpath, arch)) {
        result = false;
    }

    if (has_textrel(after_elf)) {
        /* Only complain for baseline (no before), or for gaining TEXTREL between before and after. */
        if (before_elf && !has_textrel(before_elf)) {
            xasprintf(&msg, "%s acquired TEXTREL relocations on %s", localpath, arch);
        } else if (!before_elf) {
            xasprintf(&msg, "%s has TEXTREL relocations on %s", localpath, arch);
        }

        if (msg != NULL) {
            add_result(&ri->results, RESULT_BAD, WAIVABLE_BY_SECURITY, HEADER_ELF, msg, NULL, REMEDY_ELF_TEXTREL);
            result = false;
            free(msg);
            msg = NULL;
        }
    }

    if (before_elf) {
        /* Check if we lost GNU_RELRO */
        if (!check_relro(ri, after_elf, before_elf, localpath, arch)) {
            result = false;
        }

        /* Check if the object lost fortified symbols or gained unfortified, fortifiable symbols */
        if (!check_fortified(ri, after_elf, before_elf, localpath, arch)) {
            result = false;
        }
    }

    return result;
}

static bool elf_driver(struct rpminspect *ri, rpmfile_entry_t *after)
{
    const char *arch;
    Elf *after_elf = NULL;
    Elf *before_elf = NULL;
    int after_elf_fd;
    int before_elf_fd;
    bool result = true;

    /* Skip source packages */
    if (headerIsSource(after->rpm_header)) {
        return true;
    }

    if (!after->fullpath || !S_ISREG(after->st.st_mode)) {
        return true;
    }

    if (!process_file_path(after, ri->elf_path_include, ri->elf_path_exclude)) {
        return true;
    }

    arch = headerGetString(after->rpm_header, RPMTAG_ARCH);

    /* Is this an archive or a regular ELF file? */
    if ((after_elf = get_elf_archive(after->fullpath, &after_elf_fd)) != NULL) {
        if (after->peer_file != NULL) {
            before_elf = get_elf_archive(after->peer_file->fullpath, &before_elf_fd);
        }

        result = elf_archive_tests(ri, after_elf, after_elf_fd, before_elf, before_elf_fd, after->localpath, arch);
    } else if ((after_elf = get_elf(after->fullpath, &after_elf_fd)) != NULL) {
        if (after->peer_file != NULL) {
            before_elf = get_elf_archive(after->peer_file->fullpath, &before_elf_fd);
        }

        result = elf_regular_tests(ri, after_elf, before_elf, after->localpath, arch);
    }


    if (after_elf) {
        elf_end(after_elf);
        close(after_elf_fd);
    }

    if (before_elf) {
        elf_end(before_elf);
        close(before_elf_fd);
    }

    return result;
}

bool inspect_elf(struct rpminspect *ri)
{
    bool result;

    init_elf_data();
    result = foreach_peer_file(ri, elf_driver);
    free_elf_data();

    if (result) {
        add_result(&ri->results, RESULT_OK, WAIVABLE_BY_ANYONE, HEADER_ELF, NULL, NULL, NULL);
    }

    return result;
}
