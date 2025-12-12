#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <queue>
#include <cmath>
#include <iomanip>
using namespace std;

struct Uzel {
    char simvol;
    int chastota;
    Uzel* levyj;
    Uzel* pravyj;
};

bool sravnenie(Uzel* a, Uzel* b) {
    return a->chastota > b->chastota;
}

map<char, string> kodirovki;

void sozdatKody(Uzel* vershina, string kod) {
    if (vershina == NULL) return;

    if (vershina->simvol != 0) {
        kodirovki[vershina->simvol] = kod;
        return;
    }

    sozdatKody(vershina->levyj, kod + "0");
    sozdatKody(vershina->pravyj, kod + "1");
}

void ochistitPamyat(Uzel* uzel) {
    if (uzel == NULL) return;
    ochistitPamyat(uzel->levyj);
    ochistitPamyat(uzel->pravyj);
    delete uzel;
}

int main() {
    cout << "==========================================" << endl;
    cout << "Labaratornaya: Algoritm Hafmana" << endl;
    cout << "==========================================" << endl << endl;

    string tekst = "KOV.KOROVA";

    cout << "Tekst: " << tekst << endl;
    cout << "Dlina: " << tekst.size() << " simvolov" << endl << endl;

    map<char, int> chastoty;
    for (int i = 0; i < tekst.length(); i++) {
        chastoty[tekst[i]]++;
    }

    cout << "1. Chastoty simvolov:" << endl;
    cout << "---------------------" << endl;

    int vsego = tekst.length();
    for (auto it = chastoty.begin(); it != chastoty.end(); it++) {
        double veroyatnost = (double)it->second / vsego;
        cout << "'" << it->first << "' - " << it->second
            << " raz (" << fixed << setprecision(2) << veroyatnost * 100 << "%)" << endl;
    }
    cout << endl;

    priority_queue<Uzel*, vector<Uzel*>, decltype(&sravnenie)> ochered(sravnenie);

    for (auto para : chastoty) {
        Uzel* novyj = new Uzel;
        novyj->simvol = para.first;
        novyj->chastota = para.second;
        novyj->levyj = NULL;
        novyj->pravyj = NULL;
        ochered.push(novyj);
    }

    cout << "2. Stroim derevo:" << endl;
    cout << "-----------------" << endl;

    int shag = 1;
    while (ochered.size() > 1) {
        Uzel* levyj = ochered.top(); ochered.pop();
        Uzel* pravyj = ochered.top(); ochered.pop();

        Uzel* roditel = new Uzel;
        roditel->simvol = 0;
        roditel->chastota = levyj->chastota + pravyj->chastota;
        roditel->levyj = levyj;
        roditel->pravyj = pravyj;

        ochered.push(roditel);

        cout << "Shag " << shag << ": " << levyj->chastota << "+"
            << pravyj->chastota << "=" << roditel->chastota << endl;
        shag++;
    }

    Uzel* koren = ochered.top();
    cout << "Gotovo! Koren: " << koren->chastota << endl << endl;

    sozdatKody(koren, "");

    cout << "3. Tablica kodov:" << endl;
    cout << "-----------------" << endl;

    for (auto& para : kodirovki) {
        cout << "'" << para.first << "' -> " << para.second << endl;
    }
    cout << endl;

    cout << "4. Szhatie:" << endl;
    cout << "-----------" << endl;

    int bylo = vsego * 8;
    int stalo = 0;

    for (int i = 0; i < tekst.length(); i++) {
        char c = tekst[i];
        stalo += kodirovki[c].length();
    }

    cout << "Bylo: " << bylo << " bit" << endl;
    cout << "Stalo: " << stalo << " bit" << endl;

    double koefficient = (double)bylo / stalo;
    cout << "V " << fixed << setprecision(1) << koefficient << " raza menshe" << endl << endl;

    cout << "5. Teoriya (entropiya):" << endl;
    cout << "-----------------------" << endl;

    double H = 0;
    for (auto& para : chastoty) {
        double p = (double)para.second / vsego;
        if (p > 0) {
            H -= p * log(p) / log(2);
        }
    }

    double L = (double)stalo / vsego;

    cout << "Entropiya H = " << fixed << setprecision(3) << H << endl;
    cout << "Srednyaya dlina L = " << L << endl;

    if (H <= L && L <= H + 1) {
        cout << "Teorema Shennona rabotaet" << endl;
    }
    cout << endl;

    cout << "6. Primer kodirovaniya:" << endl;
    cout << "-----------------------" << endl;

    cout << tekst << " = ";
    for (char c : tekst) {
        cout << kodirovki[c] << " ";
    }
    cout << endl << endl;

    cout << "7. Proverka:" << endl;
    cout << "-----------" << endl;

    bool prefiksnij = true;
    vector<string> vseKody;
    for (auto& para : kodirovki) {
        vseKody.push_back(para.second);
    }

    for (int i = 0; i < vseKody.size(); i++) {
        for (int j = i + 1; j < vseKody.size(); j++) {
            if (vseKody[j].compare(0, vseKody[i].length(), vseKody[i]) == 0) {
                prefiksnij = false;
            }
        }
    }

    cout << "Kod prefiksnij: " << (prefiksnij ? "da" : "net") << endl;

    double summaKrafta = 0;
    for (auto& kod : vseKody) {
        summaKrafta += 1.0 / (1 << kod.length());
    }

    cout << "Summa Krafta: " << summaKrafta << (summaKrafta <= 1 ? " <= 1" : " > 1") << endl;
    cout << endl;

    ochistitPamyat(koren);

    cout << "==========================================" << endl;
    cout << "Konec" << endl;
    cout << "==========================================" << endl;

    return 0;
}