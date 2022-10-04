fd1 = open("hello.txt")

buf = fd1.read()

fd2 = open("output.txt", 'w')

fd2.write(buf)

print("File 'output.txt' has been created.")

fd1.close()
fd2.close()