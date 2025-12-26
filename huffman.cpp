#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <queue>
#include <cmath>
#include <iomanip>
#include <bitset>
using namespace std;

struct Uzel {
    char simvol;
    int chastota;
    Uzel* levyj;
    Uzel* pravyj;

    Uzel(char s, int f) : simvol(s), chastota(f), levyj(nullptr), pravyj(nullptr) {}
    Uzel(int f, Uzel* l, Uzel* r) : simvol('\0'), chastota(f), levyj(l), pravyj(r) {}
};

struct Sravnenie {
    bool operator()(Uzel* a, Uzel* b) {
        return a->chastota > b->chastota;
    }
};

map<char, string> kodirovki;
map<string, char> dekodirovki;

void sozdatKody(Uzel* vershina, string kod) {
    if (!vershina) return;

    if (vershina->simvol != '\0') {
        kodirovki[vershina->simvol] = kod;
        dekodirovki[kod] = vershina->simvol;
        return;
    }

    sozdatKody(vershina->levyj, kod + "0");
    sozdatKody(vershina->pravyj, kod + "1");
}

void ochistitPamyat(Uzel* uzel) {
    if (!uzel) return;
    ochistitPamyat(uzel->levyj);
    ochistitPamyat(uzel->pravyj);
    delete uzel;
}

// Изменение. Сохранение частоты вместо кода
void saveChastoti(const map<char, int>& freq, ofstream& fout) {
    int size = freq.size();
    fout.write((char*)&size, sizeof(int));  // Количество символов

    for (auto& p : freq) {
        fout.put(p.first);                   // Символ
        fout.write((char*)&p.second, sizeof(int));  // Частота
    }
}

// Изменение. Будем теперь читать частоты из файла
map<char, int> loadChastoti(ifstream& fin) {
    map<char, int> freq;
    int size;
    fin.read((char*)&size, sizeof(int));

    for (int i = 0; i < size; i++) {
        char c;
        int f;
        fin.get(c);
        fin.read((char*)&f, sizeof(int));
        freq[c] = f;
    }

    return freq;
}

