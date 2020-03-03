.globl foo
foo:
  movl $1,     %eax # write(
  movl $1,     %edi #   fd=1,
  movq $s,     %rsi #   buf=s,
  movl $(e-s), %edx #   count=e-s
  syscall           # );

  movl $60, %eax    # exit(
  movl $1,  %edi    #   status=2
  syscall           # );

s:
  .ascii "\033[01;31mHello, OS World\033[0m\n"
e:

// code
.fill 1 << 20, 1, 0xcc

// data
.section .data
.fill 2 << 20, 1, 0xff

// bss
.section .bss
.fill 3 << 20
