#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <stack>
#include <string>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <fstream>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

// Definicje kolorów i wartości kart
enum KolorKarty { KIER, KARO, PIK, TREFL };
enum WartoscKarty { AS = 1, DWA, TRZY, CZTERY, PIEC, SZESC, SIEDEM, OSIEM, DZIEWIEC, DZIESIEC, WALET, DAMA, KROL };
string wynik_txt = "wyniki_solitaire.txt";

// Struktura karty
struct Karta {
    KolorKarty Kolor;
    WartoscKarty Wartosc;
    bool JestOdkryta;

    // Konstruktor domyślny
    Karta() : Kolor(KIER), Wartosc(AS), JestOdkryta(false) {}

    // Konstruktor z parametrami
    Karta(KolorKarty kolor, WartoscKarty wartosc, bool jestOdkryta = false)
        : Kolor(kolor), Wartosc(wartosc), JestOdkryta(jestOdkryta) {}
};

// Klasa przechowująca ruch do cofania
struct Ruch {
    int ZrodloTyp;  // 0 - kolumna gry, 1 - stos rezerwowy, 2 - stos końcowy
    int ZrodloIndex;
    int CelTyp;    // 0 - kolumna gry, 1 - stos końcowy
    int CelIndex;
    int LiczbaKart;

    Ruch(int zrodloTyp, int zrodloIndex, int celTyp, int celIndex, int liczbaKart)
        : ZrodloTyp(zrodloTyp), ZrodloIndex(zrodloIndex), CelTyp(celTyp), CelIndex(celIndex), LiczbaKart(liczbaKart) {}
};

// Klasa gry Pasjans
class Pasjans {
private:
    vector<vector<Karta>> KolumnyGry;
    vector<vector<Karta>> StosyKoncowe;
    deque<Karta> StosRezerwowy;
    vector<Karta> StosWidoczny;
    int PoziomTrudnosci;  // 1 - łatwy (1 karta), 2 - trudny (3 karty)
    stack<Ruch> HistoriaRuchow;
    int LiczbaRuchow;

    // Inicjalizacja kolorów w konsoli
    void InicjalizujKolory() {
        #ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
        GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
        SetConsoleTextAttribute(hConsole, consoleInfo.wAttributes);
        #endif
    }

    // Wyświetlanie karty z odpowiednim kolorem
    void WyswietlKarte(const Karta& karta) {
        if (!karta.JestOdkryta) {
            cout << "[XX]";
            return;
        }

        string symbolWartosci;
        switch (karta.Wartosc) {
        case AS: symbolWartosci = "A"; break;
        case WALET: symbolWartosci = "J"; break;
        case DAMA: symbolWartosci = "Q"; break;
        case KROL: symbolWartosci = "K"; break;
        default: symbolWartosci = to_string(static_cast<int>(karta.Wartosc)); break;
        }

        string symbolKoloru;
        bool JestCzerwony = false;

        switch (karta.Kolor) {
        case KIER:
            symbolKoloru = u8"♥";
            JestCzerwony = true;
            break;
        case KARO:
            symbolKoloru = u8"♦";
            JestCzerwony = true;
            break;
        case PIK: symbolKoloru = u8"♠"; break;
        case TREFL: symbolKoloru = u8"♣"; break;
        }

        #ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
        GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
        WORD normalColor = consoleInfo.wAttributes;

        if (JestCzerwony) {
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
        }
        #else
        if (JestCzerwony) {
            cout << "\033[31m";
        }
        #endif

        if (karta.Wartosc == DZIESIEC) {
            cout << "[" << symbolWartosci << symbolKoloru << "]";
        } else {
            cout << "[" << symbolWartosci << " " << symbolKoloru << "]";
        }

        #ifdef _WIN32
        if (JestCzerwony) {
            SetConsoleTextAttribute(hConsole, normalColor);
        }
        #else
        if (JestCzerwony) {
            cout << "\033[0m";
        }
        #endif
    }

