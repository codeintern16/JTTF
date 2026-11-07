# JUMP TO TARGET FILE (Warp) 🚀

A blazing-fast, frecency-based directory navigation engine driven by a **C++/Rust Hybrid Architecture**. 

Warp tracks your most used directories and uses a dual-layered **Tiered Match + Frecency (Frequency + Recency)** algorithm to help you jump anywhere in your filesystem with just a few keystrokes.

![C++](https://img.shields.io/badge/C++-17-blue.svg) ![Rust](https://img.shields.io/badge/Rust-2021-orange.svg) ![CMake](https://img.shields.io/badge/Build-CMake-brightgreen.svg)

## ✨ Technical Highlights
- **Dual-Core Engine**: C++ and Rust.
- **Layered Decision Algorithm**: tier 1 < tier 2 < tier 3.
- **Race-Condition Free**: OS-level atomic writes (PID-based tmp files + `rename`) to prevent `.warp_history` corruption.
- **Shell-Injection Proof**: Raw POSIX `fork`/`exec` calls to `fd` for secure, lightning-fast fallback scanning.

## 🛠️ Installation

### Prerequisites
- **C++ Compiler**: GCC or Clang (supporting C++17)
- **CMake**: `>= 3.10`
- **fd**: Extreme-speed fallback directory discovery (`sudo pacman -S fd` or `sudo apt install fd-find`)
- **Rust & Cargo**: Required to build the `jttf-matcher` engine.
  * **Arch Linux**: 
    ```bash
    sudo pacman -S rustup
    rustup default stable
    ```
  * **Ubuntu / macOS / Others**: 
    ```bash
    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
    ```

### Structure
```text
JTTF/
├── CMakeLists.txt          # The ruler of C++ and Rust
├── README.md               # Project documentation
├── scripts/
│   └── warp.sh             # Bash Interceptor
├── src/                    # C++ Core
│   ├── main.cpp            # CLI Tiered Match
│   ├── side.cpp            # POSIX fork/exec fd, Atomic I/O, FFI
│   └── side.h              # Data structure (Record) & C++ API
└── jttf-matcher/           # Rust rating subsystem (Standalone Cargo Workspace)
    ├── Cargo.toml          # Cargo config: exports staticlib target
    └── src/
        └── lib.rs          # Core logic for rating
```

### Build & Install
Clone the repository and build from source:

```bash
git clone https://github.com/codeintern16/JTTF.git
cd JTTF
mkdir -p build
cd build
cmake ..
make
sudo make install
```

### Environment Setup
Add the following line to your `~/.bashrc` (or `~/.zshrc`) to enable the directory hook:

```bash
# For Bash
echo "source /usr/local/share/warp/warp.sh" >> ~/.bashrc
source ~/.bashrc

# For Zsh
echo "source /usr/local/share/warp/warp.sh" >> ~/.zshrc
source ~/.zshrc
```

## 💡 Usage

```bash
# Jump to the highest-scoring directory matching "lab"
warp lab

# Force a deep scan for a newly cloned project "pwn" (bypasses cache)
warp +pwn

# View your directory leaderboard
warp --list

# Clean up deleted directories from the cache
warp --clean
```


