#include <iostream>
#include <string>
#include <limits>
#include <vector>
#include <ctime>
using namespace std;

// ===== Utility =====
string formatRupiah(int angka) {
    string s = to_string(angka), hasil = "";
    int hitung = 0;
    for (int i = (int)s.size() - 1; i >= 0; i--) {
        hasil = s[i] + hasil;
        hitung++;
        if (hitung % 3 == 0 && i != 0) hasil = '.' + hasil;
    }
    return hasil;
}

bool readInt(const string& prompt, int &out) {
    cout << prompt;
    if (!(cin >> out)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Input tidak valid.\n";
        return false;
    }
    return true;
}

string getWaktuSekarang() {
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char buffer[50];
    strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", ltm);
    return string(buffer);
}

// ===== Struktur Rekening =====
struct Rekening {
    string jenis;
    int saldo;
    int totalTarikHarian = 0;
    int totalTransferHarian = 0;
};

const int LIMIT_TARIK_HARIAN = 5000000;
const int LIMIT_TRANSFER_HARIAN = 10000000;

// ===== Riwayat =====
vector<string> riwayat;
string transaksiTerakhir = "Belum ada transaksi.";

void tambahRiwayat(const string& teks) {
    if (riwayat.size() >= 5) riwayat.erase(riwayat.begin());
    riwayat.push_back(teks);
    transaksiTerakhir = teks;
}

// ===== Pilih Rekening =====
Rekening* pilihRekening(Rekening &tabungan, Rekening &giro) {
    int pilih;
    cout << "\nPilih rekening:\n1. Tabungan\n2. Giro\nPilihan: ";
    if (!(cin >> pilih)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); return nullptr; }
    if (pilih == 1) return &tabungan;
    if (pilih == 2) return &giro;
    cout << "Pilihan tidak tersedia.\n"; return nullptr;
}

// ===== Transaksi =====
void cekSaldo(Rekening &rek) {
    cout << "\nSaldo " << rek.jenis << ": Rp" << formatRupiah(rek.saldo) << "\n\n";
}

void tarikTunai(Rekening &rek) {
    int pilihan, jumlah = 0;
    cout << "\n=== TARIK TUNAI (" << rek.jenis << ") ===\n";
    cout << "1. Rp50.000\n2. Rp100.000\n3. Rp200.000\n4. Rp500.000\n5. Nominal lain\n";
    cout << "Pilihan: ";
    if (!(cin >> pilihan)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); return; }

    if (pilihan == 1) jumlah = 50000;
    else if (pilihan == 2) jumlah = 100000;
    else if (pilihan == 3) jumlah = 200000;
    else if (pilihan == 4) jumlah = 500000;
    else if (pilihan == 5) {
        if (!readInt("Masukkan nominal: ", jumlah)) return;
        if (jumlah % 50000 != 0) { cout << "Harus kelipatan Rp50.000.\n"; return; }
    } else { cout << "Pilihan tidak tersedia.\n"; return; }

    if (rek.totalTarikHarian + jumlah > LIMIT_TARIK_HARIAN) { cout << "Limit tarik harian tercapai.\n"; return; }
    if (jumlah > rek.saldo - 50000) { cout << "Saldo tidak cukup (min Rp50.000 harus tersisa).\n"; return; }

    rek.saldo -= jumlah;
    rek.totalTarikHarian += jumlah;
    cout << "Tarik tunai berhasil. Saldo " << rek.jenis << ": Rp" << formatRupiah(rek.saldo) << "\n\n";
    tambahRiwayat("Tarik " + rek.jenis + " Rp" + formatRupiah(jumlah) + " | Saldo: Rp" + formatRupiah(rek.saldo) + " | " + getWaktuSekarang());
}

void setorTunai(Rekening &rek) {
    int jumlah;
    if (!readInt("Masukkan jumlah setor: ", jumlah)) return;
    if (jumlah % 50000 != 0) { cout << "Harus kelipatan Rp50.000.\n"; return; }
    if (jumlah > 5000000) { cout << "Maksimal setor Rp5.000.000.\n"; return; }

    rek.saldo += jumlah;
    cout << "Setor berhasil. Saldo " << rek.jenis << ": Rp" << formatRupiah(rek.saldo) << "\n\n";
    tambahRiwayat("Setor " + rek.jenis + " Rp" + formatRupiah(jumlah) + " | Saldo: Rp" + formatRupiah(rek.saldo) + " | " + getWaktuSekarang());
}

void transfer(Rekening &rek) {
    string rekeningTujuan; int jumlah;
    const int biayaAdmin = 6500;
    cout << "Masukkan no rekening tujuan: "; cin >> rekeningTujuan;
    if (!readInt("Masukkan jumlah transfer: ", jumlah)) return;
    if (jumlah % 50000 != 0) { cout << "Harus kelipatan Rp50.000.\n"; return; }
    if (rek.totalTransferHarian + jumlah > LIMIT_TRANSFER_HARIAN) { cout << "Limit transfer harian tercapai.\n"; return; }

    int totalDebit = jumlah + biayaAdmin;
    if (totalDebit > rek.saldo - 50000) { cout << "Saldo tidak cukup.\n"; return; }

    rek.saldo -= totalDebit;
    rek.totalTransferHarian += jumlah;
    cout << "Transfer Rp" << formatRupiah(jumlah) << " dari " << rek.jenis << " ke " << rekeningTujuan << " berhasil.\n";
    cout << "Biaya admin Rp" << formatRupiah(biayaAdmin) << ". Saldo: Rp" << formatRupiah(rek.saldo) << "\n\n";
    tambahRiwayat("Transfer " + rek.jenis + " Rp" + formatRupiah(jumlah) + " ke " + rekeningTujuan + " | Saldo: Rp" + formatRupiah(rek.saldo) + " | " + getWaktuSekarang());
}

