# Bot Evaluation Machine


## Zależności

- https://github.com/nlohmann/json

## Opis Systemu

System składa się z jednego koordynatora i wielu węzłów obliczeniowych. Komunikacja odbywa się zawsze między koordynatorem i węzłem, przy użyciu wiadomości dziedziczących z klasy `BaseMessage`. Każdy typ wiadomości implementuje własne funkcje do serializacji/deserializacji w dowolny sposób. Jedynym wspólnym elementem jest nagłówek, zawierający informację o typie i całkowitej długości.

Po stronie koordynatora dla każdego węzła tworzone są dwa wątki — do czytania i do pisania. Odpowiadają one za przekazywanie wiadomości między odpowiednimi kolejkami znajdującymi się w dzielonym przez wszystkie wątki stanie globalnym (klasie `State`). Z perspektywy koordynatora każdy węzeł ma własną kolejkę wiadomości wychodzących (przesyłanych do węzłów - w klasie `Node`), natomiast istnieje jedna kolejka wiadomości przychodzących - `recvMessageQueue`. 

Każda wiadomość przesyłana do serwera jest przetwarzana przez `MessageFactory`, której zadaniem jest deserializacja buforu bajtów do odpowiednich klas wiadomości. Następnie przesłane w kolejce wiadomości są przetwarzane przez `handler`, operujący na własnym wątku. W oparciu o przesyłane wiadomości handler aktualizuje globalny stan koordynatora.

Po stronie węzła na potrzeby komunikacji z koordynatorem również tworzone są dwa wątki, działające analogicznie. Implementacja tychże wątków oraz klasy `State` na ten moment nie jest współdzielona i występują różnice z ich odpowiednikami po stronie koordynatora, ze względu na potrzebę dystrybucji zadań na wiele węzłów oraz zbieranie zbiorczych  wyników. 

## Budowanie 

System jest budowany w oparciu o rozwiązanie CMake. 

```
mkdir build
cd build
cmake ..
make
```

### Pliki konfiguracyjne

Pliki wykonywalne powinny znajdować się w `./coordinator/coordinator` oraz `./node/node`. Należy w ich miejsce skopiować pliki `coordinator.json` oraz `node.json` w miejsce wykonania, lub podać ścieżkę przy uruchomieniu.

```
cp ../config/node.json ./node/
cp ../config/coordinator.json ./coordinator/
```

### Struktura gier

W repozytorium przygotowana została "mock" struktura, która symuluje zachowanie gier oraz botów zgodnie ze standardem występujący w rozgrywkach z przedmiotu Sztuczna Inteligencja na 5 semestrze.  

Przed uruchomieniem należy się upewnić, że node ma dostęp do struktury oraz zgadza się odpowiedni parametr `game_dir` w konfiguracji. Przykładowe przygotowanie znajduje się poniżej: 

```
cp -r ../games ./node/
```

## Uruchomienie

Uruchomienie koordynatora: 

```
cd ./coordinator && ./coordinator [config-path]
```

Uruchomienie węzła:

```
cd ./node/ && ./node [config-path]
```

## Struktura GameList

W pliku `node.json` należy ustawić atrybut `games_dir` który wyznacza korzeń folderów zawierających pliki gier. Każda gra posiada swój folder (wyznaczany przez atrybut `dirname`). W środku znajdują się pliki graczy. W korzeniu struktury znajduje się także `mock_launcher.sh` którego nazwa jest możliwa do zmiany w konfiguracji.

Przykładowa struktura plików znajduje się poniżej:

```
.
└── <games_dir>/
    ├── mock_launcher.sh
    ├── migration/
    │   ├── AlfaBetaPlayer.sh
    │   ├── MCTSPlayer.sh
    │   └── MinMaxPlayer.sh
    └── tictactoe/
        ├── GoodPlayer.sh
        └── RandomPlayer.sh
```

## Graceful Exit

