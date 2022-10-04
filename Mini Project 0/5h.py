import os

pid = os.fork()

if (pid == 0):
    os.execlp("ls", "ls", "-l")
else:
    print(f"Hello, I am parent of {os.wait()[0]}. My process ID is: {os.getpid()}")