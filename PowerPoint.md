# Powerpoint

## Inti pembahasan di dalamnya : 

### Deskripsi Solusi untuk setiap masalah yang didefinisikan


### Arsitektur dari system

  
### Use Case dari system yang bertujuan mengimplementasikan solusi yang diajukan


### Detail Program, terutama untuk menjawab kriteria-kriteria dalam deskripsi masalah proyek

---

## **Latar Belakang Masalah**

Dalam berbagai **percobaan di laboratorium elektro-mekanik Universitas Indonesia**, digunakan **tangki air** sebagai bagian dari sistem eksperimen. Pada eksperimen-eksperimen ini, **volume dan level air** di dalam tangki sangat penting karena secara langsung memengaruhi **validitas hasil percobaan**.

Saat ini, proses **pemantauan level air dilakukan secara manual**, yaitu oleh teknisi yang harus:

* **Mengukur level air** menggunakan alat ukur seperti **gauge**,
* **Mencatat hasilnya secara berkala** untuk setiap tangki yang digunakan.

Namun, metode manual ini memiliki beberapa kekurangan serius:

1. **Lambat dalam merespons kondisi kritis**:

   * Bila **level air terlalu rendah**, pompa bisa mengalami **dry-run** (beroperasi tanpa air), yang berisiko menyebabkan **kerusakan permanen** pada pompa.
   * Bila **terlalu tinggi**, air bisa **meluap**, membahayakan **peralatan elektronik** dan **lingkungan kerja** di sekitarnya.

2. **Tidak praktis untuk banyak tangki**:

   * Sering kali **lebih dari satu tangki digunakan secara bersamaan**,
   * Sehingga **pemantauan manual tidak efisien**, **memakan waktu**, dan sangat bergantung pada **ketelitian teknisi**,
   * **Risiko kelalaian meningkat**, terutama jika teknisi harus memantau banyak tangki sekaligus.

Berikut adalah **deskripsi solusi** untuk setiap masalah yang telah didefinisikan, **berdasarkan penjelasan proyek** yang diberikan:

## **1. Client–Server**

### **Solusi:**

* Sistem dirancang dengan **arsitektur client-server**, di mana setiap sensor air bertindak sebagai **client** dan sebuah aplikasi pusat sebagai **server monitoring**.
* **Client** membaca data level air dari sensor (dalam simulasi dikodekan) dan mengirimkannya secara berkala ke server melalui koneksi jaringan.
* **Server** mendengarkan koneksi pada port tertentu (default: `8888`), menerima data dari client, mencatat ID client, timestamp, dan nilai level air.
* Server dapat menerima **banyak koneksi secara paralel** menggunakan **multi-threading** (std::thread), membuat sistem mampu menangani berbagai sensor aktif secara simultan.

## **2. Pencarian & Pengurutan Data**

### **Solusi:**

* Data yang dikirim client ditampung dalam struktur buffer bersama di server.
* Data level air yang **melebihi atau di bawah ambang batas normal** dianggap **kritis** dan **diekspor secara periodik ke file `critical.json`** dalam format yang terstruktur.
* File JSON berisi informasi yang dapat dengan mudah **dicari dan diurutkan berdasarkan timestamp** menggunakan alat bantu seperti Python atau viewer JSON.
* Format JSON mendukung penyimpanan data dalam format key-value dan array of objects, sehingga mudah digunakan untuk pencarian dan pengurutan momen kritis.

## **3. Persistensi Data**

### **Solusi:**

#### a. **Backup Berkala (Biner)**

* Semua data dari client disimpan secara **berkala** ke file `backup.dat` dalam format **biner**.
* Ini memastikan **persistensi data** jika server dimatikan atau gagal; data tetap dapat dikembalikan.

#### b. **Ekspor Kritis (JSON)**

* Data yang dianggap **kritis** (di luar batas aman) akan diekspor ke file `critical.json`.
* File ini berisi **timestamp, ID sensor, dan nilai level air** untuk dokumentasi dan audit kondisi berbahaya.
* File ini terstruktur dan ringan, ideal untuk pemantauan jarak jauh atau pelaporan otomatis.

## **4. Pemrosesan Paralel & Sinkronisasi**

### **Solusi:**