Obecnie węzeł bezpiecznie zamyka wszystkie połączenia oraz zatrzymuje uruchomione wątki. Możliwe jest także zatrzymanie pracy programu wywołując sygnał SIGINT.

```
$ ./coordinator
= Bot Evaluation Machine v1.0
=
=  Iusiurandum, patri datum, usque
=  ad hanc diem ita servavi
=
= Type 'help' to view available commands.
> [WT]: Node 1 sent HELLO message!
[WT]: Set node 1 as REGISTERED. Sending Hello Response
---- GameList ----
-- [1]: Migration
[1]: Monte Carlo Tree Search Bot
[2]: Minmax Player
[3]: AlfaBeta Player
-- [2]: TicTacToe
[1]: (very) Good Player
[2]: Pretty (not) Random Player
--------
[MNCE] Connection with Node #1 broken
```

```
$ ./node
[GL]: Veryfing loaded gamelist...
---------------------------------
[2] TicTacToe : tictactoe -> [OK]
 -- [2]: [OK] - Pretty (not) Random Player -> games/tictactoe/RandomPlayer.sh
 -- [1]: [OK] - (very) Good Player -> games/tictactoe/GoodPlayer.sh
[1] Migration : migration -> [OK]
 -- [3]: [OK] - AlfaBeta Player -> games/migration/AlfaBetaPlayer.sh
 -- [2]: [OK] - Minmax Player -> games/migration/MinMaxPlayer.sh
 -- [1]: [OK] - Monte Carlo Tree Search Bot -> games/migration/MCTSPlayer.sh
---------------------------------
Sending HELLO message to coordinator...
Waiting for HELLO message from the coordinator...
Server returned ACCEPT flag in hello response.
[WFI]: Waiting for server instructions...
^C[!] Called handler for signal 2! Terminating Node program
[WFI]: Waiting for instruction loop broken. Returning.
[ET]: Execute tasks loop broken. Returning
[RFS]: Read from server loop broken. Returning.
[WTS]: Write to server loop broken. Returning.
```


## Demo

Koordynator udostępnia zestaw narzędzi do ręcznego zarządzania węzłami oraz sam automatyzuje proces kontroli poprzez śledzenie stanu połączenia. Dokładną listę poleceń można uzyskać poleceniem `help`. 

```
$ ./coordinator
= Bot Evaluation Machine v1.0
=
=  Iusiurandum, patri datum, usque
=  ad hanc diem ita servavi
=
= Type 'help' to view available commands.
> help
List of available commands:
 - help - Show this list
 - terminate <NODE_ID> - Terminate connection to node with the specified id
 - nodes - List all nodes in this system
 - tasks - List all tasks in this system
 - games - List all games recognized by this system
 - cancel <TASK_ID> - Cancel task with the specified id
 - task <GAME_ID> <AGENT_1> <AGENT_2> <BOARD_SIZE> <MOVE_LIMIT_MS> <NUM_GAMES>
     - Create new task. NUM_GAMES battles of game identified by GAME_ID
       between AGENT_1 and AGENT_2 will be performed
>
```

Węzeł umożliwia jedynie pasywne podłączenie do serwera. W trakcie wykonywanych zadań będzie wyświeltać informacje diagnostyczne. 

Poniżej znajduje się demo wysłania przykładowego zadania na 2 węzły:

### Koordynator

