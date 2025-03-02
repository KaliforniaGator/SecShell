#!/bin/bash

# GitHub repository details
REPO_OWNER="KaliforniaGator"
REPO_NAME="SecShell"
FILE_PATH="secshell.cpp"

# GitHub API URL to fetch the file content
GITHUB_API_URL="https://api.github.com/repos/$REPO_OWNER/$REPO_NAME/contents/$FILE_PATH"

# Function to download using curl
download_with_curl() {
    curl -H "Accept: application/vnd.github.v3.raw" -o secshell.cpp "$GITHUB_API_URL"
}

# Function to download using wget
download_with_wget() {
    wget --header="Accept: application/vnd.github.v3.raw" -O secshell.cpp "$GITHUB_API_URL"
}

# Try to download using curl, fallback to wget if curl is not available
echo "Downloading the latest version of secshell.cpp..."
if command -v curl &> /dev/null; then
    download_with_curl
elif command -v wget &> /dev/null; then
    download_with_wget
else
    echo "Neither curl nor wget is installed. Please install one of them to proceed."
    exit 1
fi

# Check if the download was successful
if [ $? -ne 0 ]; then
    echo "Failed to download secshell.cpp. Please check your internet connection and the URL."
    exit 1
fi

# Compile secshell.cpp using g++ or clang
echo "Compiling secshell.cpp..."

# Check if g++ is available
if command -v g++ &> /dev/null; then
    COMPILER="g++"
# Check if clang is available
elif command -v clang++ &> /dev/null; then
    COMPILER="clang++"
else
    echo "Neither g++ nor clang++ found. Please install one of them to compile secshell.cpp."
    exit 1
fi

# Compile the file
$COMPILER secshell.cpp -o secshell -lreadline

# Check if the compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful! The executable 'secshell' has been created."
else
    echo "Compilation failed. Please check the error messages above."
    exit 1
fi
