# Memperbaiki instalasi NUC140 versi 0.1.0

Perbaikan: arsip Boards Manager sekarang berisi satu direktori induk `nuc140-0.1.1/`.

1. Salin seluruh isi ZIP ke folder repo lokal `Arduino-NUC140/`, timpa file lama.
2. Buka CMD di folder repo, jalankan:

```cmd
git add .
git commit -m "Fix Board Manager archive root v0.1.1"
git push origin main
git tag v0.1.1
git push origin v0.1.1
```

3. GitHub Actions akan membuat release dan mengupdate `package_nuc140_index.json`.
4. Di Arduino IDE, buka Board Manager, refresh/restart, lalu install versi 0.1.1.
5. URL Boards Manager tetap:

```
https://raw.githubusercontent.com/subairi-git/Arduino-NUC140/main/package_nuc140_index.json
```

Jangan membuat release v0.1.1 manual sebelum workflow selesai.
Versi 0.1.1 ini masih eksperimen compile-only, upload Nu-Link belum tersedia.
