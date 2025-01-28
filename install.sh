#!/bin/sh
set -eu

flatpak_package="app.getclipboard.Clipboard"

# Color codes for better readability
GREEN="\033[32m"
RED="\033[31m"
YELLOW="\033[33m"
RESET="\033[0m"

print_success() { printf "%b\n" "${GREEN}$1${RESET}"; }
print_error() { printf "%b\n" "${RED}$1${RESET}"; }
print_warning() { printf "%b\n" "${YELLOW}$1${RESET}"; }


unsupported() {
    print_error "Sorry, but this installer script doesn't support $1."
    printf "%b\n" "${GREEN}💡 However, you can still install CB using the other methods in the readme!${RESET}"
}

verify_flatpak() {
    if flatpak list | grep -q "$flatpak_package"
    then
        print_success "Clipboard installed successfully!"
        printf "%b\n" "${RESET}Add this alias to your terminal profile (like .bashrc) to make it work every time:${RESET}"
        printf "%b\n" "${YELLOW}alias cb=\"flatpak run $flatpak_package\"${RESET}"
        exit 0
    fi
    print_error "Unable to install CB with Flatpak"
    exit 1
}

can_use_sudo() {
    prompt=$(sudo -nv 2>&1)
    if sudo -nv >/dev/null 2>&1
    then
        return 0    # No password needed
    fi
    if echo "$prompt" | grep -q '^sudo:'
    then
        return 0    # Password needed but sudo available
    fi
    return 1       # Sudo not available
}

if can_use_sudo
then
    requires_sudo=true
    install_path="/usr/local"
    sudo mkdir -p "$install_path/bin"
    sudo mkdir -p "$install_path/lib"
else
    requires_sudo=false
    install_path="$HOME/.local"
    mkdir -p "$install_path/bin"
    mkdir -p "$install_path/lib"
fi

verify() {
  cd ..
  rm -rf "$tmp_dir"
  if command -v cb >/dev/null 2>&1
  then
      if ! cb >/dev/null 2>&1
      then
          print_error "Error with the runtime of cb, but able to execute." 
          exit 1
      fi
      print_success "CB is installed at $install_path/bin, be sure its on your PATH."
      exit 0
  fi
  print_error "CB failed to install on platform $(uname):$(uname -m)"
  exit 1
}

# has_header() {
#     header="$1"
#     # See if pre-processor exists 
#     if command -v cpp >/dev/null 2>&1
#     then
#         echo "#include <${header}>" | cpp -H -o /dev/null >/dev/null 2>&1
#         return
#     fi
#     # Try gcc if available
#     if command -v gcc >/dev/null 2>&1
#     then
#         echo "#include <${header}>" | gcc -E - >/dev/null 2>&1
#         return
#     fi
#     # Try clang if available
#     if command -v clang >/dev/null 2>&1
#     then
#         echo "#include <${header}>" | clang -E - >/dev/null 2>&1
#         return
#     fi
#     # No known compiler found
#     false
#     return
# }

has_apt(){
 command -v apt-get >/dev/null 2>&1
 return 
}

install_debian_deps(){
  if can_use_sudo
  then
    sudo apt-get install -qq openssl
    sudo apt-get install -qq libssl3
    sudo apt-get install -qq libssl-dev
  fi
  }

compile() {
    if has_apt
    then
      install_debian_deps
    fi

    git clone --depth 1 https://github.com/map588/Clipboard
    cd Clipboard/build
   
    cmake ..
    cmake --build .

    if [ "$(uname)" = "OpenBSD" ]
    then
        doas cmake --install .
    else
        if can_use_sudo
        then
          sudo cmake --install . 
          verify
        else
          mkdir -p "$HOME/.local"
          cmake --install . --prefix="$HOME/.local"
          verify
        fi
    fi
}

compile_and_verify(){
  print_error "No supported release download available for $(uname):$(uname -m)"
  print_success "Attempting compile with CMake..."
  compile
  verify
}

# Start installation process
print_success "Searching for a package manager..."

# Try package managers first
if command -v apk >/dev/null 2>&1
then
    if can_use_sudo
    then
        sudo apk add clipboard
        verify
    fi
fi

if command -v yay >/dev/null 2>&1
then
    if can_use_sudo
    then
        sudo yay -S clipboard
        verify
    fi
fi

if command -v emerge >/dev/null 2>&1
then
    if can_use_sudo
    then
        sudo emerge -av app-misc/clipboard
        verify
    fi
fi

if command -v brew >/dev/null 2>&1
then
    brew install clipboard
    verify
fi

if command -v flatpak >/dev/null 2>&1
then
    if can_use_sudo
    then
        sudo flatpak install flathub "$flatpak_package" -y
    else
        flatpak install flathub "$flatpak_package" -y
    fi
    verify_flatpak
