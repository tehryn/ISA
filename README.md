#popser(1)

##NAME

popser - POP3 server

##SYNOPSIS

**popser** [-h] [-a AUTH_PATH] [-c] [-p PORT] [-d MAILDIR_PATH] [-r]


##DESCRIPTION
popser is an implementation of POP3 server and can be used for communication
with clients.

##OPTIONS

###-h
Prints help on stdoud

###-a AUTH_PATH
Path to file where is username and password stored

###-c
If set, there will be no password encryption expected and used.
User must use commands USER and PASS for authorization. If not
set, user must use APOP command for authorization.

###-p PORT
Specifies port number that will be used.

###-d MAILDIR_PATH
Specifies the path to Maildir. Directories cur and new are expected to be there.

###-r
Undo all changes (deleted files are not recovered).

##EXAMPLE 1
popser -h

Prints help on stdout.
##EXAMPLE 2
popser -r

Undo all changes.
##EXAMPLE 3
popser -a ./user.txt -p 8888 -d ./Maildir/ -c

Start server on port 8888, loads user info from ./user.txt and works with directories ./Maildir/new and ./Maildir/cur.
After the server recive SIGINT or SIGTERM, he will try to clean after himself (rename back all files, he renamed during its proccess)

##AUTHOR
Written by Matějka Jiří, xmatej52@stud.fit.vutbr.cz
