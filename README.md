# SecShell - A Secure Shell Implementation

## Overview

SecShell is a secure shell implementation designed to provide a controlled environment for executing system commands. It incorporates various security features such as command whitelisting, input sanitization, process isolation, and job tracking. The shell is intended to be used in environments where security and control over command execution are paramount.

## Features

- **Command Whitelisting**: Only commands from specified directories (`/usr/bin/`, `/bin/`, `/opt/`) are allowed to execute.
- **Input Sanitization**: Input is sanitized to remove potentially harmful characters.
- **Process Isolation**: Commands are executed in isolated processes to prevent interference.
- **Job Tracking**: Background jobs are tracked and can be listed using the `jobs` command.
- **Background Job Execution**: Commands can be executed in the background by appending `&` to the command.
- **Piped Command Execution**: Supports piping commands together (e.g., `ls | grep .txt`).
- **Input/Output Redirection**: Supports input and output redirection (e.g., `ls > output.txt`).
- **Built-in Commands**: Includes commands like `cd`, `history`, `export`, `env`, `unset`, and more.

## Installation

### Prerequisites

- **Linux**: SecShell is designed to run on Linux systems. It does **not** work on macOS.
- **C++ Compiler**: A C++ compiler that supports C++11 or later.
- **GNU Readline Library**: Required for command line input handling.

### Building SecShell

1. Clone the repository:
   ```bash
   git clone https://github.com/KaliforniaGator/SecShell.git
   cd SecShell
   ```

2. Compile the program:
   ```bash
   g++ -o secshell secshell.cpp -lreadline
   ```

3. Run the shell:
   ```bash
   ./secshell
   ```

## Usage

### Running SecShell

Once compiled, you can run SecShell by executing the `./secshell` binary. The shell will start and display a prompt where you can enter commands.

### Built-in Commands

- **help**: Display a help message with available commands and usage.
- **exit**: Exit the shell.
- **drawbox**: Create a text box with specified text and styles.
- **services**: Manage system services (start, stop, restart, status, list).
- **jobs**: List active background jobs.
- **cd**: Change the current directory.
- **history**: Show command history.
- **export**: Set an environment variable.
- **env**: List all environment variables.
- **unset**: Unset an environment variable.
- **reload**: Reload the blacklist of commands.

### Example Commands

- Change directory:
  ```bash
  cd /path/to/directory
  ```

- List files in the current directory:
  ```bash
  ls
  ```

- Run a command in the background:
  ```bash
  sleep 10 &
  ```

- List active background jobs:
  ```bash
  jobs
  ```

- Pipe commands:
  ```bash
  ls | grep .txt
  ```

- Redirect output to a file:
  ```bash
  ls > output.txt
  ```

- Manage services:
  ```bash
  services start apache2
  ```

## Security Features

- **Command Whitelisting**: Only commands from trusted directories are allowed.
- **Input Sanitization**: Removes potentially harmful characters from user input.
- **Process Isolation**: Commands are executed in isolated processes to prevent interference.
- **Job Tracking**: Background jobs are tracked and can be listed using the `jobs` command.

## License

SecShell is released under the **GNU Affero General Public License (AGPL) version 3.0**. This means that you are free to use, modify, and distribute the software, but any modifications or derivative works must also be licensed under the AGPL and made available to the public.

For more details, see the [LICENSE](LICENSE) file.

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request with your changes. Ensure that your code adheres to the existing style and includes appropriate tests.

## Issues

If you encounter any issues or have suggestions for improvements, please open an issue on the [GitHub repository](https://github.com/yourusername/SecShell/issues).

## Disclaimer

SecShell is provided "as is" without any warranties. The authors are not responsible for any damage or data loss caused by the use of this software. Use at your own risk.

---

Enjoy using SecShell! If you have any questions or need further assistance, feel free to reach out.
