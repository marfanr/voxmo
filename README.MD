# VOXMO
A Utility for Bundling Kernel Modules for the Voxia OS
### How To Use
```bash 
    mkdir build && cd build
```
```bash
    cmake .. && make
```
```bash
    ./voxmo -o {output name}.voxmo {file1} {file2} ...
```

file you must included
1.  binary elf
2.  manifest.yml

### manifest example
```yml
name: test
version: 0.2
author: mohammad arfan

main: flui.elf

cap:
  - PCI
```

# VOXMO File Layout

VOXMO menggunakan format file **modular** yang terdiri dari **header**, diikuti **metadata file**, lalu **isi file** secara berurutan. Format ini memungkinkan paket modul dengan beberapa file di dalam satu bundle.

## 1. Header (`metadata_header`)

Header berada di awal file dan berisi informasi modul secara umum.

| Field             | Tipe       | Keterangan                                           |
| ----------------- | ---------- | ---------------------------------------------------- |
| `magic`           | `uint32_t` | Magic number file, contohnya `0x564F584D` (`'VOXM'`) |
| `version`         | `uint16_t` | Versi format VOXMO                                   |
| `header_len`      | `uint32_t` | Panjang header, termasuk nama modul                  |
| `nama_module_len` | `uint16_t` | Panjang string nama modul                            |
| `nama_module`     | char[]     | Nama modul (`nama_module_len` byte)                  |

**Layout di file (byte order little-endian):**

```
[magic][version][header_len][nama_module_len][nama_module]
```

## 2. Metadata File (`metadata_file`)

Setiap file dalam modul memiliki metadata yang menyertainya. Metadata memberitahu lokasi file, ukuran, dan nama file.

| Field             | Tipe       | Keterangan                                        |
| ----------------- | ---------- | ------------------------------------------------- |
| `next_offset`     | `uint64_t` | Offset file berikutnya relatif terhadap awal file |
| `metadata_length` | `uint32_t` | Panjang metadata file ini, termasuk nama file     |
| `size`            | `uint32_t` | Ukuran file (bytes)                               |
| `nama_file_len`   | `uint16_t` | Panjang nama file                                 |
| `name`            | char[]     | Nama file (`nama_file_len` byte)                  |

**Layout di file (byte order little-endian):**

```
[next_offset][metadata_length][size][nama_file_len][name]
```

## 3. Urutan Data di File VOXMO

Secara keseluruhan, file VOXMO memiliki urutan:

```
Header
Metadata File 1
File 1
Metadata File 2
File 2
...
Metadata File N
File N
```

* `Header` selalu di awal.
* Setiap `metadata_file` diikuti langsung oleh **isi file** sesuai `size`.
* `next_offset` memungkinkan pembaca lompat ke metadata file berikutnya tanpa harus membaca seluruh file.

## 4. Catatan Tambahan

* Semua string (`nama_module` dan `name`) **tidak null-terminated**, panjangnya ditentukan oleh `_len` masing-masing.
* `magic` digunakan untuk validasi format file saat load.