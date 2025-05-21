# Monitoring Level Air pada Tangki Eksperimen (C++ Implementation)

## Deskripsi Proyek

Proyek ini bertujuan untuk membangun sistem monitoring level air pada tangki eksperimen yang digunakan di laboratorium elektro-mekanik. Sistem dirancang untuk:
- Mengirim data dari sensor (client) ke server monitoring secara otomatis.
- Menyimpan data level air secara periodik.
- Mendeteksi momen kritis (terlalu rendah/tinggi).
- Menangani banyak koneksi secara paralel dan aman.

Proyek ini dibangun dengan bahasa **C++** menggunakan arsitektur **Client-Server**, teknik **multithreading**, serta format penyimpanan **biner dan JSON**.

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
