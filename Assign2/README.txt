CMPUT 379 Winter 2017 - Assignment 1

Tianyu Zhang & Di Zhang

The program can simulate the simple client-server program.

Compile them by type "$make all".

Run server by "$./wbs379 2222 -f in".

Run client by "$./wbc379 127.0.0.1 2222 key".

The "in" file is a sample statefile. The format
of our statefile is "[type] [entry]".

The server program can respond the incoming request and display
on the server side also. Good for debug. Message format will be
"Request: !@#$% Respond:@#$% [length of response]".

The format for key file is "salt=@#$%\nkey=@#$%^\niv =#$%^".

Our default hostname is "127.0.0.1".