* Server menggunakan **multi-threading** untuk menangani koneksi dari banyak client sekaligus.
* Setiap koneksi client diproses dalam **thread terpisah**, meningkatkan efisiensi dan kemampuan skala.
* Untuk menghindari kondisi balapan (race condition), server menggunakan **mutex (std::mutex)** untuk mengamankan akses ke buffer data bersama.
* Ini memastikan integritas data meskipun banyak thread aktif secara bersamaan.

---

Tentu! Berikut penjelasan **arsitektur sistem Water Level Monitoring** secara lebih lengkap dan terstruktur:

---

## **Arsitektur Sistem Water Level Monitoring (Client-Server)**

### **Overview Sistem**

Sistem ini dirancang untuk memonitor level air pada tangki eksperimen secara otomatis menggunakan pendekatan client-server. Sensor level air bertindak sebagai client yang mengirim data secara berkala ke server pusat yang bertugas mengelola, menyimpan, dan menganalisis data tersebut.

### **Komponen Utama**

#### a. **Client (Sensor Air)**
* Berfungsi sebagai pengirim data.
* Setiap client memiliki **ID unik** untuk mengidentifikasi data yang dikirim.
* Data level air dihasilkan atau diukur secara simulasi dan dikirim ke server.
* Menggunakan protokol TCP/IP untuk komunikasi jaringan.
* Data dikirim secara **periodik** ke server melalui koneksi socket pada port yang sudah ditentukan (default: 8888).
* Bisa ada banyak client yang berjalan secara simultan, misalnya beberapa sensor pada tangki yang berbeda.

#### b. **Server Monitoring**
* Menjadi pusat penerima dan pengelola data.
* Menunggu dan menerima koneksi dari berbagai client secara bersamaan dengan memanfaatkan **multi-threading** (`std::thread`).
* Setiap client yang terhubung diproses di thread terpisah agar tidak saling mengganggu dan dapat berjalan paralel.
* Data yang diterima berupa ID client, nilai level air, dan timestamp pengiriman.
* Data disimpan sementara di buffer bersama yang diakses oleh banyak thread.
* Untuk menjaga integritas data dan mencegah race condition, akses ke buffer menggunakan **mutex** (`std::mutex`).

### **Pengolahan dan Penyimpanan Data**

#### a. **Persistensi Data**
* Data semua client disimpan secara berkala ke dalam file **backup.dat** dengan format biner.
* Tujuan backup adalah memastikan data tidak hilang jika server tiba-tiba mati atau terjadi kegagalan sistem.
* Data yang disimpan mencakup seluruh data level air yang diterima, lengkap dengan timestamp dan ID sensor.

#### b. **Pengelolaan Data Kritis**
* Data yang melebihi batas aman (ambang atas atau ambang bawah level air) dianggap **kritis**.
* Data kritis ini secara periodik diekspor ke file **critical.json** dalam format JSON yang terstruktur.
* Format JSON memudahkan pencarian, pengurutan, dan analisis data menggunakan tool eksternal seperti Python atau aplikasi pembaca JSON.
* File ini berguna untuk pemantauan kondisi bahaya dan audit kondisi sistem.

### **Komunikasi dan Protokol**

* Sistem menggunakan protokol **TCP/IP** untuk komunikasi client-server, menjamin keandalan pengiriman data.
* Port default yang digunakan adalah **8888**, tetapi dapat dikonfigurasi sesuai kebutuhan.
* Server mampu menangani banyak koneksi secara paralel dengan model multi-threaded, sehingga skalabilitasnya baik.
* Client secara otomatis mengirim data level air secara berkala dan akan berhenti ketika data sudah terkirim semua.

### **Keamanan dan Sinkronisasi**

* Karena server menggunakan multi-threading untuk menangani banyak client, data yang diakses bersama perlu dilindungi agar tidak terjadi benturan (race condition).
* Mekanisme sinkronisasi menggunakan **mutex** menjamin setiap thread yang mengakses buffer data bersama dilakukan secara aman dan terkontrol.

### **Alur Sistem**

