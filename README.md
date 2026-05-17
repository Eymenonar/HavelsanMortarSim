# Havelsan Mortar Sim

Selamlar 👋

Bu proje **Havelsan mülakat ödevi** için hazırlandı. Unreal Engine 5.3 ile yaptım.  Ödevde istenen ana özelliklerin hepsi yerli yerinde. Aşağıda neler var, nasıl oynanır onları yazdım.

## Oyun Akışı

Karakter ile sahnede dolaşıyorsun, etrafa rastgele dağılmış havan topu mermilerini topluyorsun, sonra haritadaki havan topuna geçip toplandığın mermilerle ateş ediyorsun. 

## Özellikler

### Karakter Kontrolü
- Standart third-person karakter — WASD ile yürüme, mouse ile kamera, space ile zıplama.
- Havan topuna yaklaşıp E 'ye basınca karakteri bırakıp havana geçiyorsun. Tekrar E ile karaktere dönüyorsun.
- Havana geçince karakter sahneden gözden kayboluyor, geri dönünce yine görünür hale geliyor.

### Mermi Toplama
- Etraftaki mermileri F ile alıyorsun.
- Birden fazla mermi yakınsa en yakındakini alır.
- Alınan mermi sahneden silinir.

### Envanter
- I tuşu ile envanter ekranı aç/kapat.
- Toplam 40 kg taşıma kapasitesi, üstüne mermi alamıyorsun.
- Üzerinde hangi tipten kaç tane mermi olduğunu ve toplam ağırlığı görüyorsun.

### 3 Farklı Mermi Tipi
Görsel olarak ayırt edilebilen 3 mermi var, ağırlıkları ve davranışları farklı:

| Tip      | Ağırlık | Davranış                                       |
| -------- | ------- | ---------------------------------------------- |
| HE Light | 5 kg    | Stacklenebilir  aynı tipte alınca sayı artıyor |
| Standard | 10 kg   | Stacklenmiyor, her biri ayrı slot              |
| Heavy    | 15 kg   | Stacklenmiyor, her biri ayrı slot              |

### Havan Topu
- Havan topunu sadece sağa-sola (yaw) ok tuşları ile çeviriyorsun, yukarı-aşağı hareket etmiyor.
- Hangi yöne dönerse mermi de o yöne gidiyor.

### Atış Paneli (HUD)
Havan topuna geçince ekranda kontrol paneli açılıyor:
- Dropdown envanterdeki mermi tiplerinden birini seç. Yanında o tipten kaç tane kaldığı yazıyor.
- Initial Speed merminin çıkış hızını ayarladığın slider.
- Pitch merminin yere göre açısı (45° ile 90° arasında).
- Fire Butonu  atışı yapan buton. Eğer seçili merminin envanterde sayısı 0 ise buton pasif (basamıyorsun).

### Atış Davranışı
- Mermi gerçekçi bir parabolik yörüngede uçuyor 
- Atıştan sonra envanterde o tipten 1 tane düşer, sayı güncellenir.

### Görsel ve Ses Efektleri 
- Mermide trail uçarken arkasından alev/duman izi.
- Patlama efektimermi düştüğü yerde 5 saniyelik yer yanıyor.
- Atış sesi her ateşte tek ses.
- Patlama sesi seslerden rastgele biri çalıyor, hep aynı sesi duymuyorsun.

### Harita
- Düz büyük bir test alanı, havan topu ortada, etrafa rastgele dağılmış mermiler.
- Her oyun başlangıcında mermilerin yeri yeniden rastgele oluşuyor.

## Kontroller

| Tuş       | İşlev                            |
| --------- | -------------------------------- |
| WASD      | Yürü                             |
| Mouse     | Kamera                           |
| Space     | Zıpla                            |
| F         | Yakındaki mermiyi al             |
| I         | Envanteri aç/kapat               |
| E         | Havan topuna geç / karaktere dön |
| ← →       | Havan topunu sağa-sola çevir     |
## Drive Linki

Tam proje  Drive üzerinde:

[https://drive.google.com/file/d/1wrGBOcUgh05tL1WcJWMsCMHRs9I4DA9a/view?usp=sharing](https://drive.google.com/file/d/1wrGBOcUgh05tL1WcJWMsCMHRs9I4DA9a/view?usp=sharing)

## Yetişmedi 🙏

Multiplayer tarafının altyapısını kodda bulunuyor  yani  temeli var ama açıkçası zaman yetmediği için tam düzgün test edemedim. Buglı çalışıyor.

Engine: Unreal Engine 5.3
