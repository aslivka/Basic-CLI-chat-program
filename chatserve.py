#!/usr/bin/python3

#******************************************************************
#**  Author: Artem Slivka
#**  Date: 10/25/18
#**  Description: Project1 - chatserve.py
#**  Implementation of server chat program using sockets.
#**  To run: python3 chatserver.py <server port#>
#******************************************************************/

import sys, errno
import signal
import time
from socket import *

# Function for SIGINT signal handler
# It closes socket and exits program on CTRL-C interrupt.
def sigintHandler(signum, frame):
    #Closing socket first
    if 'clientSocket' in globals():
        exitServer(clientSocket)
    print("Exiting after receiving CTRL+C!")  
    # Finally exiting program 
    sys.exit()

#Function checks passed program arguments for right number and port range
def checkServerArgs():
    if len(sys.argv) != 2:
        print("usage: python3 chatServer.py port")
        return False
    elif int(sys.argv[1]) <= 1024:
        print ("Enter port number > 1024")
        return False
    return True

# Main function that starts server. It blocks at line:
#     clientSocket, addr = serverSocket.accept()
# until server connects successfully to client
def startServer(port):
    global serverSocket
    global clientSocket
    printServer("Starting chat server on port: " + str(port))
    serverSocket = createTCPSocket()
    serverSocket.bind(('', port))
    serverSocket.listen(1)
    printServer("The server is ready to receive")
    clientSocket, addr = serverSocket.accept()
    startSession(clientSocket)

# Function creates new server session with client socket on the other side
# It continues running in the while loop until SIGINT(Ctrl-C) is received on server side
# Reference: Lecture 15	Socket Programming Primer
def startSession(socket):
    global clientSocket
    global serverHandle
    clientSocket = socket
    while True:
        try:
            # Waiting to receive new message from client, blocking call
            result = receiveMessage(clientSocket)
            # Found quit string from client side, closing client socket
            # and waiting for new client connection
            if result == -1:
                waitForNewClient()
                continue     
            # Waiting for user input to send to client, blocking call                        
            print(serverHandle + "> ", end="") 
            txMessage = input()
            #Sending message to client
            result = sendMessage(txMessage, clientSocket)
            # Found quit string from server side, closing client socket
            # and waiting for new client connection
            if result == -1:
                waitForNewClient()   
        # If client shut down unexpectedly, listen for new client connection
        except IOError as e:
            if e.errno == errno.EPIPE:
                waitForNewClient()

# Sends message to client
def sendMessage(message, socket):
    global serverHandle   
    message = message.rstrip('\r\n')
    socket.send((serverHandle + "> " + message).encode())
    foundQuit = message.find("\quit")
    if foundQuit != -1:
        return -1
    return 0

# Receives message from client
def receiveMessage(socket):
    rxMessage = socket.recv(1024).decode()
    if len(rxMessage) > 0:
        print(rxMessage)
        foundQuit = rxMessage.find("\quit")
        if foundQuit != -1:
            return -1
    return 0

# Prints message on server terminal
def printServer(message):
    print(serverHandle + "> " + message)

# Function waits to accept new client socket connection from other side.
# Blocks, until new socket connection is established
def waitForNewClient(): 
    global serverSocket
    global clientSocket
    printServer("Closing client connection!")
    clientSocket.close()
    clientSocket, addr = serverSocket.accept()        

# Function closes server socket
def exitServer(socket):
    printServer("Closing server connection!")
    socket.close()          

# Function creates new server socket 
def createTCPSocket():
    newSocket = socket(AF_INET, SOCK_STREAM)
    newSocket.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)  #Reuse socket without waiting for it to naturally time out
    return newSocket

# Program entry point
signal.signal(signal.SIGINT, sigintHandler)
global serverSocket
global clientSocket
global serverHandle
serverHandle = "Samuel"

validArgs = checkServerArgs()
if validArgs == True:
    serverPort = int(sys.argv[1])
    # Number of arguments is correct, starting server
    startServer(serverPort)




