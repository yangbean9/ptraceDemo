#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/reg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int word_size = sizeof(int); //字长,本机系统是32位，所以字长应该为4字节

/**
 * 反转str
 * */
void reverse(char *str) {

	int i, j;
	char temp;
	for (i = 0, j = strlen(str) - 2; i <= j; ++i, --j) {
		temp = str[i];
		str[i] = str[j];
		str[j] = temp;
	}
}

/**
 * 从子进程读数据
 * */
void getdata(pid_t child, long addr, char *str, int len) {

	char *laddr;
	int i, j;

	union u {
		long val;
		char chars[word_size];
	} data; //联合体，val和chars指向同一块内存

	i = 0;
	j = len / word_size;

	laddr = str;

	while (i < j) {
		//每次读一个字，也就是读4byte
		data.val = ptrace(PTRACE_PEEKDATA, child, addr + i * 4, NULL);
		//拷贝到事先分配好的字符串地址
		memcpy(laddr, data.chars, word_size);
		++i;
		laddr += word_size;
	}

	//读最后小于4字节的几个字节内容
	j = len % word_size;
	if (j != 0) {
		data.val = ptrace(PTRACE_PEEKDATA, child, addr + i * 4, NULL);
		memcpy(laddr, data.chars, j);
	}
	//结束符
	str[len] = '\0';
}

/**
 * 往子进程写数据
 * */
void putdata(pid_t child, long addr, char *str, int len) {

	char *laddr;
	int i, j;
	union u {
		long val;
		char chars[word_size];
	} data;

	i = 0;
	j = len / word_size;
	laddr = str;

	while (i < j) {
		memcpy(data.chars, laddr, word_size);
		//每次写入一个字
		ptrace(PTRACE_POKEDATA, child, addr + i * 4, data.val);
		++i;
		laddr += word_size;
	}
	j = len % word_size;
	if (j != 0) {
		memcpy(data.chars, laddr, j);
		ptrace(PTRACE_POKEDATA, child, addr + i * 4, data.val);
	}
}

int main() {

	pid_t child;
	child = fork();
	if (child < 0) {
		perror("fork error");
	} else if (child == 0) {
		//子进程执行
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		execl("hello", "hello", NULL);
	} else {
		//父进程执行
		long orig_eax;
		long params[3];
		int status;
		char *str, *laddr;
		int toggle = 0;
		while (1) {
			//等待子进程信号
			wait(&status);
			if (WIFEXITED(status)) //遇到子进程退出信号，退出循环
				break;
			orig_eax = ptrace(PTRACE_PEEKUSER, child, 4 * ORIG_EAX, NULL);
			if (orig_eax == SYS_write) {
				if (toggle == 0) {
					toggle = 1;
					params[0] = ptrace(PTRACE_PEEKUSER, child, 4 * EBX, NULL); //fd
					params[1] = ptrace(PTRACE_PEEKUSER, child, 4 * ECX, NULL); //str地址
					params[2] = ptrace(PTRACE_PEEKUSER, child, 4 * EDX, NULL); //str长度
					str = (char *) calloc((params[2] + 1), sizeof(char));
					getdata(child, params[1], str, params[2]);
					reverse(str);
					putdata(child, params[1], str, params[2]);
				} else {
					toggle = 0;
				}
			}
			ptrace(PTRACE_SYSCALL, child, NULL, NULL);
		}
	}
	return 0;
}
