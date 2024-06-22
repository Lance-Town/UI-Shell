# University of Idaho Shell (uish)

## Overview

`uish` (University of Idaho Shell) is a simple Unix/Linux shell implementation designed to mimic basic functionalities of the bash shell. It supports executing standard commands, managing shell variables, handling the `PATH` environment variable, and running commands in the background.

## Features

- **Command Execution**: 
  - Supports execution of any Unix/Linux command with options.
  - Uses `fork` and `execv` for command execution.
  
- **Shell Variables**: 
  - Allows defining and using shell variables.
  - Syntax: `VAR=value` and `echo $VAR`.
  
- **PATH Handling**: 
  - Processes the `PATH` variable to locate executables.
  - Uses `execv()` to execute commands found in the `PATH`.

- **Prompt**: 
  - Displays a prompt `$` to accept user commands.
  
- **Background Execution**: 
  - Supports running commands in the background using `&`.

## Usage

### Running uish

To start the shell, compile and run the `uish` program:

```sh
gcc -o uish uish.c
./uish
```

### Commands 
You can enter any standard Unix/Linux command, including options, and `uish` will execute it:
```uish
$ ls -l
$ grep "pattern" file.txt
$ ps aux
```

### Shell Variables 

Define and use shell variables in `uish`:
```uish
$ ABC=xyz
$ echo $ABC
xyz
```

### PATH

The `PATH` variable works similar to other shells. Define it to specify directories to search for executables:
```uish
$ PATH=/usr/local/bin:/usr/bin:/bin
$ somecommand
```

### Background Execution
Run commands in the background by appending `&`:
```uish
$ sleep 10 &
$ [prompt returns immediately]
```

## Implementation Details

- **Command Parsing**: 
  - The shell parses user input to indetify commands and options.
  - Supports variable substitutoin and background execution syntax

- **Fork and Exec**: 
  - Uses `fork()` to create a new process and `execv()` to execute commands.
  - Manages `PATH` variable manually to locate executables.

- **Prompt Loop**: 
  - Continuously displays a prompt, accepts input, and executes commands.
  - Waits for foreground commands to complete before displaying the prompt again.


