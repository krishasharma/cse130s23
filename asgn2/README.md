#CSE 130 Asssignment 2: HTTP Server 
##Professor Veenstra, Spring 2023, UCSC

###Goal: 
This assignment will provide you with expierence building a system that uses client-server/strong modularity. 

###Learing Objectives: 
>(1) practice implementing a client-server system
>(2) practice the advantages of powerful abstractions 
>(3) practice implementing a large system that solves a large problem 
>(4) review string parsing 
>(5) review memory management

###Assignment Details: 
You will be building an HTTP server for this assignment. 
Your server should execute "forever" without crashing (i.e., it should run until the user types CTRL-C on the terminal). 
Your server will create, listen, and accept connections from clients that arrive on a port. 
A key task in this assignment is buiding a server that is resilient to malformed and malicious clients: no matter what a client sends your server, your server should not crash.

Running:
Your server should take in a single-command line arrgument, an int, named port. In other words, your server should be started by specifying the following command: ./httpserver <port> 


