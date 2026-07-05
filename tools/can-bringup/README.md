# CAN bring-up (cFS'ten bağımsız)

MCP2518FD tabanlı CAN arayüzünü (`can0`) **500 kbit/s** ile, cFS'ten bağımsız
olarak ayağa kaldıran bir systemd servisi. Amaç: donanım + OS katmanını tek
başına doğrulamak, böylece cFS `SIOCGIFINDEX ... no such device` hatasının
kaynağını izole etmek.

Board **headless** (HDMI/SSH yok) olduğu için tüm teşhis çıktısı FAT boot
partisyonundaki **`/boot/firmware/can-status.log`** dosyasına yazılır. Board'a
erişemesen bile SD kartı çıkarıp bu dosyayı herhangi bir PC'de okuyabilirsin.

---

## 1. config.txt — overlay'i şematiğe göre ayarla

Şematikte CAN çipi **SPI1**'e bağlı (GPIO18=CE0, GPIO19=MISO, GPIO20=MOSI,
GPIO21=SCLK), interrupt **GPIO16**, kristal **40 MHz**.

`/boot/firmware/config.txt`:

```ini
# CAN (MCP2518FD) — SPI1, CE0=GPIO18, INT=GPIO16, 40MHz osc
dtparam=spi=off
dtoverlay=mcp251xfd,spi1-0,interrupt=16,oscillator=40000000
```

> ⚠️ Raspberry Pi'nin aux SPI'ı (spi1) mcp251xfd ile sorunlu olabilir. Eğer
> reboot sonrası `can-status.log` içinde "`can0` does NOT exist" görürsen,
> donanımı SPI0'a taşımak (CE0=GPIO8, MISO=9, MOSI=10, SCLK=11) en garanti yol;
> o durumda overlay `spi0-0` olur.

## 2a. İnternet/USB/SSH YOKKEN kurulum — sadece SD kart (önerilen)

Hedef board headless ve ağa/USB'ye bağlanmıyorsa, her şeyi PC'de SD kartın
**FAT boot partisyonunu** düzenleyerek yapabilirsin. `can-utils` gerekmez;
bring-up sadece Lite'ta zaten kurulu olan `ip`/`dmesg`'i kullanır.

Boot partisyonuna (Windows'ta görünen sürücü) şu iki dosyayı kopyala:

- `can-bringup.sh`
- `firstrun.sh`

Sonra aynı partisyondaki **`cmdline.txt`**'in sonuna (dosya **tek satır**, mevcut
satırın sonuna boşlukla ekle, yeni satır ekleme) şunu yapıştır:

```
 systemd.run=/boot/firmware/firstrun.sh systemd.run_success_action=reboot systemd.unit=kernel-command-line.target
```

Kartı tak, güç ver. İlk boot'ta `firstrun.sh` root olarak çalışır, servisi
rootfs'e kurar, `cmdline.txt`'i temizler ve yeniden başlar. Bir daha karışman
gerekmez — sonraki her boot'ta `can-bringup.service` çalışır.

Sonucu oku: SD kartı çıkar, boot partisyonundaki **`firstrun.log`** (kurulum) ve
**`can-status.log`** (bring-up sonucu) dosyalarına bak.

> Not: `cmdline.txt`'i Windows editöründe düzenlerken CRLF eklememeye dikkat et.
> `firstrun.sh` scriptin CR'lerini otomatik temizler; ama `cmdline.txt`'i Notepad
> yerine Notepad++/VS Code ile "LF" olarak kaydetmen en güvenlisi.

## 2b. Servisi elle kur (geliştirme yaptığın, erişimin olan board'da)

```bash
sudo cp can-bringup.sh /usr/local/sbin/can-bringup.sh
sudo chmod +x /usr/local/sbin/can-bringup.sh
sudo cp can-bringup.service /etc/systemd/system/can-bringup.service
sudo systemctl daemon-reload
sudo systemctl enable can-bringup.service
sudo reboot
```

`can-utils` de kurulu olsun (test için):

```bash
sudo apt install -y can-utils
```

## 3. Sonucu oku

Board'u IO board'a takıp güç verdikten sonra SD kartı çıkar ve boot
partisyonundaki **`can-status.log`**'a bak. İçinde göreceklerin:

- `=== CAN bring-up OK ===`  → `can0` 500k'da up. Donanım + OS tamam, artık cFS
  çalışmalı.
- `=== CAN bring-up FAILED ===` → üstündeki `dmesg` / SPI device satırları
  sorunu gösterir (overlay yanlış, çip SPI'da görünmüyor, vs.).

## 4. Elle test (erişim varken)

```bash
# Durum
ip -details -statistics link show can0

# Dinle (bir terminal)
candump can0

# Gönder (başka terminal / başka node)
cansend can0 123#DEADBEEF
```

## 5. cFS ile ilişki

cFS kodunda değişiklik gerekmez — `canIOMC_hal_socketcan.c` zaten hazır `can0`
arayüzüne bağlanıyor. Bu servis `can0`'ı **up** bıraktığı için (`RemainAfterExit`),
cFS servisi sonra başladığında arayüz hazır olur. cFS servisini bu servise
bağımlı yapmak istersen cFS unit dosyasına ekle:

```ini
After=can-bringup.service
Requires=can-bringup.service
```
