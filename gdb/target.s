.section .data

hello:.string "hello\n"
world:.string "world\n"

.section .text
.globl _start

_start:

#print hello
movl $4, %eax
movl $1, %ebx
movl $hello, %ecx
movl $6, %edx
int $0x80

#print world
movl $4, %eax
movl $1, %ebx
movl $world, %ecx
movl $6, %edx
int $0x80

movl $1, %eax
int $0x80

