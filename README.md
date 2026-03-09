# ![IAT Hook logo](/logo/image.png)
# IAT Hook

![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)
![Windows](https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white)
![GCC](https://img.shields.io/badge/GCC-A42E2B?style=for-the-badge&logo=gnu&logoColor=white)
![License](https://img.shields.io/badge/license-MIT-green?style=for-the-badge)

## Disclaimer
> This is a mini project demonstrating interaction with the Windows PE format and WinAPI.  
> For educational purposes only.

## Introduction

A minimal proof-of-concept of **IAT (Import Address Table) hooking** on Windows.  
Intercepts a call to `MessageBoxA` at runtime by patching the IAT entry — without touching the function itself.

## How it works

1. Gets a handle to the current process module via `GetModuleHandleA`
2. Walks the PE headers: `IMAGE_DOS_HEADER` → `IMAGE_NT_HEADERS` → `IMAGE_IMPORT_DESCRIPTOR`
3. Iterates over imported DLLs and their thunk entries
4. Finds `MessageBoxA` by name in `IMAGE_IMPORT_BY_NAME`
5. Temporarily marks the IAT entry as writable via `VirtualProtect`
6. Replaces the function pointer with a custom hook (`MyMessageBoxA`)
7. Restores original memory protection
8. Any subsequent call to `MessageBoxA` now goes through the hook

```
MessageBoxA("Original") → called before hook → shows "Original"
[hook installed]
MessageBoxA("Test")     → redirected → shows "Hooked!"
```

## Requirements

- Windows 10/11
- GCC compiler (MinGW-w64)

## PE Structure — how we get to the IAT

```
PE File in memory
│
├── IMAGE_DOS_HEADER        ← base address (GetModuleHandleA)
│   └── e_lfanew ──────────────────────────────────┐
│                                                   ▼
├── IMAGE_NT_HEADERS                         (base + e_lfanew)
│   ├── Signature                 "PE\0\0"
│   ├── IMAGE_FILE_HEADER
│   └── IMAGE_OPTIONAL_HEADER
│       └── DataDirectory[]
│           └── [1] IMPORT ──────────────────────────┐
│                                                     ▼
├── IMAGE_IMPORT_DESCRIPTOR[]            (one per imported DLL)
│   ├── Name              → "USER32.dll"
│   ├── OriginalFirstThunk → INT (Import Name Table)
│   │    └── IMAGE_IMPORT_BY_NAME
│   │         └── Name   → "MessageBoxA"   ← strcmp here
│   └── FirstThunk ────────────────────────────────────┐
│                                                       ▼
└── IAT (Import Address Table)               ← patch here!
     └── thunk->u1.Function = &MessageBoxA
                            ↓ VirtualProtect + overwrite
                            = &MyMessageBoxA
```

## Project Structure

```
IAT-Hook/
├── logo/
│   └── image.png
├── src/
│   └── main.c
├── LICENSE
└── README.md
```

## Compile with GCC

```bash
gcc main.c -o iat_hook.exe -lkernel32 -luser32
```

## Usage

Just compile and run — no arguments needed:

```bash
iat_hook.exe
```

You'll see two `MessageBox` windows:
- First one (before hook): shows `"Original"`
- Second one (after hook): shows `"Hooked!"`

## License

This project is licensed under the MIT License.