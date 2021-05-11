# hanin.6


I used a combination of message queues and shared memory for requests, releases, and communication between processes.

I track remaining resources and what resources each process currently has available to them with shared memory, and use message queues to communicate back and forth.

Outstanding problem:

Replicability.
There exists a logic bug or two that cause the program to fail and not start reading and writing from processes, they get stuck in an endless loop until they terminate individually when enough time has passed.

On occassions where they go through, output seems correct reading and writing and remaining tables are correct, as does the deadlock algorithm work.

What occurs is that sometimes, I send a message to a process to block it, and sometimes it will get stuck at that point for eternity. I have tried many different things to fix this, and have been unable to solve it at the point of project due date.

You may have to run the program a few times to get past this, if the log file looks empty, or with not a lot of information, it may take a few runs before whatever is breaking doesn't break, and then you'll see the full implementation of all the data and messagging. I apologize for this error, but I've been pulling my hair out about it for ages at this point, and have no idea what to do.

Thank you for taking the time to read this, if you did so, I appreciate it, and all you've done this semester. Thank you!

Rami Hanin
