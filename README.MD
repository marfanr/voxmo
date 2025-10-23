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

| Field         | Tipe              | Keterangan                                             |
| ------------- | ----------------- | ------------------------------------------------------ |
| `magic`       | `uint32_t`        | Magic number file, contohnya `0x564F584D` (`'VOXM'`)   |
| `version`     | `uint16_t`        | Versi format VOXMO                                     |
| `header_len`  | `uint32_t`        | Panjang header termasuk semua string dan list metadata |
| `nama_module` | `metadata_string` | Nama modul (`length` dan `data`)                       |
| `description` | `metadata_string` | Deskripsi modul                                        |
| `license`     | `metadata_string` | Lisensi modul                                          |
| `version_str` | `metadata_string` | Versi modul dalam string                               |
| `author`      | `metadata_string` | Nama penulis                                           |
| `main_file`   | `metadata_string` | Nama file utama modul                                  |
| `capability`  | `metadata_list`   | Daftar capability/module yang didukung                 |

**Layout di file (byte order little-endian):**

```
[magic][version][header_len]
[nama_module.length][nama_module.data]
[description.length][description.data]
[license.length][license.data]
[version_str.length][version_str.data]
[author.length][author.data]
[main_file.length][main_file.data]
[capability.count][capability.items...]
```

* `metadata_string` terdiri dari 2 byte length diikuti data string (tidak null-terminated).
* `metadata_list` terdiri dari 2 byte count, diikuti `metadata_string` untuk tiap item.

## 2. Metadata File (`metadata_file`)

Setiap file dalam modul memiliki metadata yang menyertainya:

| Field             | Tipe              | Keterangan                                        |
| ----------------- | ----------------- | ------------------------------------------------- |
| `next_offset`     | `uint64_t`        | Offset file berikutnya relatif terhadap awal file |
| `metadata_length` | `uint32_t`        | Panjang metadata file ini, termasuk nama file     |
| `size`            | `uint32_t`        | Ukuran file (bytes)                               |
| `nama_file`       | `metadata_string` | Nama file (`length` dan `data`)                   |

**Layout di file (byte order little-endian):**

```
[next_offset][metadata_length][size][nama_file.length][nama_file.data]
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

* Semua string (`metadata_string.data`) **tidak null-terminated**, panjangnya ditentukan oleh `length`.
* Magic number digunakan untuk validasi format file saat load.
