# HTTPS Downloader

This program is designed to download files from a user input URL in a number of part specified by the user then recombine the parts to an output file specified by the user.
The program parses the given URL into a host name and file path. To interact with the server, the program creates a TCP socket and SSL session on top of that. The program 
then requests and parses the header for content length. It uses that information to make mulitple range requests in parallel to the server each over a seperate SSL session encapsulated in 
its own thread. The requests are then written to individual files, and then recompiled to a single output file. 
