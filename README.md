# Proyek Monitoring Level Air (Client-Server)

Proyek ini mengimplementasikan sistem client-server untuk memonitor level air pada tangki eksperimen. Sensor (client) mengirimkan data level air secara periodik ke server monitoring, yang kemudian menyimpan data tersebut, melakukan backup, dan mengekspor data kritis.

## Fitur Utama

*   **Arsitektur Client-Server:** Sensor (client) mengirim data ke server.
*   **Penanganan Koneksi Paralel:** Server dapat menangani koneksi dari banyak sensor (client) secara bersamaan menggunakan multi-threading.
*   **Sinkronisasi Data:** Akses ke buffer data bersama di server dilindungi oleh mutex untuk keamanan thread.
*   **Identifikasi Client:** Setiap client mengirimkan ID unik ke server untuk identifikasi data.
*   **Persistensi Data:**
    *   Backup data lengkap ke file biner (`backup.dat`) secara periodik.
    *   Ekspor data level kritis (di luar ambang batas normal) ke file JSON (`critical.json`) secara periodik.
*   **Logging:** Server menyediakan log untuk status operasi, koneksi, dan data yang diterima.

## Prasyarat

*   **Compiler C++:** GCC (MinGW-w64 direkomendasikan di Windows) yang mendukung C++11 atau lebih tinggi dan POSIX threads (pthreads).
    *   Untuk Windows, disarankan menginstal MinGW-w64 melalui MSYS2 (misalnya, toolchain UCRT64).
*   **Library nlohmann/json:** Diperlukan untuk server guna menangani ekspor data JSON.
    *   Letakkan file `json.hpp` di direktori `include/nlohmann/` dalam root proyek.

## Struktur Direktori Proyek
![image](https://github.com/user-attachments/assets/89a4a4ed-b3ef-4338-ad11-998d9d5ccfd1)
- .vscode/ # Pengaturan VS Code (opsional)
- bin/ # Direktori output untuk executable
- include/nlohmann untuk tempat library JSON
- src/ Kode sumber client dan server

  
## Skrip Build (Kompilasi)

Pastikan Anda berada di direktori root proyek (`Water Level Monitoring/`) saat menjalankan perintah kompilasi di terminal. Ganti path ke compiler `g++.exe` jika instalasi MSYS2 Anda berbeda.

**1. Buat Direktori Output (jika belum ada):**
   ```bash
   mkdir bin
   mkdir data
   ```

**2. Kompilasi Client:**
Setiap client sebaiknya memiliki ID unik. Untuk contoh ini, ID di-hardcode dalam client.cpp. Jika Anda ingin beberapa client dengan ID berbeda, Anda perlu memodifikasi CLIENT_ID di client.cpp dan mengompilasi ulang untuk setiap client, atau memodifikasi client untuk menerima ID sebagai argumen.

- Ganti `C:/msys64/ucrt64/bin/g++.exe` dengan path compiler Anda jika berbeda
- Ketik C:/msys64/ucrt64/bin/g++.exe -std=c++11 src/client.cpp -o bin/client_sensorA.exe -lws2_32 -pthread` pada terminal untuk compile program client. Jika Anda ingin client lain, ubah `CLIENT_ID` di `src/client.cpp`, lalu kompilasi lagi: `C:/msys64/ucrt64/bin/g++.exe -std=c++11 src/client.cpp -o bin/client_sensorB.exe -lws2_32 -pthread`

**3. Kompilasi Server:**
Pastikan Anda telah mengatur DATA_DIRECTORY_PATH dengan benar di dalam file src/server.cpp agar menunjuk ke direktori data/ proyek Anda secara absolut.

- Ganti `C:/msys64/ucrt64/bin/g++.exe` dengan path compiler Anda jika berbeda 
- Lalu ketik `C:/msys64/ucrt64/bin/g++.exe -std=c++11 src/server.cpp -o bin/server.exe -Iinclude -lws2_32 -pthread` pada terminal

Catatan: Jika Anda telah menambahkan direktori bin dari compiler MinGW-w64 Anda ke PATH environment variable sistem dan telah me-restart komputer/terminal, Anda mungkin bisa menggunakan g++ saja tanpa path lengkap.

# Cara Pemakaian:
1. Jalankan Server:
- Buka terminal.
- Navigasi ke direktori root proyek (Water Level Monitoring/).
- Jalankan server: `.\bin\server.exe`
- Server akan mulai mendengarkan koneksi di port yang ditentukan (default 8888 dalam kode terakhir). Anda akan melihat log status di terminal server.

2. Jalankan Client (Sensor):
- Buka terminal baru (atau beberapa terminal baru jika ingin menjalankan banyak client).
- Navigasi ke direktori root proyek (Water Level Monitoring/).
- Jalankan client (misalnya, client_sensorA.exe):`.\bin\client_sensorA.exe`
- Client akan mencoba terhubung ke server, mengirimkan ID-nya, lalu mengirim 10 data level air secara periodik.
- Anda akan melihat log di terminal client dan juga log di terminal server yang menunjukkan data diterima dari client tersebut.

3. Amati Output dan File Data:
- Terminal Server: Akan menampilkan log koneksi client, data yang diterima, dan informasi backup.
Direktori `data/`:
`backup.dat`: Akan dibuat/diperbarui secara periodik, berisi semua data yang diterima dalam format biner.
`critical.json`: Akan dibuat/diperbarui secara periodik, berisi data level yang dianggap kritis dalam format JSON.

4. Menghentikan Aplikasi:
- Client: Akan berhenti secara otomatis setelah mengirim semua datanya.
- Server: Tekan Ctrl+C di terminal tempat server berjalan untuk menghentikannya.

# Debugging
1. Simulasi Data Ekstrem
Uji dengan data di bawah dan di atas ambang batas normal untuk memastikan file critical.json bekerja dengan benar.

2. Stress Test
Jalankan 10+ client sekaligus untuk menguji kestabilan multi-threading dan sinkronisasi mutex.

3. Tes File I/O
Simulasi disk penuh atau file terkunci untuk memverifikasi apakah sistem gagal secara aman.

# Konfigurasi Penting dalam Kode

- src/client.cpp:
`const std::string CLIENT_ID` = "SensorA001";: Ubah ini untuk setiap ID sensor yang unik.
`serverAddr.sin_port = htons(8888);`: Pastikan port ini cocok dengan port server.
- src/server.cpp:
`const std::string DATA_DIRECTORY_PATH = "...";`: WAJIB DIUBAH menjadi path absolut ke direktori data/ proyek Anda.
`std::thread serverThread(startServer, 8888);`: Port tempat server akan mendengarkan.

# Troubleshooting Umum
- "Bind failed!" pada Server: Port sudah digunakan oleh aplikasi lain. Hentikan aplikasi tersebut atau ubah port di server (dan client).
- "Connection failed." pada Client: Server tidak berjalan, alamat/port salah, atau firewall memblokir.
- "Failed to open ... file" pada Server: Pastikan DATA_DIRECTORY_PATH benar, direktori data/ ada, dan program memiliki izin tulis.
- Error kompilasi terkait std::thread atau -lpthread: Pastikan Anda menggunakan compiler MinGW-w64 yang mendukung pthreads dan menggunakan flag -pthread saat kompilasi.
