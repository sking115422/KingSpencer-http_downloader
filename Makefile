all:
	gcc main.c conn_utils.c f_clean_copy.c range_to_file.c -o http_downloader -lssl -lcrypto -lpthread

clean:
	rm -rf *.o *.gif *.jpg http_downloader