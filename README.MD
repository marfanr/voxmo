# VOXMO
Utility untuk membundel kernel module dalam satu file biner untuk sistem operasi **Voxia OS**.

---

## Cara Penggunaan

```bash
mkdir build && cd build
cmake .. && make
./voxmo -o {output_name}.voxmo {file1} {file2} ...
```

File yang wajib disertakan:
1. File biner ELF
2. File `manifest.yml`

---

## Contoh Manifest

```yml
name: test
version: 0.2
author: mohammad arfan

main: flui.elf

cap:
  - PCI
```

---

## Format File VOXMO

Format `.voxmo` memiliki struktur modular: dimulai dari **header**, diikuti beberapa **metadata file**, dan masing-masing diikuti oleh **data file**.  
Struktur ini memungkinkan bundling beberapa file menjadi satu paket modul yang mudah dibaca oleh kernel loader.

---

### 1. Header (`metadata_header`)

Header berada di awal file dan berisi informasi umum tentang modul.

| Field          | Tipe              | Keterangan                                             |
| --------------- | ----------------- | ------------------------------------------------------ |
| `magic`         | `uint32_t`        | Magic number file, contoh: `0x564F584D` (`'VOXM'`)     |
| `version`       | `uint16_t`        | Versi format metadata                                  |
| `header_len`    | `uint32_t`        | Panjang keseluruhan header (termasuk semua string dan list) |
| `file_counts`   | `uint32_t`        | Jumlah file dalam bundle                               |
| `nama_module`   | `metadata_string` | Nama modul                                             |
| `description`   | `metadata_string` | Deskripsi modul                                        |
| `license`       | `metadata_string` | Lisensi modul                                          |
| `version_str`   | `metadata_string` | Versi modul                                            |
| `author`        | `metadata_string` | Nama pembuat                                           |
| `main_file`     | `metadata_string` | Nama file utama modul                                  |
| `capability`    | `metadata_list`   | Daftar kemampuan atau modul yang didukung              |

#### Struktur Header

```cpp
#ifndef __METADATA_HH__
#define __METADATA_HH__

#include <cstdint>

#pragma pack(push, 1)
struct metadata_string {
    uint16_t length;
    uint64_t pos;
};

struct metadata_list {
    uint16_t count;
    struct metadata_string *items;
};

struct metadata_header {
    uint32_t magic;
    uint16_t version;
    uint32_t header_len;
    uint32_t file_counts;

    struct metadata_string nama_module;
    struct metadata_string description;
    struct metadata_string license;
    struct metadata_string version_str;
    struct metadata_string author;
    struct metadata_string main_file;

    struct metadata_list capability;
};

struct metadata_file {
    uint64_t offset;
    uint32_t metadata_length;
    uint32_t size;
    struct metadata_string nama_file;
};

#pragma pack(pop)
#endif // __METADATA_HH__
```

---

### 2. Metadata File (`metadata_file`)

Setiap file yang dibundel memiliki metadata-nya sendiri.

| Field             | Tipe              | Keterangan                                        |
| ----------------- | ----------------- | ------------------------------------------------- |
| `offset`          | `uint64_t`        | Offset ke metadata file berikutnya (0 jika akhir) |
| `metadata_length` | `uint32_t`        | Panjang metadata (termasuk nama file)             |
| `size`            | `uint32_t`        | Ukuran isi file dalam byte                        |
| `nama_file`       | `metadata_string` | Nama file yang disertakan                         |

#### Layout di File (Little Endian)

```
[offset][metadata_length][size]
[nama_file.length][nama_file.pos]
```

---

### 3. Urutan Data dalam File `.voxmo`

Secara keseluruhan, urutan isi file adalah sebagai berikut:

```
+-------------------------+
| metadata_header         |
+-------------------------+
| metadata_file #1        |
+-------------------------+
| metadata_file #2        |
+-------------------------+
| .....                   |
+-------------------------+
| string data block       |
+-------------------------+
| file data #1            |
+-------------------------+
| file data #2            |
+-------------------------+
| ...                     |
+-------------------------+
```

Keterangan:
- Header selalu diletakkan di awal.
- Semua `metadata_file` diletakkan berurutan.
- Setelah  metadata string, baru diikuti oleh isi file masing-masing sesuai urutan.
- `offset` menunjukkan posisi metadata file berikutnya untuk navigasi cepat.

---

### 4. Catatan Penting

- Semua field numerik disimpan dalam **little-endian**.
- Semua string diakhiri dengan `'