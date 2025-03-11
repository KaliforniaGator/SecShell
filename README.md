# SecShell - A Secure Shell Implementation

## Overview

SecShell is a secure shell implementation designed to provide a controlled environment for executing system commands. It incorporates various security features such as command whitelisting, input sanitization, process isolation, and job tracking. The shell is intended to be used in environments where security and control over command execution are paramount.

## Features

- **Command Whitelisting**: Only commands from specified directories (`/usr/bin/`, `/bin/`, `/opt/`) are allowed to execute.
- **Input Sanitization**: Input is sanitized to remove potentially harmful characters.
- **Process Isolation**: Commands are executed in isolated processes to prevent interference.
- **Job Tracking**: Background jobs are tracked and can be listed using the `jobs` command.
- **Services Manager**: Start, Stop, and check the Status of your services in one convenient place. Use the `services start | stop | list | status` command.
- **Background Job Execution**: Commands can be executed in the background by appending `&` to the command.
- **Piped Command Execution**: Supports piping commands together (e.g., `ls | grep .txt`).
- **Input/Output Redirection**: Supports input and output redirection (e.g., `ls > output.txt`).
- **Built-in Commands**: Includes commands like `cd`, `history`, `export`, `env`, `unset`,`blacklist`,`edit-blacklist`, and more.

- **Admin-Control**: All the blacklisted commands go in the .blacklist file. Write each command in its own line. Then use ```bash sudo chown (root|admin|sudo) .blacklist ``` to prevent a unprivileged user from editing this file.
  
- You can also blacklist commands like edit-blacklist to make sure the file stays uneditable to the user. Blacklisting expands to commands such as : `exit`, `shutdown`,`reboot`, or any other command you do not want the user to run. **WARNING:** If `exit` is blacklisted you WILL NOT be able to exit the SecShell, so use the blacklistings wisely.

## Installation

### Prerequisites

- **Linux**: SecShell is designed to run on Linux systems. It does **not** work on macOS.
- **C++ Compiler**: A C++ compiler that supports C++11 or later.
- **GNU Readline Library**: Required for command line input handling.
- **DrawBox**: Required for GUI Element rendering inside the CLI. [DrawBox](https://github.com/KaliforniaGator/Drawbox)

### More Tools
KaliforniaGator has other tools you might like to use with SecShell:
  
- **TempConv**: A CLI temperature converter. [TempConv](https://github.com/KaliforniaGator/TempConv)
- **XTool**: Just type `x` to clear the console. [XTool](https://github.com/KaliforniaGator/XTool)
- **EasyMap**: A simple NMAP command wizzard written in bash. [EasyMap](https://github.com/KaliforniaGator/EasyMap)
- **VPNSel-ng**: An easy to use CLI for OpenVPN configuration management, selection, and usage. [VPNSel-ng](https://github.com/KaliforniaGator/VPNSel)

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
## Installation

1. Make sure you have DrawBox installed in your `/bin/` directory as `drawbox` lowercase. Otherwise SecShell might show you an error anytime DrawBox is used.

2. After compiling SecShell as `secshell` mv it to your desired directory using
   ```bash
   sudo mv secshell /DIRECTORY/
   ```
3. Make sure to creat the `.blacklist` file in the directory you are installing secshell.

   You can do this by typing
   ```bash
   sudo touch DIRECTORY/.blacklist
   ```
   then use your editor of choice (nano|vim|emacs) to add blacklisted commands. 

## Usage

###Running SecShell

Once compiled, you can run SecShell by executing the `./secshell` binary. The shell will start and display a prompt where you can enter commands.

**Or**

Copy the secshell executable and the .blacklist file to your `/bin/`, `/usr/bin/`, or `/opt/` directory.
Then you will be able to run SecShell directly from anywhere.

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
- **blacklist**: Lists all blacklisted commands.
- **edit-blacklist**: Edit the .blacklist file. (All the blacklisted commands are stored here)

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
