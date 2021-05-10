# hanin.6


I used a combination of message queues and shared memory for requests, releases, and communication between processes.

I track remaining resources and what resources each process currently has available to them with shared memory, and use message queues to communicate back and forth.

Outstanding problem is replicability.
There exists a logic bug or two that cause the program to fail and not start reading and writing from processes, they get stuck in an endless loop until they terminate individually when enough time has passed.

On occassions where they go through, output seems correc,t reading and writing and remaining tables are correct, as does the deadlock algorithm work.

