#!/bin/bash

# Function to install Vulkan SDK using apt
install_vulkan_apt() {
    echo "Detected apt package manager. Installing Vulkan SDK using apt..."
    # Add LunarG public signing key
    wget -qO- https://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo tee /etc/apt/trusted.gpg.d/lunarg.asc
    # Add LunarG Vulkan repository
    sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-noble.list http://packages.lunarg.com/vulkan/lunarg-vulkan-noble.list
    # Update package list and install Vulkan SDK
    sudo apt update
    sudo apt install vulkan-sdk -y
}

# Function to install Vulkan SDK using pacman
install_vulkan_pacman() {
    echo "Detected pacman package manager. Installing Vulkan SDK using pacman..."

    # Update the system and install Vulkan packages
    sudo pacman -Syu --noconfirm

    # Install Vulkan headers, tools, and drivers (for 64-bit systems)
    sudo pacman -S --noconfirm vulkan-headers vulkan-tools vulkan-utility-libraries vulkan-validation-layers

    # Install GPU-specific Vulkan drivers (for example, for Intel or AMD)
    if lspci | grep -i "vga" | grep -i "amd"; then
        sudo pacman -S --noconfirm vulkan-radeon
    elif lspci | grep -i "vga" | grep -i "intel"; then
        sudo pacman -S --noconfirm vulkan-intel
    fi

    echo "Vulkan SDK and dependencies installed successfully."
}



# Detect the package manager
if command -v apt &>/dev/null; then
    install_vulkan_apt
elif command -v pacman &>/dev/null; then
    install_vulkan_pacman
else
    echo "Error: Neither apt nor pacman package manager found. Unable to install Vulkan SDK."
    exit 1
fi