    // Sprawdzanie czy karty można położyć na sobie w kolumnach gry
    bool MoznaPolozyc(const Karta& kartaGorna, const Karta& kartaDolna) {
        bool PrzeciwnyKolor = ((kartaGorna.Kolor == KIER || kartaGorna.Kolor == KARO) &&
                           (kartaDolna.Kolor == PIK || kartaDolna.Kolor == TREFL)) ||
                          ((kartaGorna.Kolor == PIK || kartaGorna.Kolor == TREFL) &&
                           (kartaDolna.Kolor == KIER || kartaDolna.Kolor == KARO));

        return (kartaGorna.Wartosc == kartaDolna.Wartosc + 1) && PrzeciwnyKolor;
    }

    // Sprawdzanie czy można położyć kartę na stosie końcowym
    bool MoznaPolozycNaStosieKoncowym(const Karta& karta, const vector<Karta>& stos) {
        if (stos.empty()) {
            return karta.Wartosc == AS;
        }
        return (karta.Kolor == stos.back().Kolor) && (karta.Wartosc == stos.back().Wartosc + 1);
    }

    // Zapis wyniku do pliku
    void ZapiszWynik() {
        ofstream plik(wynik_txt, ios::app);
        if (!plik.is_open()) {
            cout << "Nie można otworzyć pliku wyników!\n";
            return;
        }

        time_t teraz = time(nullptr);
        tm* czasLokalny = localtime(&teraz);

        ostringstream strumien;
        strumien << put_time(czasLokalny, "%Y-%m-%d") << ", "
                 << put_time(czasLokalny, "%H:%M:%S") << ", "
                 << (PoziomTrudnosci == 1 ? "Latwy" : "Trudny") << ", "
                 << LiczbaRuchow;

        plik << strumien.str() << "\n";
        plik.close();
    }

public:
    Pasjans(int poziomTrudnosci = 1) : PoziomTrudnosci(poziomTrudnosci), LiczbaRuchow(0) {
        InicjalizujKolory();
        InicjalizujGre();
    }

    // Inicjalizacja gry - tasowanie kart i rozłożenie początkowego układu
    void InicjalizujGre() {
        KolumnyGry.clear();
        StosyKoncowe.clear();
        StosRezerwowy.clear();
        StosWidoczny.clear();
        while (!HistoriaRuchow.empty()) {
            HistoriaRuchow.pop();
        }
        LiczbaRuchow = 0;

        vector<Karta> Talia;
        for (int kolor = KIER; kolor <= TREFL; kolor++) {
            for (int wartosc = AS; wartosc <= KROL; wartosc++) {
                Talia.push_back(Karta(static_cast<KolorKarty>(kolor), static_cast<WartoscKarty>(wartosc)));
            }
        }

        random_device rd;
        mt19937 g(rd());
        shuffle(Talia.begin(), Talia.end(), g);

        KolumnyGry.resize(7);
        int kartaIndex = 0;

        for (int i = 0; i < 7; i++) {
            KolumnyGry[i].resize(i + 1);
            for (int j = 0; j < i + 1; j++) {
                KolumnyGry[i][j] = Talia[kartaIndex++];
                KolumnyGry[i][j].JestOdkryta = (j == i);
            }
        }

        StosyKoncowe.resize(4);

        for (size_t i = kartaIndex; i < Talia.size(); i++) {
            StosRezerwowy.push_back(Talia[i]);
        }
    }

