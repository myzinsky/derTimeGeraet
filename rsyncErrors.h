#include<QString>

const QString rsyncErrors[] = {
    "Success", // 0
    "Syntax or usage error", // 1
    "Protocol incompatibility", // 2
    "Errors selecting input/output files, dirs", // 3
    "Requested  action not supported: an attempt \
     was made to manipulate 64-bit files on a platform \
    that cannot support them; or an option was specified \
    that is supported by the client and not by the server.",  // 4
    "Error starting client-server protocol",  // 5
    "Daemon unable to append to log-file", // 6
    "N/A", // 7
    "N/A", // 8
    "N/A", // 9
    "Error in socket I/O", // 10
    "Error in file I/O", // 11
    "Error in rsync protocol data stream", // 12
    "Errors with program diagnostics", // 13
    "Error in IPC code", // 14
    "N/A", // 15
    "N/A", // 16
    "N/A", // 17
    "N/A", // 18
    "N/A", // 19
    "Received SIGUSR1 or SIGINT", // 20
    "Some error returned by waitpid()", // 21
    "Error allocating core memory buffers", // 22
    "Partial transfer due to error", // 23
    "Partial transfer due to vanished source files", // 24
    "The --max-delete limit stopped deletions", //25
    "N/A", // 26
    "N/A", // 27
    "N/A", // 28
    "N/A", // 29
    "Timeout in data send/receive", // 30
    "N/A", // 31
    "N/A", // 32
    "N/A", // 33
    "N/A", // 34
    "Timeout waiting for daemon connection" // 35
};