void kodirovatFile(string inputFile, string outputFile) {
    ifstream fin(inputFile);
    if (!fin) {
        cout << "Oshibka: ne udalos otkryt fail " << inputFile << endl;
        return;
    }

    string text((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
    fin.close();

    if (text.empty()) {
        cout << "Fail pustoy!" << endl;
        return;
    }

    cout << "Kodiruem fail: " << inputFile << endl;
    cout << "Dlina teksta: " << text.length() << " simvolov" << endl;

    map<char, int> chastoty;
    for (char c : text) {
        chastoty[c]++;
    }

    priority_queue<Uzel*, vector<Uzel*>, Sravnenie> ochered;

    for (auto& para : chastoty) {
        ochered.push(new Uzel(para.first, para.second));
    }

    while (ochered.size() > 1) {
        Uzel* levyj = ochered.top(); ochered.pop();
        Uzel* pravyj = ochered.top(); ochered.pop();
        ochered.push(new Uzel(levyj->chastota + pravyj->chastota, levyj, pravyj));
    }

    Uzel* koren = ochered.top();

    kodirovki.clear();
    dekodirovki.clear();
    sozdatKody(koren, "");

    string zakodirovanniyText = "";
    for (char c : text) {
        zakodirovanniyText += kodirovki[c];
    }

    ofstream fout(outputFile, ios::binary);
    if (!fout) {
        cout << "Oshibka: ne udalos sozdat fail " << outputFile << endl;
        return;
    }

    // Изменение: Новый формат
    fout << "HUFFMAN2" << endl;
    fout << text.length() << endl;

    // Изменение: Сохраняем частоты вместо кодов
    saveChastoti(chastoty, fout);

    fout << "CODED" << endl;

    int bitCount = zakodirovanniyText.length();
    fout << bitCount << endl;

    for (size_t i = 0; i < zakodirovanniyText.length(); i += 8) {
        string byteStr = zakodirovanniyText.substr(i, 8);
        while (byteStr.length() < 8) byteStr += "0";
        bitset<8> bits(byteStr);
        fout.put(static_cast<char>(bits.to_ulong()));
    }

    fout.close();

    int originalSize = text.length() * 8;
    int compressedSize = bitCount;
    double compressionRatio = (double)originalSize / compressedSize;

    cout << "\nRezultaty szhatiya:" << endl;
    cout << "Ishodnyy razmer: " << originalSize << " bit" << endl;
    cout << "Szhatyy razmer: " << compressedSize << " bit" << endl;
    cout << "Koeffitsient szhatiya: " << fixed << setprecision(2) << compressionRatio << ":1" << endl;
    cout << "Szhatyy fail sohranen kak: " << outputFile << endl;

    ochistitPamyat(koren);
}

void dekodirovatFile(string inputFile, string outputFile) {
    ifstream fin(inputFile, ios::binary);
    if (!fin) {
        cout << "Oshibka: ne udalos otkryt fail " << inputFile << endl;
        return;
    }

    string magic;
    fin >> magic;

    if (magic != "HUFFMAN2") {
        cout << "Oshibka: eto ne huffman-file novogo formata!" << endl;
        return;
    }

    int originalLength;
    fin >> originalLength;
    fin.ignore();

    // Изменение: читаем таблицу частот
    map<char, int> chastoty = loadChastoti(fin);

    // Изменение: строим дерево Хаффмана из частот
    priority_queue<Uzel*, vector<Uzel*>, Sravnenie> ochered;
    for (auto& para : chastoty) {
        ochered.push(new Uzel(para.first, para.second));
    }
    while (ochered.size() > 1) {
        Uzel* levyj = ochered.top(); ochered.pop();
        Uzel* pravyj = ochered.top(); ochered.pop();
        ochered.push(new Uzel(levyj->chastota + pravyj->chastota, levyj, pravyj));
    }
    Uzel* koren = ochered.top();

    string line;
    getline(fin, line);

    int bitCount;
    fin >> bitCount;
    fin.ignore();

    // Изменение. Декодируем напрямую из дерева
    ofstream fout(outputFile);
    if (!fout) {
        cout << "Oshibka: ne udalos sozdat fail " << outputFile << endl;
        ochistitPamyat(koren);
        return;
    }

    Uzel* current = koren;
    char byte;
    int bitsProcessed = 0;
    int charsDecoded = 0;

    while (bitsProcessed < bitCount && fin.get(byte)) {
        for (int i = 7; i >= 0 && bitsProcessed < bitCount; i--) {
            bool bit = (byte >> i) & 1;

            if (bit == 0) {
                current = current->levyj;
            }
            else {
                current = current->pravyj;
            }

            bitsProcessed++;

            if (current->levyj == nullptr && current->pravyj == nullptr) {
                fout.put(current->simvol);
                charsDecoded++;
                current = koren;

                if (charsDecoded >= originalLength) {
                    break;
                }
            }
        }
    }

    fout.close();

    cout << "Dekodirovanie vipolneno!" << endl;
    cout << "Rasshifrovannyy text sohranen v: " << outputFile << endl;
    cout << "Proverka: " << (charsDecoded == originalLength ? "OK" : "ERROR") << endl;

    ochistitPamyat(koren);
}

void pokazatMenu() {
    cout << "    ALGORITM HAFMANA - LABORATORNAYA (NOVYY FORMAT)" << endl;
    cout << "1. Kodirovat fail" << endl;
    cout << "2. Dekodirovat fail" << endl;
    cout << "3. Vihod" << endl;
    cout << "Viberite deystvie (1-3): ";
}

int main() {
    int vibor;

    do {
        pokazatMenu();
        cin >> vibor;
        cin.ignore();

        if (vibor == 1) {
            string inputFile, outputFile;
            cout << "Vvedite imya vhodnogo faila (txt): ";
            getline(cin, inputFile);
            cout << "Vvedite imya vihodnogo faila: ";
            getline(cin, outputFile);
            kodirovatFile(inputFile, outputFile);
        }
        else if (vibor == 2) {
            string inputFile, outputFile;
            cout << "Vvedite imya szhatogo faila: ";
            getline(cin, inputFile);
            cout << "Vvedite imya rasshifrovannogo faila: ";
            getline(cin, outputFile);
            dekodirovatFile(inputFile, outputFile);
        }
        else if (vibor == 3) {
            cout << "Vihod..." << endl;
        }
        else {
            cout << "Nekorrektniy vibor!" << endl;
        }

        cout << endl;
    } while (vibor != 3);

    return 0;
}