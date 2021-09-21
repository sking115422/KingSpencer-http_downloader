all:
	gcc main.c tcp_conn.c tls_sess.c f_clean_copy.c -o http_downloader -lssl -lcrypto

clean:
	rm -rf *.o *.gif *.jpg http_downloader