# IOCP_Bulls-Cows
Number baseball game based on IOCP 

Multiple connections of clients are possible, and the server and client are in a 1: N structure.

This is a routine where the game ends when a random number is created on the server and the client matches the number on the server in order.

## Manual
First, start the server, then the clients, and the client supports multiple clients.

And if the client inputs three digits in succession without duplication and hits enter,
the server processes the strike and ball value and sends it to the client. 


Repeat until you hit the number of servers and if the number of servers is corrected, the corresponding client that was hit wins and ends.
