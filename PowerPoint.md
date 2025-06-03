# Powerpoint

## Inti pembahasan di dalamnya : 

###- Deskripsi Solusi untuk setiap masalah yang didefinisikan


###- Arsitektur dari system

  
###- Use Case dari system yang bertujuan mengimplementasikan solusi yang diajukan


###- Detail Program, terutama untuk menjawab kriteria-kriteria dalam deskripsi masalah proyek

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

Terdapat 4 kriteria utama yang terdapat pada  definisi masalah berupa :

####Arsitektur Client - Server
  
Arsitektur umum sistem berbasis TCP. Client mewakilkan sensor yang mensimulasikan pengiriman data water level. Sementara server menerima data dari client dan menyimpan secara temporer dengan pencadangan berkala, data kritis dicatat dalam format JSON.

Mekanisme Komunikasi menggunakan protokol TCP socket karena reliable.

Alur komunikasinya berupa :

Client : Membuka koneksi ke server (127.0.0.1:8888) - > Mengirimkan ID sensor (CLIENT_ID) sebagai identifikasi -> Mengirim 10 data level air acak (0.00 - 100.00) setiap 2 detik

Server : Menerima ID sensor, mencatat sumber data -> Menerima data level air dan mencatatnya dalam buffer (std::vector<DataPoint>) -> Data disimpan dalam in-memory buffer sampai backup periodik dijalankan.

Terdapat penyimpanan lokal dari server ke dua jenis file : 
- backup.dat: Binary file untuk semua data (timestamp, level, clientId).
- critical.json: File JSON hanya berisi data yang melewati ambang kritis (< 20 atau > 80).


####Searching & Sorting Data


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

####Persistensi Data

####Paralel Processing & Sinkronisasi
