# Monitoring Level Air pada Tangki Eksperimen (C++ Implementation)

## Deskripsi Proyek

Proyek ini bertujuan untuk membangun sistem monitoring level air pada tangki eksperimen yang digunakan di laboratorium elektro-mekanik. Sistem dirancang untuk:
- Mengirim data dari sensor (client) ke server monitoring secara otomatis.
- Menyimpan data level air secara periodik.
- Mendeteksi momen kritis (terlalu rendah/tinggi).
- Menangani banyak koneksi secara paralel dan aman.

Proyek ini dibangun dengan bahasa **C++** menggunakan arsitektur **Client-Server**, teknik **multithreading**, serta format penyimpanan **biner dan JSON**.

---

## Arsitektur Sistem
+-------------+ TCP Socket +-------------+
| Sensor 1 | ---------------------> | |
| (Client) | | |
+-------------+ | |
| |
+-------------+ TCP Socket | Server |
| Sensor 2 | ---------------------> | |
| (Client) | | |
+-------------+ | |
| |
Multithreaded & Buffered |
+-------------+

## Struktur Folder
project/
├── include/ # Header files
│ └── data_point.hpp
├── src/ # Source code
│ ├── server.cpp
│ ├── client.cpp
│ └── utils.cpp
├── data/ # Output data
│ ├── backup.dat # File biner
│ └── critical.json # Data level kritis
├── README.md # Dokumentasi proyek
├── Makefile # Build system
└── requirements.txt # Dependency eksternal

---

## Fitur

### 1. Client–Server Communication
- Menggunakan socket TCP.
- Client mengirim data level air secara periodik.
- Server menerima data dan menyimpannya dalam buffer.

### 2. Data Management
- Setiap data mencakup `timestamp` dan `level`.
- Disimpan dalam `std::vector<DataPoint>`.
- Dapat dicari dan diurutkan berdasarkan waktu.

### 3. Persistensi Data
- Backup otomatis ke file biner (`backup.dat`).
- Ekspor kondisi kritis ke file JSON (`critical.json`) menggunakan `nlohmann/json`.

### 4. Pemrosesan Paralel & Sinkronisasi
- Server multithread untuk menangani banyak sensor sekaligus.
- `std::mutex` digunakan untuk menjaga konsistensi buffer data.

---

##  Dependencies

- C++17 atau lebih baru
- POSIX socket (Linux/macOS) atau Winsock (Windows)
- [`nlohmann/json`](https://github.com/nlohmann/json) untuk ekspor data ke JSON

---
