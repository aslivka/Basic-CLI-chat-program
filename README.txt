==============================================================================
NOTE: Chat and server program can only communicate in turns.
Testing environment: any flip server

==============================================================================
Compiling Client/Server programs
1. Enter command "make" in flip terminal
2. Manual compilation commands below:
Compiling client: 			gcc -o chatclient chatclient.c -std=c99
Making server runnable: 	chmod +x chatserve.py

==============================================================================
Chat Server program
Starting 
1. login into flip server in 1st terminal window, ex: flip1.engr.oregonstate.edu
2. To run server program, enter following command: 
    ./chatserve.py portNumber   //port Number is integer > 1024
    Example: ./chatserve.py 12600   

Quitting 
a). To disconnect client, when it's server's turn to send message to client, 
enter "\quit" command 
b) To exit server, enter Ctrl-C keyboard shortcut

==============================================================================
Chat client program
Starting 
1. Login into flip server in 2nd terminal window, ex: flip1.engr.oregonstate.edu
2. To run server program, enter command:
    ./chatclient serverIP serverPort 
    Example: ./chatclient localhost 12600   

Quitting 
a). To disconnect client, when it's client's turn to send message to server, 
enter "\quit" command 

==============================================================================
Communicating between client/server
NOTE: After starting server, the client must be the first side to send message (to server)
1. Start server program first, then start client program next.
2. Client: enter client's handle
3. Client: enter data into terminal and enter key 
4. Server: enter data into terminal and enter key 
5. Repeat 3-4 until \quit command from Client/Server or closing server program






