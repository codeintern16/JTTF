# JUMP TO TARGET FILE 🚀

file jumper for Linux, written in C++ and Bash. 

Warp tracks your most used directories and uses a **Frecency (Frequency + Recency)** algorithm to help you jump anywhere in your filesystem with just a few keystrokes.

## 🛠️ Installation

### Prerequisites
* `cmake` (version 3.10+)
* A C++17 compatible compiler (GCC/Clang)

### Build & Install
Clone the repository and build from source:

```bash
git clone [https://github.com/codeintern16/JTTF.git](https://github.com/codeintern16/JTTF.git)
cd warp/build
cmake ..
make
sudo make install
```

### Environment Setup
Add the following line to your ~/.bashrc (or ~/.zshrc) to enable the cd hook:

```Bash
source /usr/local/share/warp/warp.sh
```

### 💡 Usage
```
# Jump to the highest-scoring directory matching "lab"
warp lab

# Force a deep scan for a newly cloned project "pwn"
warp +pwn

# View your directory leaderboard
warp --list

# Clean up deleted directories from the cache
warp --clean
```

