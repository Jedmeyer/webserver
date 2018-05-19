A webserver that recieves GET requests and sends the recieved file.

Advanced features:

-IP logging: the logfile also writes the IP that recieved from the GET request. In testing, this has only been 127.0.0.1, since clients have been only on the local machine.

-Kill command: by sending a request with "QUIT password", the server will end all threads, and the proceed to end the main loop, closing the binded server socket descriptor, and then shutting down the server.
