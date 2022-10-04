import os

pid = os.fork()
if (pid == 0):
    print(f"Hello, I am child process. My process ID is: {os.getpid()}.")
else:
    print(f"Hello, I am parent process of child {pid}. My process ID is: {os.getpid()}.")