    // Wyświetlanie aktualnego stanu gry
    void WyswietlGre() {
        system("cls");

        cout << "STOSY KOŃCOWE:\n";
        for (size_t i = 0; i < StosyKoncowe.size(); i++) {
            cout << i + 1 << ": ";
            if (StosyKoncowe[i].empty()) {
                cout << "[  ]";
            } else {
                WyswietlKarte(StosyKoncowe[i].back());
            }
            cout << "  ";
        }
        cout << "\n\n";

        cout << "STOS REZERWOWY: ";
        if (StosRezerwowy.empty()) {
            cout << "[  ]";
        } else {
            cout << "[XX] (" << StosRezerwowy.size() << ")";
        }
        cout << "  WIDOCZNE: ";

        if (StosWidoczny.empty()) {
            cout << "[  ]";
        } else {
            for (size_t i = 0; i < min(static_cast<size_t>(PoziomTrudnosci == 1 ? 1 : 3), StosWidoczny.size()); i++) {
                WyswietlKarte(StosWidoczny[StosWidoczny.size() - 1 - i]);
            }
        }
        cout << "\n\n";

        cout << "KOLUMNY GRY:\n";
        size_t maxDlugosc = 0;
        for (const auto& kolumna : KolumnyGry) {
            maxDlugosc = max(maxDlugosc, kolumna.size());
        }

        cout << "    ";
        for (size_t i = 0; i < KolumnyGry.size(); i++) {
            cout << i + 1 << "      ";
        }
        cout << "\n";

        for (size_t wiersz = 0; wiersz < maxDlugosc; wiersz++) {
            cout << "    ";
            for (size_t kolumna = 0; kolumna < KolumnyGry.size(); kolumna++) {
                if (wiersz < KolumnyGry[kolumna].size()) {
                    WyswietlKarte(KolumnyGry[kolumna][wiersz]);
                } else {
                    cout << "     ";
                }
                cout << "  ";
            }
            cout << "\n";
        }
        cout << "\n";
    }

    // Dobieranie kart ze stosu rezerwowego
    void DobierzKarty() {
        if (StosRezerwowy.empty() && StosWidoczny.empty()) {
            return;
        }

        if (StosRezerwowy.empty()) {
            while (!StosWidoczny.empty()) {
                StosRezerwowy.push_front(StosWidoczny.back());
                StosWidoczny.pop_back();
            }
            return;
        }

        int ileKart = PoziomTrudnosci == 1 ? 1 : 3;
        for (int i = 0; i < ileKart && !StosRezerwowy.empty(); i++) {
            Karta karta = StosRezerwowy.front();
            StosRezerwowy.pop_front();
            karta.JestOdkryta = true;
            StosWidoczny.push_back(karta);
        }
    }

    // Przenoszenie karty z jednej kolumny do drugiej
    bool PrzeniesKartyMiedzyKolumnami(int zKolumny, int doKolumny, int ileKart) {
        if (zKolumny < 1 || zKolumny > 7 || doKolumny < 1 || doKolumny > 7) {
            return false;
        }

        zKolumny--;
        doKolumny--;

        if (KolumnyGry[zKolumny].empty() ||
            KolumnyGry[zKolumny].size() < ileKart ||
            ileKart <= 0) {
            return false;
        }

        int pierwszaKartaIndex = KolumnyGry[zKolumny].size() - ileKart;
        if (!KolumnyGry[zKolumny][pierwszaKartaIndex].JestOdkryta) {
            return false;
        }

        for (size_t i = pierwszaKartaIndex; i < KolumnyGry[zKolumny].size() - 1; i++) {
            if (!MoznaPolozyc(KolumnyGry[zKolumny][i], KolumnyGry[zKolumny][i + 1])) {
                return false;
            }
        }

        if (KolumnyGry[doKolumny].empty()) {
            if (KolumnyGry[zKolumny][pierwszaKartaIndex].Wartosc != KROL) {
                return false;
            }
        } else {
            if (!MoznaPolozyc(KolumnyGry[doKolumny].back(), KolumnyGry[zKolumny][pierwszaKartaIndex])) {
                return false;
            }
        }

        HistoriaRuchow.push(Ruch(0, zKolumny, 0, doKolumny, ileKart));

        vector<Karta> kartyDoPrzeniesienia;
        for (size_t i = KolumnyGry[zKolumny].size() - ileKart; i < KolumnyGry[zKolumny].size(); i++) {
            kartyDoPrzeniesienia.push_back(KolumnyGry[zKolumny][i]);
        }

        KolumnyGry[zKolumny].resize(KolumnyGry[zKolumny].size() - ileKart);

        if (!KolumnyGry[zKolumny].empty() && !KolumnyGry[zKolumny].back().JestOdkryta) {
            KolumnyGry[zKolumny].back().JestOdkryta = true;
        }

        for (const auto& karta : kartyDoPrzeniesienia) {
            KolumnyGry[doKolumny].push_back(karta);
        }

        LiczbaRuchow++;
        return true;
    }