1. **Client (sensor)** membaca data level air secara simulasi.
2. Client mengirimkan data ke server melalui koneksi TCP.
3. Server menerima data dan menyimpannya di buffer bersama.
4. Server menilai apakah data termasuk kritis berdasarkan ambang batas.
5. Data kritis diekspor ke file JSON (`critical.json`) secara periodik.
6. Semua data secara berkala di-backup ke file biner (`backup.dat`) untuk memastikan persistensi.
7. Sistem siap menerima data dari banyak client secara simultan dan terus berjalan.

---

### Use Case dari system yang bertujuan mengimplementasikan solusi yang diajukan

Solusi yang diajukan pada intinya terdapat pada fitur utama yang telah kami rancang untuk proyek ini yang berupa : 

#### Arsitektur Client-Server: Sensor (client) mengirim data ke server.

Setiap sensor bertindak sebagai client yang menghubungi server, mengirimkan ID dan level air.

Kode untuk Client : 
```cpp
sendString(sock, CLIENT_ID); // Kirim ID unik sensor
send(sock, reinterpret_cast<char*>(&level), sizeof(level), 0); // Kirim level air
```

Kode untuk Server : 
```cpp
receiveString(clientSocket, clientId); // Terima ID
recv(clientSocket, reinterpret_cast<char*>(&level), sizeof(level), 0); // Terima data level air
```

#### Penanganan Koneksi Paralel: Server dapat menangani koneksi dari banyak sensor (client) secara bersamaan menggunakan multi-threading.

Server membuat thread baru untuk setiap koneksi client.

```cpp
std::thread(handleClient, clientSocket, std::string(clientIpStr), clientPortNum).detach();
```

#### Sinkronisasi Data: Akses ke buffer data bersama di server dilindungi oleh mutex untuk keamanan thread.

 Karena banyak thread mengakses dataBuffer (vector global), digunakan std::mutex dimana : 
- std::mutex bufferMutex; → objek kunci global.
- std::lock_guard<std::mutex> lock(bufferMutex); → digunakan setiap akses tulis/baca ke buffer.

```cpp
{
    std::lock_guard<std::mutex> lock(bufferMutex);
    dataBuffer.push_back({now, level, clientId});
}
```

#### Identifikasi Client: Setiap client mengirimkan ID unik ke server untuk identifikasi data.

Setiap sensor mengirim ID unik agar data dapat dilacak sumbernya.

dimana kode untuk Client : 
```cpp
const std::string CLIENT_ID = "SensorA001";
```
dimana kode untuk Server : 
```cpp
receiveString(clientSocket, clientId); // Membaca ID dari client
```

#### Persistensi Data:
Backup data lengkap ke file biner (backup.dat) secara periodik.
Ekspor data level kritis (di luar ambang batas normal) ke file JSON (critical.json) secara periodik.

File menyimpan semua data yang masuk, disimpan berkala.  (backup.dat)

```cpp
std::ofstream out(filename, std::ios::binary | std::ios::trunc);
out.write(reinterpret_cast<const char*>(&dp.timestamp), sizeof(dp.timestamp));
out.write(reinterpret_cast<const char*>(&dp.level), sizeof(dp.level));
```

Ekspor Level Kritis ke JSON (critical.json) Jika level < 20 atau > 80, simpan ke JSON.

```cpp
if (dp.level < LOW_THRESHOLD || dp.level > HIGH_THRESHOLD) {
    j_array.push_back({
        {"clientId", dp.clientId},
        {"timestamp", dp.timestamp},
        {"level", dp.level}
    });
}
```

#### Logging: Server menyediakan log untuk status operasi, koneksi, dan data yang diterima.

Server mencatat semua kejadian penting ke std::cout.

```cpp
std::cout << "[INFO] Client connected: ID=" << clientId << ...
std::cerr << "[ERROR] Client [" << clientId << "]: recv failed ...
std::cout << "[DATA] Client [" << clientId << "]: Level: ...
```
---
### Detail Program, terutama untuk menjawab kriteria-kriteria dalam deskripsi masalah proyek
Terdapat 4 kriteria utama yang terdapat pada  definisi masalah berupa :
---
#### Arsitektur Client - Server
  
Arsitektur umum sistem berbasis TCP. Client mewakilkan sensor yang mensimulasikan pengiriman data water level. Sementara server menerima data dari client dan menyimpan secara temporer dengan pencadangan berkala, data kritis dicatat dalam format JSON.

