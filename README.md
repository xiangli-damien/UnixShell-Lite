# msh - Lite Shell in C



## Introduction

**msh** is a lightweight Linux shell implemented in C, designed to simulate essential shell functionality and provide fundamental command execution and job control features. This shell can parse user input, fork child processes to execute commands, and handle interruptions using signals. It also maintains a command history, allowing users to recall and re-execute previous commands.

## Features

- **Command Parsing and Execution**: Parses user input, forks child processes, and executes commands using `execve`.
- **Signal Handling**: Supports keyboard interruption (`^C`) and job suspension (`^Z`) for foreground jobs.
- **Command History**: Preserves user command history in a designated file for easy recall.
- **Job Control**: Allows background and foreground job management, supporting commands like `jobs`, `bg`, and `fg`.
- **File Redirection**: Reads commands from files for automated execution.

## Getting Started

### 1. Build the msh Program

Navigate to the root directory of the project and build `msh` using the provided script:

```bash
./scripts/build.sh
```

### 2. Running msh

Once built, you can start `msh` with default settings by simply typing:

```bash
./msh
```

### 3. Using msh

Below are examples of various commands and features within `msh`.

#### 3.1 Basic Command Execution

```bash
msh> /usr/bin/echo "Hello, World!"
Hello, World!
```

#### 3.2 Command History

The `history` command displays previously executed commands. The command history is saved in the `./data/.msh_history` file, so history is preserved across sessions.

```bash
msh> history
1   /usr/bin/ls -la
2   /usr/bin/echo "Hello"
3   history
```

Users can also re-execute a specific command by using the `!N` syntax, where `N` is the line number in the history list.

```bash
msh> !2
Hello
```

#### 3.3 Customizing msh with Optional Arguments

You can start `msh` with optional settings using flags:

- `-s NUMBER`: Maximum number of commands to store in history.
- `-j NUMBER`: Maximum number of active jobs.
- `-l NUMBER`: Maximum command length.

Example:

```bash
./msh -s 100 -j 10 -l 80
```

### 4. Advanced Usage

#### 4.1 File Redirection

You can use `msh` with file input to execute commands from a file.

Example file (`test001.in`):

```bash
/usr/bin/seq 1 10
/usr/bin/sleep 8 &
/usr/bin/seq 1 5
```

To execute:

```bash
./msh < ./data/test001.in
```

#### 4.2 Job Control Commands

**jobs** - Lists all active jobs:

```Bash
msh> jobs
[1] 1234 RUNNING   /usr/bin/sleep 10
[2] 1235 SUSPENDED /usr/bin/sleep 20
```

**bg [PID|%JID]** - Resumes a suspended job in the background:

```bash
msh> bg %1
```

**fg [PID|%JID]** - Brings a background job to the foreground:

```bash
msh> fg %2
```

#### 4.3 Signal Handling

- **^C** - Sends `SIGINT` to the current foreground job, terminating it.
- **^Z** - Sends `SIGTSTP` to suspend the current foreground job.

### 5. Built-in Commands

- **jobs**: Displays a list of all active jobs.

- **history**: Shows command history.

- **!N**: Re-executes a command from the history, where `N` is the history line number.

- kill [SIG_NUM] [PID|%JID]

  : Sends a signal to terminate or manage a job.

  - Valid signals: `SIGINT` (2), `SIGKILL` (9), `SIGCONT` (18), and `SIGSTOP` (19).

Example:

```bash
msh> kill 9 %1
```

### 6. Exiting msh

To exit the shell, type:

```bash
msh> exit
```

## Project Structure

- **src/**: Contains the source code for `msh`.
- **scripts/**: Contains build scripts.
- **data/**: Directory for storing history files and input files for redirection.