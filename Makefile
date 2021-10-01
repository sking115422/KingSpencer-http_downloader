all:
	gcc main.c conn_utils.c range_to_file.c files_to_out.c -o http_downloader -lssl -lcrypto -lpthread

clean:
	rm -rf *.o part_* *.gif *.jpg http_downloader