Mekanisme Komunikasi menggunakan protokol TCP socket karena reliable.

Alur komunikasinya berupa :

Client : Membuka koneksi ke server (127.0.0.1:8888) - > Mengirimkan ID sensor (CLIENT_ID) sebagai identifikasi -> Mengirim 10 data level air acak (0.00 - 100.00) setiap 2 detik

Server : Menerima ID sensor, mencatat sumber data -> Menerima data level air dan mencatatnya dalam buffer (std::vector<DataPoint>) -> Data disimpan dalam in-memory buffer sampai backup periodik dijalankan.

Terdapat penyimpanan lokal dari server ke dua jenis file : 
- backup.dat: Binary file untuk semua data (timestamp, level, clientId).
- critical.json: File JSON hanya berisi data yang melewati ambang kritis (< 20 atau > 80).


#### Searching & Sorting Data

Kriteria yang diperlukan berdasarkan definisi masalah : 

- Mencari momen level kritis: level < 20.0 atau > 80.0.
- Mengurutkan kejadian berdasarkan waktu (timestamp).

Detail Implementasinya pada program berupa kode berikut : 

```cpp void exportCriticalToJson(const std::string& filename) {
    std::lock_guard<std::mutex> lock(bufferMutex);
    json j_array = json::array(); 
    bool hasCriticalData = false;

    for (const auto& dp : dataBuffer) {
        if (dp.level < LOW_THRESHOLD || dp.level > HIGH_THRESHOLD) {
            hasCriticalData = true;
            j_array.push_back({
                {"clientId", dp.clientId}, 
                {"timestamp", dp.timestamp},
                {"level", dp.level}
            });
        }
    }
```
- if (dp.level < LOW_THRESHOLD || dp.level > HIGH_THRESHOLD) merupakan filter untuk data kritis sesuai dengan threshold yang diberikan.
- Data kemudian ditambahkan ke array JSON.
- Data tidak secara langsung di sort berdasarkan timestamp, tapi urutan data buffer sudah berdasarkan waktu yang diterma

#### Persistensi Data

Kriteria yang diperlukan berdasarkan definisi masalah : 

- Backup data level ke file biner
- Export timestamp ke JSON untuk dokumentasi

Detail implementasi ada pada cuplikan kode : 

```cpp
void backupToBinary(const std::string& filename) {
    std::lock_guard<std::mutex> lock(bufferMutex);
    ...
    for (const auto& dp : dataBuffer) {
        out.write(reinterpret_cast<const char*>(&dp.timestamp), sizeof(dp.timestamp));
        out.write(reinterpret_cast<const char*>(&dp.level), sizeof(dp.level));
        ...
    }
}

```

Serta secara periodik dipanggil dengan : 

```cpp
void periodicBackup() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        backupToBinary(...);
        exportCriticalToJson(...);
    }
}
```

- File biner digunakan (std::ios::binary) untuk keperluan backup yang didalamnya ditulis timestamp, level, dan panjang serta isi dari clientId.

#### Paralel Processing & Sinkronisasi


Kriteria yang diperlukan berdasarkan definisi masalah : 

-  Paralel Processing untuk menangani banyak client bersamaan
- Sinkronisasi antara koneksi aman

Detail kode untuk paralel processing : 

```cpp
std::thread(handleClient, clientSocket, std::string(clientIpStr), clientPortNum).detach();
```

- Setiap client ditangani thread yang terpisah (multithreaded)
- handleClient() berjalan paralel untuk setiap sensor yang aktif.

Detail kode untuk paralel Sinkronisasi : 

```cpp
std::mutex bufferMutex;
...
{
    std::lock_guard<std::mutex> lock(bufferMutex);
    dataBuffer.push_back({now, level, clientId});
}
```
- Semua akses read/write dataBuffer di kunci dengan lock_guard

# PowerPoint berisi rangkuman dari apa yang ada disini dan dapat diakses pada link : 
https://www.canva.com/design/DAGpTiErw1w/01iJRxzQnAsq00c3qrc6JQ/edit?utm_content=DAGpTiErw1w&utm_campaign=designshare&utm_medium=link2&utm_source=sharebutton