```bash
$ ./coordinator
= Bot Evaluation Machine v1.0
=
=  Iusiurandum, patri datum, usque
=  ad hanc diem ita servavi
=
= Type 'help' to view available commands.

# uruchomienie Node 1
[WT]: Node 1 sent HELLO message!
[WT]: Set node 1 as REGISTERED. Sending Hello Response

# uruchomienie Node 2
[WT]: Node 2 sent HELLO message!
[WT]: Set node 2 as REGISTERED. Sending Hello Response

# Wyświetl listę węzłów
> nodes
[2] 127.0.0.1:60412 flags<R..> IDLE
[1] 127.0.0.1:41984 flags<R..> IDLE

# Wyświetl listę obecnych zadań
> tasks
> games
---- GameList ----
-- [2]: TicTacToe
[2]: Pretty (not) Random Player
[1]: (very) Good Player
-- [1]: Migration
[3]: AlfaBeta Player
[2]: Minmax Player
[1]: Monte Carlo Tree Search Bot
--------

# rozegraj 27 gier pomiędzy "AlfaBeta Player" a "Minmax Player", w grę "Migration" 
# na planszy o rozmiarze 5 z limitem czasu 500 ms na rundę
> task 1 2 3 5 500 27

# Rozdzielenie zadań na dostępne i możliwe węzły
Given task: 1, 2, 3
Hello: 2, 1, 1
Hello: 1, 1, 1
Node ids for given task: [2, 1, ]
Splitting tasks to eligible nodes:
[2] -> 14
[1] -> 13
> tasks
[1] RUNNING (waiting for 2 sub-tasks to complete)
> nodes
[2] 127.0.0.1:60412 flags<R..> RUNNING TASK [1]
[1] 127.0.0.1:41984 flags<R..> RUNNING TASK [1]


# Otrzymanie pierwszego rezultatu od węzła
[WT]: Node 2 sent RESULT for task id: 1
- 14 games played.
- 1 games failed.
- 5 games won by agent1
- 8 games won by agent2
- 4 times agent1 timed out (agent2 won)
- 2 times agent2 timed out (agent1 won)
- 0 draws.
- GROUP [1] now has 1 remaining.

# Otrzymanie drugiego rezultatu od węzła
[WT]: Node 1 sent RESULT for task id: 2
- 13 games played.
- 0 games failed.
- 6 games won by agent1
- 7 games won by agent2
- 2 times agent1 timed out (agent2 won)
- 4 times agent2 timed out (agent1 won)
- 0 draws.
- GROUP [1] now has 0 remaining.

# Powiadomienie o zakończeniu grupy zadań - agregacja wyników
- GROUP [1] has finished processing.
-- Total of 27 games played.
-- Total of 11 won by agent 1.
-- Total of 15 won by agent 2.
```


### Węzeł 1

```bash
$ ./node

# Weryfikacja konfiguracji z plikami systemowymi
[GL]: Veryfing loaded gamelist...
---------------------------------
[2] TicTacToe : tictactoe -> [OK]
 -- [2]: [OK] - Pretty (not) Random Player -> games/tictactoe/RandomPlayer.sh
 -- [1]: [OK] - (very) Good Player -> games/tictactoe/GoodPlayer.sh
[1] Migration : migration -> [OK]
 -- [3]: [OK] - AlfaBeta Player -> games/migration/AlfaBetaPlayer.sh
 -- [2]: [OK] - Minmax Player -> games/migration/MinMaxPlayer.sh
 -- [1]: [OK] - Monte Carlo Tree Search Bot -> games/migration/MCTSPlayer.sh
---------------------------------

# Hello-Hanshake z koordynatorem
Sending HELLO message to coordinator...
Waiting for HELLO message from the coordinator...
Server returned ACCEPT flag in hello response.
[WFI]: Waiting for server instructions...

# Otrzymanie zadania o id 1 - uruchomienie 14 gier
> [1] (1/14) Running:  games/mock_launcher.sh games/migration/MinMaxPlayer.sh games/migration/AlfaBetaPlayer.sh Migration 5 500
Playing game Migration with board size of 500 and 13 rounds.
Playing round: 1, sleeping .500 [s].
Playing round: 2, sleeping .500 [s].
Playing round: 3, sleeping .500 [s].
Playing round: 4, sleeping .500 [s].
Playing round: 5, sleeping .500 [s]

# <ucięte>
# ...

> [1] (14/14) Running:  games/mock_launcher.sh games/migration/MinMaxPlayer.sh games/migration/AlfaBetaPlayer.sh Migration 5 500
Playing game Migration with board size of 500 and 18 rounds.
Playing round: 1, sleeping .500 [s].
Playing round: 2, sleeping .500 [s].
Playing round: 3, sleeping .500 [s].
Playing round: 4, sleeping .500 [s].
Playing round: 5, sleeping .500 [s].
Playing round: 6, sleeping .500 [s].
[EJ]: games/migration/MinMaxPlayer.sh;games/migration/AlfaBetaPlayer.sh;"PLAYER2";TIMEOUT;

# Przesłanie wyniku do koordynatora
[ET]: Task with id 1, DONE. Sending result.
```

