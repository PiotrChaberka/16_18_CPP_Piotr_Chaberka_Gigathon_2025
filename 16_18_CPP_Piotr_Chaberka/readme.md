# **Pasjans**

**Pasjans to klasyczna gra w pasjansa napisana w języku C++. Gra zawiera dwa poziomy trudności, możliwość cofania ruchów, zapis wyników do pliku oraz tryb debugowania z dodatkowymi funkcjami.**

# **Uruchomienie gry**

**Windows: Uruchom plik** `pasjans.exe`**.**

**Linux/Mac: Użyj programu WINE lub innego emulatora Windows, aby uruchomić** `pasjans.exe`**.**

**Wyniki gry zapisywane są w pliku** `wyniki_solitaire.txt`**, który znajduje się w tym samym katalogu co plik wykonywalny gry.**

# **Sterowanie**

**Gra obsługiwana jest za pomocą komend wpisywanych w konsoli:**

`kk [z] [do] [ile]` **– Przenieś** `[ile]` **kart z kolumny** `[z]` **do kolumny** `[do]`**.**

`dk` **– Dobierz kartę (lub karty, w zależności od poziomu trudności) ze stosu rezerwowego.**

`wk [kolumna]` **– Przenieś kartę ze stosu widocznego (odkrytego) do wybranej** `[kolumny]`**.**

`s k[kolumna]` **– Przenieś kartę z wybranej** `[kolumny]` **na jeden ze stosów końcowych.**

`s w` **– Przenieś kartę ze stosu widocznego na jeden ze stosów końcowych.**

`cofnij` **– Cofnij ostatni wykonany ruch.**

`restart` **– Rozpocznij nową grę od początku.**

`wyjdz` **– Zakończ grę i zamknij program.**

`pomoc` **– Wyświetl pełną listę dostępnych komend w grze.**

# **Tryb debugowania**

**Aby włączyć tryb debugowania, wpisz komendę** `debugstick` **w menu głównym gry. Tryb ten odblokowuje dodatkowe opcje:**

**Dodaj testowe wyniki: Generuje 5 losowych wyników i zapisuje je do pliku** `wyniki_solitaire.txt`**.**

**Usuń wszystkie wyniki: Usuwa wszystkie dotychczasowe wyniki z pliku** `wyniki_solitaire.txt`**.**

# **Cel gry**

**Celem gry jest uporządkowanie wszystkich kart w czterech stosach końcowych, układając je w kolejności rosnącej (od asa do króla) w ramach tego samego koloru (kier, karo, pik, trefl).**

# **Zasady gry**

**W kolumnach: Karty układa się w kolejności malejącej (Król → Dama → Walet → ... → As) z naprzemiennymi kolorami (czerwony: kier/karo, czarny: pik/trefl).**

**Na stosach końcowych: Karty układa się w kolejności rosnącej (As → 2 → 3 → ... → Król) w tym samym kolorze.**

**Pusta kolumna: Na pustą kolumnę można położyć wyłącznie kartę Króla.**

**Poziomy trudności**

**Łatwy: Ze stosu rezerwowego dobierana jest 1 karta na raz.**

**Trudny: Ze stosu rezerwowego dobierane są 3 karty naraz, ale tylko wierzchnia karta jest dostępna do użycia.**

# **Opis kodu źródłowego**

**Kod gry napisany jest w C++ i zawiera szczegółowe komentarze opisujące każdą klasę oraz metodę. Główne klasy w kodzie:**

**Karta: Reprezentuje pojedynczą kartę z jej wartością i kolorem.**

**Ruch: Przechowuje informacje o wykonanym ruchu, umożliwiając jego cofnięcie.**

**Pasjans: Odpowiada za główną logikę gry, w tym zarządzanie kartami i zasadami.**

**Menu: Zarządza interfejsem menu głównego oraz funkcjami dodatkowymi, takimi jak zapis wyników czy tryb debugowania.**

# **Kontakt**

**W razie wystąpienia problemów lub dodatkowych pytań proszę o kontakt za pośrednictwem poczty e-mail `PiotrChaberka@proton.me` lub discord `axello1`. Miłej gry!**