    // Przenoszenie karty na stos końcowy
    bool PrzeniesKarteNaStosKoncowy(int zrodloTyp, int zrodloIndex) {
        Karta* kartaDoPrezeniesienia = nullptr;
        int stosKoncowyIndex = -1;

        if (zrodloTyp == 0) {
            if (zrodloIndex < 1 || zrodloIndex > 7 || KolumnyGry[zrodloIndex - 1].empty()) {
                return false;
            }
            kartaDoPrezeniesienia = &KolumnyGry[zrodloIndex - 1].back();
        } else if (zrodloTyp == 1) {
            if (StosWidoczny.empty()) {
                return false;
            }
            kartaDoPrezeniesienia = &StosWidoczny.back();
        } else {
            return false;
        }

        for (size_t i = 0; i < StosyKoncowe.size(); i++) {
            if (MoznaPolozycNaStosieKoncowym(*kartaDoPrezeniesienia, StosyKoncowe[i])) {
                stosKoncowyIndex = i;
                break;
            }
        }

        if (stosKoncowyIndex == -1 && kartaDoPrezeniesienia->Wartosc == AS) {
            for (size_t i = 0; i < StosyKoncowe.size(); i++) {
                if (StosyKoncowe[i].empty()) {
                    stosKoncowyIndex = i;
                    break;
                }
            }
        }

        if (stosKoncowyIndex == -1) {
            return false;
        }

        HistoriaRuchow.push(Ruch(zrodloTyp, zrodloIndex - 1, 1, stosKoncowyIndex, 1));

        StosyKoncowe[stosKoncowyIndex].push_back(*kartaDoPrezeniesienia);

        if (zrodloTyp == 0) {
            KolumnyGry[zrodloIndex - 1].pop_back();
            if (!KolumnyGry[zrodloIndex - 1].empty() && !KolumnyGry[zrodloIndex - 1].back().JestOdkryta) {
                KolumnyGry[zrodloIndex - 1].back().JestOdkryta = true;
            }
        } else {
            StosWidoczny.pop_back();
        }

        LiczbaRuchow++;
        return true;
    }

    // Przenoszenie karty ze stosu widocznego do kolumny
    bool PrzeniesKarteZeStosWidocznegoDoKolumny(int doKolumny) {
        if (doKolumny < 1 || doKolumny > 7 || StosWidoczny.empty()) {
            return false;
        }

        doKolumny--;

        if (KolumnyGry[doKolumny].empty()) {
            if (StosWidoczny.back().Wartosc != KROL) {
                return false;
            }
        } else {
            if (!MoznaPolozyc(KolumnyGry[doKolumny].back(), StosWidoczny.back())) {
                return false;
            }
        }

        HistoriaRuchow.push(Ruch(1, 0, 0, doKolumny, 1));

        KolumnyGry[doKolumny].push_back(StosWidoczny.back());
        StosWidoczny.pop_back();

        LiczbaRuchow++;
        return true;
    }