### Węzeł 2

```bash
$ ./node
[GL]: Veryfing loaded gamelist...
---------------------------------
[2] TicTacToe : tictactoe -> [OK]
 -- [2]: [OK] - Pretty (not) Random Player -> games/tictactoe/RandomPlayer.sh
 -- [1]: [OK] - (very) Good Player -> games/tictactoe/GoodPlayer.sh
[1] Migration : migration -> [OK]
 -- [3]: [OK] - AlfaBeta Player -> games/migration/AlfaBetaPlayer.sh
 -- [2]: [OK] - Minmax Player -> games/migration/MinMaxPlayer.sh
 -- [1]: [OK] - Monte Carlo Tree Search Bot -> games/migration/MCTSPlayer.sh
---------------------------------
Sending HELLO message to coordinator...
Waiting for HELLO message from the coordinator...
Server returned ACCEPT flag in hello response.
[WFI]: Waiting for server instructions...

# Otrzymanie zadania od koordynatora
> [2] (1/13) Running:  games/mock_launcher.sh games/migration/MinMaxPlayer.sh games/migration/AlfaBetaPlayer.sh Migration 5 500
Playing game Migration with board size of 500 and 20 rounds.
Playing round: 1, sleeping .500 [s].
Playing round: 2, sleeping .500 [s].
Playing round: 3, sleeping .500 [s].
Playing round: 4, sleeping .500 [s].
Playing round: 5, sleeping .500 [s].
Playing round: 6, sleeping .500 [s]
# <ucięte>
# ...

> [2] (13/13) Running:  games/mock_launcher.sh games/migration/MinMaxPlayer.sh games/migration/AlfaBetaPlayer.sh Migration 5 500
Playing game Migration with board size of 500 and 18 rounds.
Playing round: 1, sleeping .500 [s].
Playing round: 2, sleeping .500 [s].
Playing round: 3, sleeping .500 [s].
Playing round: 4, sleeping .500 [s].
Playing round: 5, sleeping .500 [s].
Playing round: 6, sleeping .500 [s].
Playing round: 7, sleeping .500 [s].
Playing round: 8, sleeping .500 [s].
Playing round: 9, sleeping .500 [s].
Playing round: 10, sleeping .500 [s].
Playing round: 11, sleeping .500 [s].
Playing round: 12, sleeping .500 [s].
Playing round: 13, sleeping .500 [s].
Playing round: 14, sleeping .500 [s].
Playing round: 15, sleeping .500 [s].
Playing round: 16, sleeping .500 [s].
Playing round: 17, sleeping .500 [s].
Playing round: 18, sleeping .500 [s].

# Wynik gry zwrócony w formacie zgodnym dla wszystkich gier
[EJ]: games/migration/MinMaxPlayer.sh;games/migration/AlfaBetaPlayer.sh;"PLAYER2";;

# Wysłanie wyniku do koordynatora
[ET]: Task with id 2, DONE. Sending result.

# Zakończenie programu
^C[!] Called handler for signal 2! Terminating Node program
[WFI]: Waiting for instruction loop broken. Returning.
[ET]: Execute tasks loop broken. Returning
[RFS]: Read from server loop broken. Returning.
[WTS]: Write to server loop broken. Returning.
```