void pembayaran(Rekening &rek) {
    int pilihan, jumlah; string nomor;
    cout << "\n=== PEMBAYARAN (" << rek.jenis << ") ===\n";
    cout << "1. PLN\n2. Pulsa HP\n3. Internet\nPilihan: "; cin >> pilihan;
    if (pilihan == 1) cout << "ID PLN: ";
    else if (pilihan == 2) cout << "Nomor HP: ";
    else if (pilihan == 3) cout << "ID Internet: ";
    else { cout << "Pilihan tidak ada.\n"; return; }
    cin >> nomor;

    if (!readInt("Masukkan jumlah: ", jumlah)) return;
    if (jumlah % 50000 != 0) { cout << "Harus kelipatan Rp50.000.\n"; return; }
    if (jumlah > rek.saldo - 50000) { cout << "Saldo tidak cukup.\n"; return; }

    rek.saldo -= jumlah;
    string jenis = (pilihan == 1 ? "PLN" : (pilihan == 2 ? "Pulsa" : "Internet"));
    cout << "Pembayaran " << jenis << " Rp" << formatRupiah(jumlah) << " berhasil.\n";
    cout << "Saldo " << rek.jenis << ": Rp" << formatRupiah(rek.saldo) << "\n\n";
    tambahRiwayat("Bayar " + jenis + " dari " + rek.jenis + " Rp" + formatRupiah(jumlah) + " | Saldo: Rp" + formatRupiah(rek.saldo) + " | " + getWaktuSekarang());
}

// ===== Akhir Bulan =====
void prosesAkhirBulan(Rekening &tabungan, Rekening &giro) {
    double bunga = tabungan.saldo * 0.005; // 0.5%
    int admin = 10000; // biaya giro

    tabungan.saldo += (int)bunga;
    giro.saldo -= admin;

    cout << "\n=== PROSES AKHIR BULAN ===\n";
    cout << "Tabungan dapat bunga Rp" << formatRupiah((int)bunga) << ". Saldo: Rp" << formatRupiah(tabungan.saldo) << "\n";
    cout << "Giro kena biaya admin Rp" << formatRupiah(admin) << ". Saldo: Rp" << formatRupiah(giro.saldo) << "\n\n";

    tambahRiwayat("Bunga Tabungan Rp" + formatRupiah((int)bunga) + " | Saldo: Rp" + formatRupiah(tabungan.saldo) + " | " + getWaktuSekarang());
    tambahRiwayat("Biaya Admin Giro Rp" + formatRupiah(admin) + " | Saldo: Rp" + formatRupiah(giro.saldo) + " | " + getWaktuSekarang());
}

// ===== Riwayat & Cetak =====
void lihatRiwayat() {
    cout << "\n=== RIWAYAT TRANSAKSI ===\n";
    if (riwayat.empty()) cout << "Belum ada transaksi.\n";
    else for (auto &r : riwayat) cout << "- " << r << "\n";
    cout << "\n";
}

void cetakStruk() {
    cout << "\n========== STRUK ATM ==========\n";
    cout << transaksiTerakhir << "\nDicetak: " << getWaktuSekarang() << "\n===============================\n\n";
}

// ===== Menu =====
void tampilkanMenu() {
    cout << "=== MENU ATM ===\n"
         << "1. Cek Saldo\n2. Tarik Tunai\n3. Setor Tunai\n4. Transfer\n5. Pembayaran\n6. Riwayat\n7. Cetak Struk\n8. Proses Akhir Bulan\n9. Keluar\nPilihan: ";
}

int main() {
    Rekening tabungan = {"Tabungan", 1000000};
    Rekening giro = {"Giro", 5000000};
    string pin = "1234", inputPin; int pilihan;

    cout << "===== SELAMAT DATANG DI ATM =====\n";
    for (int i = 0; i < 3; i++) {
        cout << "Masukkan PIN: "; cin >> inputPin;
        if (inputPin == pin) break;
        else if (i == 2) { cout << "PIN salah 3x. Kartu diblokir.\n"; return 0; }
        else cout << "PIN salah.\n";
    }

    do {
        tampilkanMenu();
        if (!readInt("", pilihan)) continue;
        Rekening* rek = nullptr;
        if (pilihan >= 1 && pilihan <= 5) rek = pilihRekening(tabungan, giro);
        if (rek == nullptr && pilihan >= 1 && pilihan <= 5) continue;

        switch (pilihan) {
            case 1: cekSaldo(*rek); break;
            case 2: tarikTunai(*rek); break;
            case 3: setorTunai(*rek); break;
            case 4: transfer(*rek); break;
            case 5: pembayaran(*rek); break;
            case 6: lihatRiwayat(); break;
            case 7: cetakStruk(); break;
            case 8: prosesAkhirBulan(tabungan, giro); break;
            case 9: cout << "Terima kasih. Sampai jumpa!\n"; break;
            default: cout << "Pilihan tidak ada.\n\n"; break;
        }
    } while (pilihan != 9);

    return 0;
}