    // Cofanie ostatniego ruchu
    bool CofnijRuch() {
        if (HistoriaRuchow.empty()) {
            return false;
        }

        Ruch ostatniRuch = HistoriaRuchow.top();
        HistoriaRuchow.pop();

        if (ostatniRuch.ZrodloTyp == 0 && ostatniRuch.CelTyp == 0) {
            for (int i = 0; i < ostatniRuch.LiczbaKart; i++) {
                if (KolumnyGry[ostatniRuch.CelIndex].empty()) {
                    break;
                }
                KolumnyGry[ostatniRuch.ZrodloIndex].push_back(KolumnyGry[ostatniRuch.CelIndex].back());
                KolumnyGry[ostatniRuch.CelIndex].pop_back();
            }
            if (!KolumnyGry[ostatniRuch.CelIndex].empty() && KolumnyGry[ostatniRuch.CelIndex].back().JestOdkryta) {
                bool bylJedynaOdkryta = true;
                for (size_t i = 0; i < KolumnyGry[ostatniRuch.CelIndex].size() - 1; i++) {
                    if (KolumnyGry[ostatniRuch.CelIndex][i].JestOdkryta) {
                        bylJedynaOdkryta = false;
                        break;
                    }
                }
                if (bylJedynaOdkryta) {
                    KolumnyGry[ostatniRuch.CelIndex].back().JestOdkryta = false;
                }
            }
        } else if (ostatniRuch.ZrodloTyp == 0 && ostatniRuch.CelTyp == 1) {
            if (!StosyKoncowe[ostatniRuch.CelIndex].empty()) {
                KolumnyGry[ostatniRuch.ZrodloIndex].push_back(StosyKoncowe[ostatniRuch.CelIndex].back());
                StosyKoncowe[ostatniRuch.CelIndex].pop_back();
            }
        } else if (ostatniRuch.ZrodloTyp == 1 && ostatniRuch.CelTyp == 0) {
            if (!KolumnyGry[ostatniRuch.CelIndex].empty()) {
                StosWidoczny.push_back(KolumnyGry[ostatniRuch.CelIndex].back());
                KolumnyGry[ostatniRuch.CelIndex].pop_back();
            }
        } else if (ostatniRuch.ZrodloTyp == 1 && ostatniRuch.CelTyp == 1) {
            if (!StosyKoncowe[ostatniRuch.CelIndex].empty()) {
                StosWidoczny.push_back(StosyKoncowe[ostatniRuch.CelIndex].back());
                StosyKoncowe[ostatniRuch.CelIndex].pop_back();
            }
        }

        LiczbaRuchow--;
        return true;
    }

    // Sprawdzanie czy gra została wygrana
    bool SprawdzWygrana() {
        for (const auto& stos : StosyKoncowe) {
            if (stos.empty() || stos.back().Wartosc != KROL) {
                return false;
            }
        }
        return true;
    }