fi

# if command -v snap >/dev/null 2>&1
# then
#     if can_use_sudo
#     then
#         sudo snap install clipboard
#         verify
#     fi
# fi

if command -v nix-env >/dev/null 2>&1
then
    nix-env -iA nixpkgs.clipboard-jh
    verify
fi

if command -v pacstall >/dev/null 2>&1
then
    pacstall -I clipboard-bin
    verify
fi

if command -v scoop >/dev/null 2>&1
then
    scoop install clipboard
    verify
fi

if command -v xbps-install >/dev/null 2>&1
then
    if can_use_sudo
    then
        sudo xbps-install -S clipboard
        verify
    fi
fi


print_error "No supported package manager found."
print_success "Attempting to download release zip file for architecture..."

tmp_dir=$(mktemp -d -t cb-XXXXXXXXXX)
cd "$tmp_dir" || exit 1

download_link="skip"

case "$(uname)" in
  "Linux")
    case "$(uname -m)" in
      "x86_64"  | "amd64")  download_link="https://github.com/Slackadays/Clipboard/releases/download/0.10.0/clipboard-linux-amd64.zip" ;;
      "aarch64" | "arm64") download_link="https://github.com/Slackadays/Clipboard/releases/download/0.10.0/clipboard-linux-arm64.zip" ;;
      "riscv64") download_link="https://github.com/Slackadays/Clipboard/releases/download/0.10.0/clipboard-linux-riscv64.zip" ;;
      "i386")    download_link="https://github.com/Slackadays/Clipboard/releases/download/0.10.0/clipboard-linux-i386.zip" ;;
      "ppc64le") download_link="https://github.com/Slackadays/Clipboard/releases/download/0.10.0/clipboard-linux-ppc64le.zip" ;;
      "s390x")   download_link="https://github.com/Slackadays/Clipboard/releases/download/0.10.0/clipboard-linux-s390x.zip" ;;
      *)         download_link="skip" ;;
    esac
    ;; 
  "Darwin")
    case "$(uname -m)" in
      "x86_64") download_link="https://github.com/Slackadays/Clipboard/releases/download/0.10.0/clipboard-macos-amd64.zip" ;;
      "arm64")  download_link="https://github.com/Slackadays/Clipboard/releases/download/0.10.0/clipboard-macos-arm64.zip" ;;
    esac
    ;;
   "FreeBSD")
      case "$(uname -m)" in
        "x86_64" | "amd64") download_link="https://github.com/Slackadays/Clipboard/releases/download/0.10.0/clipboard-freebsd-amd64.zip" ;;
                *) compile_and_verify 
      ;;
      esac
      ;;
    "NetBSD")
       case "$(uname -m)" in
        "x86_64" | "amd64") download_link="https://github.com/Slackadays/Clipboard/releases/download/0.10.0/clipboard-netbsd-amd64.zip" ;;
               *) compile_and_verify 
                  ;; 
       esac
       ;;
  *) compile_and_verify 
    ;;
esac

if [ "$download_link" != "skip" ]
then
  if [ "$(uname)" = "Linux" ]
  then
    curl -SsLl $download_link -o clipboard-linux.zip
    unzip clipboard-linux.zip
    rm clipboard-linux.zip
    set +e
     if [ "$requires_sudo" = true ]
      then
        sudo mv bin/cb "$install_path/bin/cb" 
        [ -f "lib/libcbx11.so" ] && sudo mv "lib/libcbx11.so" "$install_path/lib/libcbx11.so"
        [ -f "lib/libcbwayland.so" ] && sudo mv "lib/libcbwayland.so" "$install_path/lib/libcbwayland.so"
        sudo chmod +x "$install_path/bin/cb"
      else
        mv bin/cb "$install_path/bin/cb"
        [ -f "lib/libcbx11.so" ] && mv "lib/libcbx11.so" "$install_path/lib/libcbx11.so"
        [ -f "lib/libcbwayland.so" ] && mv "lib/libcbwayland.so" "$install_path/lib/libcbwayland.so"
        chmod +x "$install_path/bin/cb"
      fi  
    set -e
  fi
  elif [ "$(uname)" = "Darwin" ]
  then 
    curl -SsLl $download_link -o clipboard-mac.zip
    unzip clipboard-macos.zip
    rm clipboard-macos.zip
    sudo mv bin/cb "$install_path/bin/cb"
    chmod +x "$install_path/bin/cb"
  elif [ "$(uname)" = "NetBSD" ]
  then
    print_warning "Release is at $download_link, download and move libs and bin/cb somewhere sensible." 
    exit 0
else
  compile_and_verify
fi
