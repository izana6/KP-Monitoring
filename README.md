# KP-Monitoring
Proyek ini bertujuan untuk merancang sistem yang memantau kualitas udara secara real-time di area bengkel SMK Al Huda Bumiayu, yang sering digunakan untuk aktivitas praktikum otomotif. Sistem ini bertujuan memberikan data mengenai suhu, kelembapan, dan konsentrasi gas berbahaya seperti karbon dioksida (CO₂), karbon monoksida (CO), dan hidrokarbon (HC), yang dapat berdampak negatif pada kesehatan.

Sistem ini menggunakan beberapa komponen utama sensor, antara lain:
1. MQ-135: Untuk mendeteksi gas CO₂.
2. MiCS 5524: Untuk mendeteksi gas CO dan HC.
3. DHT22: Untuk mengukur suhu dan kelembapan udara.

Data dari sensor dikumpulkan menggunakan mikrokontroler ESP8266 yang mengirimkan data ke platform ThingSpeak untuk visualisasi dan analisis. Sistem ini juga dilengkapi dengan modul RTC DS3231 untuk penanda waktu, LCD 16x2 I2C untuk menampilkan data real-time, relay 2 channel yang dikendalikan sesuai kondisi tertentu dan Module Traffic Light sebagai peringatan visual.

Data yang dikumpulkan dari sensor secara berkala dikirim ke ThingSpeak, di mana data kualitas udara divisualisasikan untuk memantau kondisi secara real-time. Data ditampilkan dalam grafik sehingga sekolah dapat melihat perubahan parameter kualitas udara dari waktu ke waktu. Jika polusi udara mencapai tingkat yang berbahaya, sistem memberikan peringatan visual menggunakan lampu indikator.

Sistem monitoring ini dinilai berhasil mengumpulkan data secara akurat dan memberikan informasi real-time tentang kondisi kualitas udara di lingkungan sekolah. diharapkan dapat membantu pihak sekolah dalam menjaga kesehatan siswa dan guru, serta meningkatkan kesadaran tentang pentingnya kualitas udara yang baik di lingkungan belajar