    // Główna pętla gry
    void GrajGre() {
        string Komenda;
        while (true) {
            WyswietlGre();

            if (SprawdzWygrana()) {
                ZapiszWynik(); // Zapis wyniku po wygranej
                cout << "GRATULACJE! Wygrałeś grę w " << LiczbaRuchow << " ruchów!\n";
                cout << "Wynik zapisano do pliku wyniki_solitaire.txt\n";
                cout << "Naciśnij Enter, aby zagrać ponownie, lub wpisz 'wyjdz', aby zakończyć...\n";
                getline(cin, Komenda);
                if (Komenda == "wyjdz" || Komenda == "Wyjdz" || Komenda == "WYJDZ") {
                    break;
                }
                InicjalizujGre();
                continue;
            }

            cout << "Liczba ruchów: " << LiczbaRuchow << "\n";
            cout << "Komenda (pomoc - lista komend): ";
            getline(cin, Komenda);

            if (Komenda == "pomoc" || Komenda == "Pomoc" || Komenda == "POMOC") {
                cout << "\nDostępne komendy:\n";
                cout << "kk [z] [do] [ile] - Przenieś karty między kolumnami\n";
                cout << "dk - Dobierz karty ze stosu rezerwowego\n";
                cout << "wk [kolumna] - Przenieś kartę z widocznego stosu do kolumny\n";
                cout << "s [źródło] - Przenieś kartę na stos końcowy (źródło: k[1-7] lub w)\n";
                cout << "cofnij - Cofnij ostatni ruch\n";
                cout << "restart - Rozpocznij grę od nowa\n";
                cout << "wyjdz - Zakończ grę\n";
                cout << "\nNaciśnij Enter, aby kontynuować...";
                cin.get();
            }
            else if (Komenda == "dk" || Komenda == "Dk" || Komenda == "DK") {
                DobierzKarty();
            }
            else if (Komenda == "cofnij" || Komenda == "Cofnij" || Komenda == "COFNIJ") {
                if (!CofnijRuch()) {
                    cout << "Brak ruchów do cofnięcia!\n";
                    cin.get();
                }
            }
            else if (Komenda == "restart" || Komenda == "Restart" || Komenda == "RESTART") {
                InicjalizujGre();
            }
            else if (Komenda == "wyjdz" || Komenda == "Wyjdz" || Komenda == "WYJDZ") {
                break;
            }
            else if (Komenda.substr(0, 2) == "kk" || Komenda.substr(0, 2) == "KK" || Komenda.substr(0, 2) == "Kk") {
                int zKolumny, doKolumny, ileKart;
                if (sscanf(Komenda.c_str(), "kk %d %d %d", &zKolumny, &doKolumny, &ileKart) == 3) {
                    if (!PrzeniesKartyMiedzyKolumnami(zKolumny, doKolumny, ileKart)) {
                        cout << "Nieprawidłowy ruch!\n";
                        cin.get();
                    }
                } else {
                    cout << "Nieprawidłowa składnia! Użyj: kk [z] [do] [ile]\n";
                    cin.get();
                }
            }
            else if (Komenda.substr(0, 2) == "wk" || Komenda.substr(0, 2) == "WK" || Komenda.substr(0, 2) == "Wk") {
                int doKolumny;
                if (sscanf(Komenda.c_str(), "wk %d", &doKolumny) == 1) {
                    if (!PrzeniesKarteZeStosWidocznegoDoKolumny(doKolumny)) {
                        cout << "Nieprawidłowy ruch!\n";
                        cin.get();
                    }
                } else {
                    cout << "Nieprawidłowa składnia! Użyj: wk [kolumna]\n";
                    cin.get();
                }
            }
            else if (Komenda.substr(0, 1) == "s" || Komenda.substr(0, 1) == "S") {
                char zrodlo;
                int zrodloIndex;
                if (sscanf(Komenda.c_str(), "s %c%d", &zrodlo, &zrodloIndex) == 2) {
                    if (zrodlo == 'k' || zrodlo == 'K') {
                        if (!PrzeniesKarteNaStosKoncowy(0, zrodloIndex)) {
                            cout << "Nieprawidłowy ruch!\n";
                            cin.get();
                        }
                    } else {
                        cout << "Nieprawidłowa składnia! Użyj: s k[1-7] lub s w\n";
                        cin.get();
                    }
                } else if (sscanf(Komenda.c_str(), "s %c", &zrodlo) == 1) {
                    if (zrodlo == 'w' || zrodlo == 'W') {
                        if (!PrzeniesKarteNaStosKoncowy(1, 1)) {
                            cout << "Nieprawidłowy ruch!\n";
                            cin.get();
                        }
                    } else {
                        cout << "Nieprawidłowa składnia! Użyj: s k[1-7] lub s w\n";
                        cin.get();
                    }
                } else {
                    cout << "Nieprawidłowa składnia! Użyj: s k[1-7] lub s w\n";
                    cin.get();
                }
            }
            else {
                cout << "Nieznana komenda. Wpisz 'pomoc', aby zobaczyć dostępne komendy.\n";
                cin.get();
            }
        }
    }
};

class Menu {
private:
    bool trybDebugowania; // Zmienna śledząca tryb debugowania

public:
    Menu() : trybDebugowania(false) {} // Inicjalizacja trybu debugowania na false

