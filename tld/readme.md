# tld
tld can stand both for "Tiny LD" or "Tayoky's LD"  
tld is a small and portable linker targeting `i386` `x86_64` and `aarch64` (with cross linking option :D)  
> [!NOTE]
> relocation only work on `i386` for the moment

# specs
- format
  - binary
  - elf32
  - elf64
- arch
  - `x86_64` (no reloc)
  - `i386`
  - `aarch64` (no relocs)
- linker script
  - symbols declatation
  - input/output sections
  - `ENTRY`/`OUTPUT_FORMAT` directives
  - `ALIGN`/`BLOCK` function

# limitations
there are some big limitation that make tld non production ready
- no bss section
- no sections typed
- no common symbols
- no relocs on `aarch64`/`x86_64`
- no PHDR directivie
