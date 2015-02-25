#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/reg.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * 获取子进程系统调用号
 * */
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
		int status;
		while (1) {
			//等待子进程信号
			wait(&status);
			if (WIFEXITED(status)) //子进程发送退出信号，退出循环
				break;
			//调用ptrace从子进程取数据
			orig_eax = ptrace(PTRACE_PEEKUSER, child, 4 * ORIG_EAX, NULL);
			printf("orig_eax = %ld \n", orig_eax);
			//让子进程继续执行
			ptrace(PTRACE_SYSCALL, child, NULL, NULL);
		}
	}
	return 0;
}