    void WyswietlMenu() {
        while (true) {
            system("cls");
            cout << "=========================\n";
            cout << "        PASJANS\n";
            cout << "=========================\n\n";
            cout << "1. Nowa gra (poziom łatwy - 1 karta)\n";
            cout << "2. Nowa gra (poziom trudny - 3 karty)\n";
            cout << "3. Zasady gry\n";
            cout << "4. Najlepsze wyniki\n";
            if (trybDebugowania) {
                cout << "5. Dodaj testowe wyniki\n";
                cout << "6. Usuń wszystkie wyniki\n";
                cout << "7. Wyjdź\n";
            } else {
                cout << "5. Wyjdź\n";
            }
            cout << "\nWybierz opcję: ";

            string Wybor;
            getline(cin, Wybor);

            if (Wybor == "debugstick") {
                trybDebugowania = true;
                cout << "Tryb debugowania włączony. Opcje debugowania są teraz dostępne.\n";
                cout << "Naciśnij Enter, aby kontynuować...";
                cin.get();
                continue;
            }
            else if (Wybor == "1") {
                Pasjans gra(1);
                gra.GrajGre();
            }
            else if (Wybor == "2") {
                Pasjans gra(2);
                gra.GrajGre();
            }
            else if (Wybor == "3") {
                WyswietlZasady();
            }
            else if (Wybor == "4") {
                WyswietlWyniki();
            }
            else if (Wybor == "5" && trybDebugowania) {
                GenerujTestoweWyniki();
            }
            else if (Wybor == "6" && trybDebugowania) {
                UsunWyniki();
            }
            else if ((Wybor == "5" && !trybDebugowania) || (Wybor == "7" && trybDebugowania)) {
                break;
            }
            else {
                cout << "Nieprawidłowy wybór. Naciśnij Enter, aby kontynuować...";
                cin.get();
            }
        }
    }

private:
    void WyswietlZasady() {
        system("cls");
        cout << "=========================\n";
        cout << "      ZASADY GRY\n";
        cout << "=========================\n\n";
        cout << "Cel gry:\n";
        cout << "Uporządkować wszystkie karty według koloru i wartości w czterech stosach końcowych,\n";
        cout << "od asa do króla (A-2-3-4-5-6-7-8-9-10-J-Q-K) dla każdego koloru.\n\n";

        cout << "Układ początkowy:\n";
        cout << "- 7 kolumn kart, gdzie w każdej kolumnie tylko ostatnia karta jest odkryta.\n";
        cout << "- Reszta kart trafia na stos rezerwowy.\n\n";

        cout << "Zasady przenoszenia kart:\n";
        cout << "- W kolumnach: karty układane są malejąco (K-Q-J-...-3-2-A) naprzemiennie kolorami.\n";
        cout << "- Na stosach końcowych: karty układane są rosnąco (A-2-3-...-Q-K) według kolorów.\n";
        cout << "- Na pustą kolumnę można położyć tylko króla (K).\n\n";

        cout << "Poziomy trudności:\n";
        cout << "- Łatwy: Dobiera się po 1 karcie ze stosu rezerwowego.\n";
        cout << "- Trudny: Dobiera się po 3 karty, ale użyć można tylko wierzchniej.\n\n";

        cout << "Sterowanie:\n";
        cout << "- kk [z] [do] [ile] - Przenosi [ile] kart z kolumny [z] do kolumny [do].\n";
        cout << "- dk - Dobiera karty ze stosu rezerwowego.\n";
        cout << "- wk [kolumna] - Przenosi kartę z widocznego stosu do kolumny [kolumna].\n";
        cout << "- s k[kolumna] - Przenosi kartę z kolumny [kolumna] na odpowiedni stos końcowy.\n";
        cout << "- s w - Przenosi kartę z widocznego stosu na odpowiedni stos końcowy.\n";
        cout << "- cofnij - Cofa ostatni ruch.\n";
        cout << "- restart - Rozpoczyna grę od nowa.\n";
        cout << "- wyjdz - Kończy grę.\n\n";

        cout << "Naciśnij Enter, aby wrócić do menu...";
        cin.get();
    }

