# BrotliPack

A lightweight ELF binary packer/obfuscator based off the brotli compression algorithm

## Dependencies

- libbrotli-dev
- gcc
- make

Install on Debian/Ubuntu:

```bash
sudo apt update
sudo apt install -y build-essential libbrotli-dev
```

## Building

```shell
git clone https://github.com/BrooksRogers/brotliPack.git
cd brotliPack
make
```

## Usage

```shell
./pack.sh <your_binary_name> <output_packed_binary>
```
