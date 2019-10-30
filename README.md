The steps to compile are as follows:
1- run 
	./build.sh
2- run 
	bin/server
3- For each client run the command below, where [NAME] is any name that you like but must be provided 
	bin/client [NAME]
4- Each client can type messages which will be broadcast to all clients

NOTE: If you want to convert this to an actuall application with UI, just use messages that come after ">>>>>", since they are the ones sent by the server to all connected clients.
