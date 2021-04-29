all:
	gcc mftpserve.c mftp.h -o mftpserve
	gcc mftp.c mftp.h -o mftp
mftpserve:
	gcc mftpserve.c mftp.h -o mftpserve
mftpclient:
	gcc mftp.c mftp.h -o mftp
runserver:
	./mpftpserve
runclient:
	./mftp 49999 localhost
clean:
	rm -f mftpserve mpft
