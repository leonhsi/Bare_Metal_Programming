import serial
import os

filename = "../lab1/kernel8.img"

#portname = "/dev/ttyUSB0"
#portname = "/dev/pts/1"
portname = "/dev/pts/2"

with open(portname, "wb", 115200) as tty:
	with open(filename, 'rb') as binary_file:

		command = 'lk\n'
		command_bytes = command.encode()
		print(command_bytes)
		tty.write(command_bytes)

		file_size = os.stat(filename).st_size
		file_str = str(file_size)
		file_str = file_str + '\n'
		file_bytes = file_str.encode()
		print (file_bytes)
		tty.write(file_bytes)

		print ("sending file [", filename, "] to port [", portname, "]")
		print ("file length : [", file_size, "]")
		
		offset = 0
		while (offset < file_size):
			binary_file.seek(offset, 0)
			#print(binary_file.read(1))
			tty.write(binary_file.read(1))
			offset = offset + 1

		print ("sending complete : ", offset, "bytes transfered.")

