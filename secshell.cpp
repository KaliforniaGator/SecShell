#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <csignal>
#include <fcntl.h>
#include <fstream>

class SecShell {
    std::unordered_map<pid_t, std::string> jobs;
    bool running = true;

    // Security whitelists
    const std::vector<std::string> ALLOWED_DIRS = {"/usr/bin/", "/bin/","/opt/"};
    const std::vector<std::string> ALLOWED_COMMANDS = {"ls", "ps", "netstat", "tcpdump","cd","clear","ifconfig"};
    
    // Blacklist of commands
    std::vector<std::string> BLACKLISTED_COMMANDS;
    
    // Function to load blacklisted commands from a file
    void load_blacklist(const std::string& filename);

    std::string repeat_string(const std::string& str, int n) {
        std::string result;
        for (int i = 0; i < n; ++i) {
            result += str;
        }
        return result;
    }

public:
    SecShell() {
        load_blacklist("blacklist.txt"); // Load blacklisted commands from file
    }

    void run() {
        // Register signal handlers
        signal(SIGINT, signal_handler);
        signal(SIGTSTP, signal_handler);

        while (running) {
            std::string input = get_input();
            process_command(input);
        }
    }

private:
    void display_prompt() {
        // Get current user
        const char* user = getenv("USER");
        if (!user) user = "unknown"; // Fallback if USER is not set

        // Get current working directory
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) == nullptr) {
            print_error("Failed to get current working directory");
            return;
        }

        // Print prompt with user and working directory
        std::cout << "\033[32m┌─[SecShell]\033[0m \033[1;34m(" << user << ")\033[0m \033[1;37m[" << cwd << "]\033[0;32m\n└─\033[0m$ ";
    }

    std::string get_input() {
        // Get current user
        const char* user = getenv("USER");
        if (!user) user = "unknown"; // Fallback if USER is not set

        // Get current working directory
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) == nullptr) {
            print_error("Failed to get current working directory");
            return "";
        }

        // Construct the prompt string
        std::string prompt = "\033[32m┌─[SecShell]\033[0m \033[1;34m(" + std::string(user) + ")\033[0m \033[1;37m[" + std::string(cwd) + "]\033[0;32m\n└─\033[0m$ ";

        // Read input using readline
        char* input = readline(prompt.c_str());
        if (input && *input) {
            add_history(input);
        }
        std::string sanitized = sanitize_input(input ? input : "");
        free(input);
        return sanitized;
    }

    std::string sanitize_input(const std::string& input) {
        std::string sanitized = input;
        const std::string forbidden = ";`";
        sanitized.erase(std::remove_if(sanitized.begin(), sanitized.end(),
            [&forbidden](char c) { return forbidden.find(c) != std::string::npos; }), sanitized.end());
        return sanitized;
    }

    void change_directory(const std::vector<std::string>& args) {
        std::string dir;

        if (args.size() < 2) {
            // If no directory is provided, default to the home directory
            const char* home = getenv("HOME");
            if (!home) {
                print_error("cd failed: HOME environment variable not set");
                return;
            }
            dir = home;
        } else {
            dir = args[1];
        }

        if (chdir(dir.c_str()) != 0) {
            print_error("cd failed: " + std::string(strerror(errno)));
        }
    }

    void display_history() {
		std::string drawbox_command = "drawbox \" Command History \" solid bg_blue bold_white";
        int result = system(drawbox_command.c_str());

        if (result != 0) {
            print_error("Failed to execute drawbox command.");
            return;
        }
        HIST_ENTRY** history_entries = history_list(); // Renamed variable to avoid conflict
        if (history_entries) {
            for (int i = 0; history_entries[i]; i++) {
                std::cout << "  " << (i + 1) << "  " << history_entries[i]->line << "\n";
            }
        } else {
            std::cout << "No command history available.\n";
        }
    }
    
    void export_variable(const std::vector<std::string>& args) {
		if (args.size() < 2) {
			print_error("Usage: export VAR=value");
			return;
		}

		std::string var_value = args[1];
		size_t equals_pos = var_value.find('=');
		if (equals_pos == std::string::npos) {
			print_error("Invalid export syntax. Use VAR=value");
			return;
		}

		std::string var = var_value.substr(0, equals_pos);
		std::string value = var_value.substr(equals_pos + 1);

		if (setenv(var.c_str(), value.c_str(), 1) != 0) {
			print_error("Failed to set environment variable: " + std::string(strerror(errno)));
		} else {
			print_alert("Exported: " + var + "=" + value);
		}
	}
	
	void list_env_variables() {
		extern char** environ;
		for (char** env = environ; *env; env++) {
			std::cout << *env << "\n";
		}
	}

	void unset_env_variable(const std::vector<std::string>& args) {
		if (args.size() < 2) {
			print_error("Usage: unset VAR");
			return;
		}
		if (unsetenv(args[1].c_str()) != 0) {
			print_error("Failed to unset environment variable: " + std::string(strerror(errno)));
		}
	}
	
	void reload_blacklist() {
        BLACKLISTED_COMMANDS.clear(); // Clear the existing blacklist
        load_blacklist("blacklist.txt"); // Reload the blacklist from the file
        print_alert("Blacklist reloaded.");
    }

	void process_command(const std::string& input) {
		std::string input_copy = input;
		if (input_copy.empty()) return;

		// Split commands by pipe
		std::vector<std::vector<std::string>> commands;
		size_t pos = 0;
		while (pos < input_copy.length()) {
			size_t pipe_pos = input_copy.find('|', pos);
			std::string command_str = input_copy.substr(pos, pipe_pos - pos);
			std::vector<std::string> args = parse_arguments(command_str);
			if (!args.empty()) {
				commands.push_back(args);
			}
			if (pipe_pos == std::string::npos) break;
			pos = pipe_pos + 1;
		}

		if (commands.size() > 1) {
			execute_piped_commands(commands);
		} else if (!commands.empty()) {
			std::vector<std::string> args = commands[0];
			bool background = false;
			if (args.back() == "&") {
				background = true;
				args.pop_back();
			}

			if (args[0] == "exit") {
				running = false;
			} else if (args[0] == "services") {
				manage_services(args);
			} else if (args[0] == "jobs") {
				list_jobs();
			} else if (args[0] == "help") {
				display_help();
			} else if (args[0] == "cd") {
				change_directory(args);
			} else if (args[0] == "history") {
				display_history();
			} else if (args[0] == "export") {
				export_variable(args);
			} else if (args[0] == "env") {
				list_env_variables();
			} else if (args[0] == "unset") {
				unset_env_variable(args);
			} else if (args[0] == "reload") { // Add the reload command
                reload_blacklist();
			} else {
				if (std::find(BLACKLISTED_COMMANDS.begin(), BLACKLISTED_COMMANDS.end(), args[0])
					!= BLACKLISTED_COMMANDS.end()) {
					print_error("Command is blacklisted: " + args[0]);
					return;
				}
				// Special handling for 'cat' without arguments
				if (args[0] == "cat" && args.size() == 1) {
					print_error("Usage: cat <file>");
					return;
				}
				execute_system_command(args, background);
			}
		}
	}

    void manage_services(const std::vector<std::string>& args) {
        if (args.size() < 2) {
            print_error("Usage: services <start|stop|restart|status|list> <service_name>");
            return;
        }

        std::string action = args[1];
        std::string service_name = args.size() > 2 ? args[2] : "";

        if (action != "start" && action != "stop" && action != "restart" && action != "status" && action != "list") {
            print_error("Invalid action. Use start, stop, restart, status, or list.");
            return;
        }

        std::string command;
        if (action == "list") {
            command = "systemctl list-units --type=service";
        } else if (action == "status") {
            command = "systemctl status " + service_name;
        } else {
            command = "sudo systemctl " + action + " " + service_name;
        }

        // Use drawbox for the title
        std::string drawbox_command = "drawbox \" Service Manager \" solid bg_blue bold_white";
        int result = system(drawbox_command.c_str());

        if (result != 0) {
            print_error("Failed to execute drawbox command.");
            return;
        }

        std::cout << "Executing: " << command << "\n";
        int status = system(command.c_str());

        if (status != 0) {
			print_error("Failed to execute service command.");
		} else {
			print_alert("Service command executed successfully.");
		}
    }
    
    void background_job_complete(pid_t pid) {
		std::string message = "Background job " + std::to_string(pid) + " completed.";
		print_alert(message);
	}

    void list_jobs() {
		std::string drawbox_command = "drawbox \" Jobs \" solid bg_blue bold_white";
        int result = system(drawbox_command.c_str());

        if (result != 0) {
            print_error("Failed to execute drawbox command.");
            return;
        }
        std::cout << "\033[33mActive Jobs:\033[0m\n";
        for (const auto& job : jobs) {
            std::cout << "PID: " << job.first << " - " << job.second << "\n";
        }
    }
    
    void execute_piped_commands(const std::vector<std::vector<std::string>>& commands) {
		int num_commands = commands.size();
		int pipes[num_commands - 1][2];

		// Create pipes
		for (int i = 0; i < num_commands - 1; ++i) {
			if (pipe(pipes[i]) == -1) {
				print_error("Failed to create pipe");
				return;
			}
		}

		for (int i = 0; i < num_commands; ++i) {
			pid_t pid = fork();
			if (pid == 0) { // Child process
				if (i > 0) {
					// Redirect stdin from the previous pipe
					dup2(pipes[i - 1][0], STDIN_FILENO);
				}
				if (i < num_commands - 1) {
					// Redirect stdout to the next pipe
					dup2(pipes[i][1], STDOUT_FILENO);
				}

				// Close all pipe ends
				for (int j = 0; j < num_commands - 1; ++j) {
					close(pipes[j][0]);
					close(pipes[j][1]);
				}

				// Modify grep commands to use --color=always
				std::vector<std::string> modified_args = commands[i];
				if (modified_args[0] == "grep") {
					// Check if --color=always is already present
					bool has_color_flag = false;
					for (const auto& arg : modified_args) {
						if (arg == "--color=always" || arg == "--color=auto") {
							has_color_flag = true;
							break;
						}
					}
					// Add --color=always if not already present
					if (!has_color_flag) {
						modified_args.push_back("--color=always");
					}
				}

				// Execute the command
				std::vector<char*> argv;
				for (const auto& arg : modified_args) {
					argv.push_back(const_cast<char*>(arg.c_str()));
				}
				argv.push_back(nullptr);

				if (execvp(argv[0], argv.data()) == -1) {
					print_error("Command execution failed: " + std::string(strerror(errno)));
					exit(EXIT_FAILURE);
				}
			} else if (pid > 0) { // Parent process
				if (i > 0) {
					close(pipes[i - 1][0]);
					close(pipes[i - 1][1]);
				}
			} else {
				print_error("Fork failed: " + std::string(strerror(errno)));
				return;
			}
		}

		// Wait for all child processes to finish
		for (int i = 0; i < num_commands; ++i) {
			wait(NULL);
		}
	}

	void execute_system_command(const std::vector<std::string>& args, bool background = false) {
		if (!is_command_allowed(args[0])) {
			print_error("Command not permitted: " + args[0]);
			return;
		}

		std::vector<std::string> modified_args = args;
		int input_fd = -1, output_fd = -1;

		// Handle input redirection (<)
		auto input_redirect_pos = std::find(modified_args.begin(), modified_args.end(), "<");
		if (input_redirect_pos != modified_args.end()) {
			if (input_redirect_pos + 1 == modified_args.end()) {
				print_error("Syntax error: No input file specified for redirection.");
				return;
			}
			std::string input_file = *(input_redirect_pos + 1);
			input_fd = open(input_file.c_str(), O_RDONLY);
			if (input_fd == -1) {
				print_error("Failed to open input file: " + input_file);
				return;
			}
			// Remove the redirection tokens from args
			modified_args.erase(input_redirect_pos, input_redirect_pos + 2);
		}

		// Handle output redirection (> or >>)
		auto output_redirect_pos = std::find(modified_args.begin(), modified_args.end(), ">");
		auto append_redirect_pos = std::find(modified_args.begin(), modified_args.end(), ">>");
		if (output_redirect_pos != modified_args.end() || append_redirect_pos != modified_args.end()) {
			auto redirect_pos = (output_redirect_pos != modified_args.end()) ? output_redirect_pos : append_redirect_pos;
			if (redirect_pos + 1 == modified_args.end()) {
				print_error("Syntax error: No output file specified for redirection.");
				if (input_fd != -1) close(input_fd);
				return;
			}
			std::string output_file = *(redirect_pos + 1);
			int flags = (redirect_pos == output_redirect_pos) ? (O_WRONLY | O_CREAT | O_TRUNC) : (O_WRONLY | O_CREAT | O_APPEND);
			output_fd = open(output_file.c_str(), flags, 0644);
			if (output_fd == -1) {
				print_error("Failed to open output file: " + output_file);
				if (input_fd != -1) close(input_fd);
				return;
			}
			// Remove the redirection tokens from args
			modified_args.erase(redirect_pos, redirect_pos + 2);
		}

		// Add --color=always for grep
		if (modified_args[0] == "grep") {
			bool has_color_flag = false;
			for (const auto& arg : modified_args) {
				if (arg == "--color=always" || arg == "--color=auto") {
					has_color_flag = true;
					break;
				}
			}
			if (!has_color_flag) {
				modified_args.push_back("--color=always");
			}
		}

		// Add --color=auto for ls
		if (modified_args[0] == "ls") {
			modified_args.push_back("--color=auto");
		}

		pid_t pid = fork();
		if (pid == 0) { // Child process
			// Redirect input if necessary
			if (input_fd != -1) {
				dup2(input_fd, STDIN_FILENO);
				close(input_fd);
			}

			// Redirect output if necessary
			if (output_fd != -1) {
				dup2(output_fd, STDOUT_FILENO);
				close(output_fd);
			}

			// Prepare arguments for execvp
			std::vector<char*> argv;
			for (const auto& arg : modified_args) {
				argv.push_back(const_cast<char*>(arg.c_str()));
			}
			argv.push_back(nullptr);

			// Execute the command
			if (execvp(argv[0], argv.data()) == -1) {
				print_error("Command execution failed: " + std::string(strerror(errno)));
				exit(EXIT_FAILURE);
			}
		} else if (pid > 0) { // Parent process
			// Close file descriptors in the parent process
			if (input_fd != -1) close(input_fd);
			if (output_fd != -1) close(output_fd);

			// Track the job
			jobs[pid] = args[0];
			if (!background) {
				int status;
				waitpid(pid, &status, 0); // Wait for the child process to finish
				if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
					print_error("Command exited with status: " + std::to_string(WEXITSTATUS(status)));
				}
				jobs.erase(pid); // Remove the job from tracking
			} else {
				std::string message = "[" + std::to_string(pid) + "] " + args[0] + " running in background";
				print_alert(message);
			}
		} else {
			print_error("Fork failed: " + std::string(strerror(errno)));
		}
	}

	void display_help() {
		// Use the drawbox binary to draw the title box
		std::string drawbox_command = "drawbox \" SecShell Help \" bold_white";
		int result = system(drawbox_command.c_str());

		if (result != 0) {
			print_error("Failed to execute drawbox command.");
			return;
		}

		std::cout <<
			"\n\033[36mBuilt-in Commands:\033[0m\n"
			"  \033[1mhelp\033[0m       - Show this help message\n"
			"  \033[1mexit\033[0m       - Exit the shell\n"
			"  \033[1mdrawbox\033[0m    - Create a text box\n"
			"               Usage: drawbox \"Your text here\" [solid] [bg_color] [text_color]\n"
			"  \033[1mservices\033[0m   - Manage system services\n"
			"               Usage: services <start|stop|restart|status|list> <service_name>\n"
			"  \033[1mjobs\033[0m      - List active background jobs\n"
			"  \033[1mcd\033[0m        - Change directory\n"
			"               Usage: cd [directory]\n"
			"  \033[1mhistory\033[0m    - Show command history\n"
			"  \033[1mexport\033[0m     - Set an environment variable\n"
			"               Usage: export VAR=value\n"
			"  \033[1menv\033[0m        - List all environment variables\n"
			"  \033[1munset\033[0m      - Unset an environment variable\n"
			"               Usage: unset VAR\n"
			"  \033[1mreload\033[0m     - Reload the blacklist of commands\n" // Add the reload command
			"\n\033[36mAllowed System Commands:\033[0m\n";

		// Display whitelisted commands
		for (const auto& cmd : ALLOWED_COMMANDS) {
			std::cout << "  - " << cmd << "\n";
		}

		std::cout << "\n\033[36mSecurity Features:\033[0m\n"
			"  - Command whitelisting\n"
			"  - Input sanitization\n"
			"  - Process isolation\n"
			"  - Job tracking\n"
			"  - Background job execution\n"
			"  - Piped command execution\n"
			"  - Input/output redirection\n"
			"\n\033[36mExamples:\033[0m\n"
			"  > drawbox \"Security Alert\" solid green white\n"
			"  > ls -l\n"
			"  > jobs\n"
			"  > services list\n"
			"  > export MY_VAR=value\n"
			"  > env\n"
			"  > unset MY_VAR\n"
			"  > history\n"
			"\n\033[36mNote:\033[0m\n"
			"All commands are subject to security checks and sanitization.\n"
			"Only executables from trusted directories are permitted.\n\033[0m";
	}

    bool is_command_allowed(const std::string& cmd) {
        // Check if the command is blacklisted
        if (std::find(BLACKLISTED_COMMANDS.begin(), BLACKLISTED_COMMANDS.end(), cmd)
            != BLACKLISTED_COMMANDS.end()) {
            return false; // Command is blacklisted
        }

        // If not blacklisted, check if it's in the allowed directories
        for (const auto& dir : ALLOWED_DIRS) {
            std::string path = dir + cmd;
            if (access(path.c_str(), X_OK) == 0) {
                return true;
            }
        }
        return false;
    }

    void print_alert(const std::string& message) {
		std::string drawbox_command = "drawbox \"" + message + "\" solid bg_yellow bold_white";
		int result = system(drawbox_command.c_str());
		if (result != 0) {
			std::cerr << "\033[33m[ALERT] " << message << "\033[0m\n"; // Fallback if drawbox fails
		}
	}

	void print_error(const std::string& message) {
		std::string drawbox_command = "drawbox \"" + message + "\" solid bg_red bold_white";
		int result = system(drawbox_command.c_str());
		if (result != 0) {
			std::cerr << "\033[31m[ERROR] " << message << "\033[0m\n"; // Fallback if drawbox fails
		}
	}

    std::vector<std::string> parse_arguments(const std::string& input) {
		std::vector<std::string> args;
		std::string arg;
		bool in_quotes = false;
		char quote_char = '\0';
		bool escape = false;

		for (size_t i = 0; i < input.length(); ++i) {
			char c = input[i];

			if (escape) {
				arg += c;
				escape = false;
				continue;
			}

			if (c == '\\') {
				escape = true;
				continue;
			}

			if (c == '"' || c == '\'') {
				if (in_quotes && c == quote_char) {
					in_quotes = false;
					quote_char = '\0';
				} else if (!in_quotes) {
					in_quotes = true;
					quote_char = c;
				} else {
					arg += c;
				}
			} else if (c == ' ' && !in_quotes) {
				if (!arg.empty()) {
					args.push_back(arg);
					arg.clear();
				}
			} else {
				arg += c;
			}
		}

		if (!arg.empty()) {
			args.push_back(arg);
		}

		return args;
	}

    static void signal_handler(int signum) {
        std::cout << "\nReceived signal " << signum << ". Use 'exit' to quit.\n";
    }
};

// Define the load_blacklist function outside the class
void SecShell::load_blacklist(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        print_error("Failed to open blacklist file: " + filename);
        return;
    }

    std::string command;
    while (std::getline(file, command)) {
        // Remove any leading/trailing whitespace from the command
        command.erase(0, command.find_first_not_of(" \t\n\r\f\v"));
        command.erase(command.find_last_not_of(" \t\n\r\f\v") + 1);

        if (!command.empty()) {
            BLACKLISTED_COMMANDS.push_back(command);
        }
    }
}

int main() {
    SecShell shell;
    shell.run();
    return 0;
}
