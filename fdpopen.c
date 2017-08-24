/*
** File   : fdpopen.c
** Author : Plan C
** GitHub : https://github.com/hubenchang0515
** Blog   : blog.kurukurumi.com
** E-main : hubenchang0515@outlook.com
**

MIT License

Copyright (c) 2017 Plan C

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <unistd.h>
#include <sys/wait.h>
#include "fdpopen.h"

/* 保存子进程的id */
static pid_t childpids[PIPE_MAX];


/* 创建进程并返回管道的描述符 */
int fdpopen(const char* cmd,const char* mode)
{
	/* 创建管道 */
	int pfd[2];
	if(pipe(pfd) == -1)
	{
		return -1;
	}
	
	/* 超过上限，关闭管道并返回 */
	if(pfd[0] >= PIPE_MAX || pfd[1] >= PIPE_MAX)
	{
		close(pfd[0]);
		close(pfd[1]);
		
		return -1;
	}
	
	pid_t pid = fork(); // 创建进程
	if(pid == 0) // 子进程
	{
		if(mode[0] == 'r')  // 父进程读,子进程写模式
		{
			close(pfd[0]); // 子进程关闭管道的读取端
			/* 将stdout重定向到管道的写端 */
			int out = dup2(pfd[1],STDOUT_FILENO); 
			if(out == -1)  // 重定向失败
			{	
				_exit(1);
			}
			close(pfd[1]); // 关闭旧的描述符
			/* 执行命令 */
			int rval = execl("/bin/sh","sh","-c",cmd,NULL); 
			close(out); // 关闭管道
			_exit(rval); // 子进程退出
		}
		else if(mode[0] == 'w') // 父进程写,子进程读模式
		{
			close(pfd[1]); // 子进程关闭写入端
			/* 将stdin重定向到管道的读取端 */
			int in = dup2(pfd[0],STDIN_FILENO);
			if(in == -1) // 重定向失败
			{
				_exit(1);
			}
			close(pfd[0]); // 关闭旧的描述符
			/* 执行命令 */
			int rval = execl("/bin/sh","sh","-c",cmd,NULL); 
			close(in);   // 关闭管道
			_exit(rval); // 子进程退出
		}
		else // 错误的模式
		{
			_exit(1);
		}
	}
	
	/* 父进程 */
	if(pid == -1) // fork失败 返回-1
	{
		return -1;
	}
	else if(mode[0] == 'r') // 父进程读模式
	{
		childpids[pfd[0]] = pid; // 保存子进程的pid
		return pfd[0]; // 返回管道读取端的描述符
	}
	else if(mode[0] == 'w') // 父进程写模式
	{
		childpids[pfd[1]] = pid; // 保存子进程的pid
		return pfd[1]; // 返回管道写入端的描述符
	}
	else
	{
		return -1;
	}

}

/* 关闭fdpopen打开的描述符 */
int fdpclose(int fd)
{
	int stat;
	close(fd); // 关闭管道
	waitpid(childpids[fd],&stat,0); // 等待子进程退出
	return stat; // 返回子进程的退出码
}