    void WyswietlWyniki() {
        system("cls");
        cout << "=========================\n";
        cout << "     NAJLEPSZE WYNIKI\n";
        cout << "=========================\n\n";

        ifstream plik("wyniki_solitaire.txt");
        if (!plik.is_open()) {
            cout << "Brak wyników lub plik wyników nie istnieje.\n";
            cout << "\nNaciśnij Enter, aby wrócić do menu...";
            cin.get();
            return;
        }

        struct Wynik {
            string data;
            string godzina;
            string poziom;
            int ruchy;
        };

        vector<Wynik> wyniki;
        string linia;
        while (getline(plik, linia)) {
            stringstream ss(linia);
            string data, godzina, poziom, ruchyStr;
            getline(ss, data, ',');
            getline(ss, godzina, ',');
            getline(ss, poziom, ',');
            getline(ss, ruchyStr, ',');

            int ruchy = stoi(ruchyStr);
            wyniki.push_back({data, godzina, poziom, ruchy});
        }
        plik.close();

        // Sortowanie wyników według liczby ruchów rosnąco
        sort(wyniki.begin(), wyniki.end(), [](const Wynik& a, const Wynik& b) {
            return a.ruchy < b.ruchy;
        });

        cout << left << setw(12) << "Data" << setw(10) << "Godzina" << setw(10) << "Poziom" << setw(10) << "Ruchy" << "\n";
        cout << string(42, '-') << "\n";

        for (const auto& wynik : wyniki) {
            cout << left << setw(12) << wynik.data << setw(10) << wynik.godzina << setw(10) << wynik.poziom << setw(10) << wynik.ruchy << "\n";
        }

        cout << "\nNaciśnij Enter, aby wrócić do menu...";
        cin.get();
    }

    void GenerujTestoweWyniki() {
        system("cls");
        cout << "=========================\n";
        cout << "   DODAWANIE TESTOWYCH WYNIKÓW\n";
        cout << "=========================\n\n";

        ofstream plik(wynik_txt, ios::app);
        if (!plik.is_open()) {
            cout << "Nie można otworzyć pliku wyników!\n";
            cout << "Naciśnij Enter, aby wrócić do menu...";
            cin.get();
            return;
        }

        // Generowanie 5 testowych wyników
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> rokDist(2023, 2025);
        uniform_int_distribution<> miesiacDist(1, 12);
        uniform_int_distribution<> dzienDist(1, 28);
        uniform_int_distribution<> godzinaDist(0, 23);
        uniform_int_distribution<> minutaDist(0, 59);
        uniform_int_distribution<> sekundaDist(0, 59);
        uniform_int_distribution<> poziomDist(1, 2);
        uniform_int_distribution<> ruchyDist(50, 200);

        for (int i = 0; i < 5; i++) {
            int rok = rokDist(gen);
            int miesiac = miesiacDist(gen);
            int dzien = dzienDist(gen);
            int godzina = godzinaDist(gen);
            int minuta = minutaDist(gen);
            int sekunda = sekundaDist(gen);
            string poziom = poziomDist(gen) == 1 ? "Latwy" : "Trudny";
            int ruchy = ruchyDist(gen);

            ostringstream strumien;
            strumien << setfill('0') << setw(4) << rok << "-"
                     << setfill('0') << setw(2) << miesiac << "-"
                     << setfill('0') << setw(2) << dzien << ", "
                     << setfill('0') << setw(2) << godzina << ":"
                     << setfill('0') << setw(2) << minuta << ":"
                     << setfill('0') << setw(2) << sekunda << ", "
                     << poziom << ", " << ruchy;

            plik << strumien.str() << "\n";
        }

        plik.close();
        cout << "Dodano 5 testowych wyników do pliku wyniki_solitaire.txt.\n";
        cout << "Naciśnij Enter, aby wrócić do menu...";
        cin.get();
    }

    void UsunWyniki() {
        system("cls");
        cout << "=========================\n";
        cout << "   USUWANIE WYNIKÓW\n";
        cout << "=========================\n\n";

        ofstream plik("wyniki_solitaire.txt", ios::trunc);
        if (!plik.is_open()) {
            cout << "Nie można otworzyć pliku wyników!\n";
            cout << "Naciśnij Enter, aby wrócić do menu...";
            cin.get();
            return;
        }
        plik.close();
        cout << "Wszystkie wyniki zostały usunięte.\n";
        cout << "Naciśnij Enter, aby wrócić do menu...";
        cin.get();
    }
};

int main() {
    #ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    #endif

    srand(static_cast<unsigned int>(time(nullptr)));

    Menu menu;
    menu.WyswietlMenu();

    return 0;
}
