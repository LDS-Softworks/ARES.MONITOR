#!/usr/bin/env bash

##
##  Helpers/install.sh
##  Copyright (c) 2025 Lilly Aizawa @ LDS LLC. All rights reserved.
##  License: MIT
##  Helper script to install ARES MONITOR on a Linux system, and refuse if we are on Android or if we somehow passed through the macOS Checks by the Make task.
##

BNAME="ares"
# Default selection for the binary.
## This script is supposed to only run if we are on Linux, but, it is possible that we are on BSD or some other POSIX Compliant OS that COULD pass through the makefile with a few tweaks on environment variables or spoofing.
## Not recommended to spoof your OS Name, it will break stuff, and some stuff might be missing. causing instability.
BINARY="bin/ARES.MON.LINUX_x64"

if [[ ! -f "$BINARY" ]]; then
    echo "Error: Binary $BINARY not found. Please build the Binary first, before installing."
    return 1
fi


INSTALL_DIR="$HOME/System/Public/Programs/" ## Default, we default to the LDS Standard PATH Directory. only if it exist, as we check this later.
## This is simply a default value.
DEF_INSTDIR="$HOME/.local/bin"

# Override INSTALL_DIR if INSTPATH is set
if [ -n "$INSTPATH" ]; then
    INSTALL_DIR="$INSTPATH"
    echo "Using INSTPATH: $INSTPATH"
else
    echo "Using default INSTALL_DIR: $INSTALL_DIR"
fi

# Use [ -d ] to check for directory existence directly
# Adding spaces inside brackets is mandatory in Bash
if [[ ! -z $ANDROID_ROOT && ! -z $ANDROID_DATA ]]; then
    echo "Cannot install ARES On an Android device via normal means, please use on a rooted device, and install via the Magisk Module."
    return 1
fi

install_Custom() {
    if [[ -d "$INSTALL_DIR" ]]; then
        echo "Custom Install selected. Installing to $INSTALL_DIR..."
        cp "$BINARY" "$INSTALL_DIR/$BNAME" && chmod +x "$INSTALL_DIR/$BNAME"
    else
        echo "Specified installation directory $INSTALL_DIR does not exist. Please create it and re-run the installer."
        return 1
    fi
}

installPrivileged() {
    if [[ -d "/usr/local/bin" ]]; then
        echo "Privileged Install selected. Installing to /usr/local/bin..."
        echo "Requires sudo privileged for copying $BINARY into /usr/local/bin:"
        su -c "cp '$BINARY' '/usr/local/bin/$BINARY'"
        echo "Requires sudo privileges to set executable bit for /usr/local/bin/ares:"
        su -c "chmod +x '/usr/local/bin/$BNAME'"
    else
        echo "Directory /usr/local/bin not found. Please ensure it exists and you are in the sudoers file or use --prefix when installing."
        return 1
    fi
}

installSafe() {
    if [[ -d "$INSTALL_DIR" ]]; then
        echo "LDS DPATH found. Installing to $INSTALL_DIR..."
        cp "$BINARY" "$INSTALL_DIR/$BNAME" && chmod +x "$INSTALL_DIR/$BNAME"
    else
        if [[ -d "$DEF_INSTDIR" ]]; then
            ## If the LDS Path is not found, we default to $HOME/.local/bin. this might need to be added to the path if not already added.
            echo "LDS DPATH not found. Defaulting to $DEF_INSTDIR..."
            cp "$BINARY" "$DEF_INSTDIR/$BNAME" && chmod +x "$DEF_INSTDIR/$BNAME"
        elif [[ -d "/usr/local/bin" ]]; then
            echo "Neither LDS DPATH nor $DEF_INSTDIR found. Defaulting to /usr/local/bin..."
            cp "$BINARY" "/usr/local/bin/$BNAME" && chmod +x "/usr/local/bin/$BNAME"
        else
            echo "No suitable installation directory found."
            echo "Do you wish to install to '/usr/local/bin' instead? This may require sudo privileges. (y/n)"
            read -r response
            if [[ "$response" == "y" || "$response" == "Y" ]]; then
                installPrivileged
            else
                echo "Installation aborted. Please create either $INSTALL_DIR or $DEF_INSTDIR and re-run the installer."
                return 1
            fi
        fi
    fi
}

if [[ $(uname) == "Linux" ]]; then
    echo "Linux system detected. Proceeding with installation..."
    case $1 in
        --prefix)
            if [[ -z $2 ]]; then
                echo "Error: No installation directory specified after --prefix. Please provide a directory."
                return 1
            fi
            INSTALL_DIR="$2"
            install_Custom
            shift 2
            ;;
        --safe-lds)
            echo "Safe LDS Install selected. Defaulting to $HOME/System/Public/Programs/ if it exists, otherwise falling back to $DEF_INSTDIR or /usr/local/bin."
            INSTALL_DIR="$HOME/System/Public/Programs/"
            installSafe
            shift
            ;;
        --privileged)
            echo "Privileged Install selected. Defaulting to /usr/bin. This may require sudo privileges and is not recommended for general users."
            INSTALL_DIR="/usr/bin"
            installPrivileged
            shift
        ;;
        *)
            echo "No valid installation option provided. Defaulting to safe install method."
            installSafe
            ;;
    esac
#     if [[ -d "$INSTALL_DIR" ]]; then
#         echo "LDS DPATH found. Installing to $INSTALL_DIR..."
#         cp "$BINARY" "$INSTALL_DIR/$BNAME" && chmod +x "$INSTALL_DIR/$BNAME"
#     else
#         if [[ -d "$DEF_INSTDIR" ]]; then
#             ## If the LDS Path is not found, we default to $HOME/.local/bin. this might need to be added to the path if not already added.
#             echo "LDS DPATH not found. Defaulting to $DEF_INSTDIR..."
#             cp "$BINARY" "$DEF_INSTDIR/$BNAME" && chmod +x "$DEF_INSTDIR/$BNAME"
#         elif [[ -d "/usr/local/bin" ]]; then
#             echo "Neither LDS DPATH nor $DEF_INSTDIR found. Defaulting to /usr/local/bin..."
#             cp "$BINARY" "/usr/local/bin/$BNAME" && chmod +x "/usr/local/bin/$BNAME"
#         else
#             echo "No suitable installation directory found. Please create either $INSTALL_DIR or $DEF_INSTDIR and re-run the installer."
#             return 1
#         fi
#     fi
elif [[ $(uname) == "Darwin" ]]; then
    echo "Your system is running macOS, yet for some reason you were redirected to this script, please do not tamper with the installation logic if you do not know what you're doiing."
    echo "This is a bug. Please report this to LDS at https://github.com/LDS-Softworks/ARES.MONITOR/issues with the tag: InstallerBug and specify what steps you took before this bug presented on your device."
    echo "We apologize for the inconvenience, you can at any time run the proper installer for your system using the Helpers/macinstall.sh script."
    return 1
else
    echo "Unsupported operating system: $(uname). Installation aborted."
fi