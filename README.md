# 🐶 Doge Miner

A lightweight Dogecoin Stratum miner written in C for low-power devices like the **Raspberry Pi Zero 2 W**.  
This miner uses the **scrypt** hashing algorithm and connects to public mining pools over **Stratum**.

---

## 📦 Features

- 🔑 Supports Stratum protocol (tested with Aikapool)
- ⚡ Adjustable CPU intensity
- 🐢 Lightweight — designed for minimal systems (like Pi Zero 2)
- 📈 Hashrate statistics and accepted share logs
- 🛠 Written in clean portable C (C99)

---

## 🛠 Build Instructions

Tested on Raspberry Pi Zero 2 W (Debian/Raspberry Pi OS).  
Dependencies: OpenSSL, Jansson.

### 1. Install dependencies

```bash
sudo apt update
sudo apt install build-essential libssl-dev libjansson-dev
```

### 2. Clone & build

```bash
git clone https://github.com/YOUR_USERNAME/doge-miner.git
cd doge-miner
make
```

This will generate an executable `doge-miner`.

---

## 🚀 Usage

```bash
./doge-miner [--debug] [--intensity <1-100>]
```

- `--debug` — print raw JSON from server
- `--intensity` — limit CPU usage by throttling hash loop (default: 100)

### Example

```bash
./doge-miner --intensity 65
```

---

## ⚙️ Configuration

Edit the `config.h` file:

```c
#define POOL_HOST "stratum.aikapool.com"
#define POOL_PORT 7915
#define USERNAME  "houne"
#define PASSWORD  "x"
```

---

## 📁 Output

- `miner.log` — logs accepted shares with timestamp
- Console shows:
  - connection state
  - current job
  - accepted shares and hashrate

---

## 📄 License

MIT License