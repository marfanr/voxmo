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

#### Struktur `metadata_string`

```c
struct metadata_string {
    uint16_t length;  // Panjang string (termasuk '\0')
    uint64_t pos;     // Posisi data string dalam file
};
```

String sekarang **selalu diakhiri dengan `\0`** untuk mempermudah parsing tanpa tambahan copy buffer.

#### Struktur `metadata_list`

```c
struct metadata_list {
    uint16_t count;              // Jumlah item dalam daftar
    struct metadata_string *items; // Pointer ke elemen string
};
```

#### Layout Header di File (Little Endian)

```
[magic][version][header_len][file_counts]
[nama_module.length][nama_module.pos]
[description.length][description.pos]
[license.length][license.pos]
[version_str.length][version_str.pos]
[author.length][author.pos]
[main_file.length][main_file.pos]
[capability.count][capability.items...]
```

---

### 2. Metadata File (`metadata_file`)

Setiap file yang dibundel memiliki metadata-nya sendiri.

| Field             | Tipe              | Keterangan                                        |
| ----------------- | ----------------- | ------------------------------------------------- |
| `next_offset`     | `uint64_t`        | Offset ke metadata file berikutnya (0 jika akhir) |
| `metadata_length` | `uint32_t`        | Panjang metadata (termasuk nama file)             |
| `size`            | `uint32_t`        | Ukuran isi file dalam byte                        |
| `nama_file`       | `metadata_string` | Nama file yang disertakan                         |

#### Layout di File (Little Endian)

```
[next_offset][metadata_length][size]
[nama_file.length][nama_file.pos]
```

---

### 3. Urutan Data dalam File `.voxmo`

Secara keseluruhan, urutan isi file adalah sebagai berikut:

```
+-------------------------+
| metadata_header         |
+-------------------------+
| string data block       |
+-------------------------+
| metadata_file #1        |
+-------------------------+
| file data #1            |
+-------------------------+
| metadata_file #2        |
+-------------------------+
| file data #2            |
+-------------------------+
| ...                     |
+-------------------------+
```

Keterangan:
- Header selalu diletakkan di awal.
- Setiap `metadata_file` diikuti langsung oleh isi file sesuai dengan ukuran (`size`).
- `next_offset` memungkinkan sistem melompat langsung ke metadata berikutnya tanpa traversal penuh.

---

### 4. Catatan Penting

- Semua field numerik disimpan dalam **little-endian**.
- Semua string diakhiri dengan `\0`, tetapi panjang tetap dihitung di field `length`.
- Magic number wajib dicek untuk memvalidasi format file sebelum parsing.

---

### 5. Nilai Magic

| Field | Nilai | Deskripsi |
| ------ | ------ | ---------- |
| `magic` | `0x564F584D` | ASCII: `'VOXM'` |
| `version` | `1` | Versi pertama format VOXMO |
