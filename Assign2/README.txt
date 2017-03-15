CMPUT 379 Winter 2017 - Assignment 1

Tianyu Zhang & Di Zhang

The program can simulate the simple client-server program.

Compile them by type "$make all".

Run server by "$./wbs379 2222 -f statefile".

Run client by "$./wbc379 127.0.0.1 2222 keyfile".

The "in" file is a sample statefile. The format will be the following:
+==============================================================================+
!1p5
hello
!2p6
the
cs
!2c64
nk6gPCjxUArSlO/FvvNwV4Bum/STec4kYNBce1e322lRmI5afEkMn/7lfil5Fd+O
+==============================================================================+

The server program can respond the incoming request and display
on the server side also. Good for debug. Message format will be

+==============================================================================+
Request: !@#$
Respond:@#$%
[length of response]
+==============================================================================+

The client side is well introduced in the interface. Please follow the guide
displayed on the screen. Type in the command you want.

When server is on, the daemon server will strat working for all client.
As you can see the server will tell you which pid it is and it will work behind
the terminal and not reachable unless use terminal and input:

$ kill [pid]

When kill (a.k.a. SIGTERM) detected, the sever will save all the works into a
file called whiteboard.all. That will follow the format of statefile.

The format for the keyfile is the following:
+==============================================================================+
salt=9240EF9C494B3E1E
key=ACBA25B78AB63B278D536A7C44F7947EEAE26036314B38AE
iv =B882970A832EE79F4DD59A0E3CF8F4C6
salt=263BC60258FF4876
key=B374A26A71490437AA024E4FADD5B497FDFF1A8EA6FF12F6FB65AF2720B59CCF
iv =7E892875A52C59A3B588306B13C31FBD
salt=80EE5A332F8E7D0C
key=71AF2A705E3CD53B7100FFA0339F8DF818677B8B6D12B7CE
iv =974780E492065369A8FDC7E815929602
+==============================================================================+

Each three line indecates a unique AES-256-CBC key. The client will go through
all the key in the keyfile and try to decrypt the message.

Once the message has been decrypted, it will check if decrypted corectly by
looking through the message make sense or not by check the specific token
"CMPUT379 Whiteboard Encrypted v0\n". If yes, print the message.

Our default hostname is "127.0.0.1".
