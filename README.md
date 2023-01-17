# Bot Evaluation Machine


## Zależności

- TUI: https://github.com/jmicjm/TUI

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

## Pliki konfiguracyjne

Pliki wykonywalne powinny znajdować się w `./coordinator/coordinator` oraz `./node/node`. Należy w ich miejsce skopiować pliki `coordinator.json` oraz `node.json`. 

```
cp ./config/node.json ./build/node/
cp ./config/coordinator.json ./build/coordinator/
```
## Uruchomienie

Uruchomienie koordynatora: 

```
cd ./build/coordinator/ && ./coordinator 
```

Uruchomienie węzła:

```
cd ./build/node/ && ./node
```

## Struktura GameList

W pliku `node.json` należy ustawić atrybut `games_dir` który wyznacza korzeń folderów zawierających pliki gier. Każda gra posiada swój folder (wyznaczany przez atrybut `dirname`). W środku znajuje się plik gry `<filename>` w formacie JAR oraz folder `agents/` zawierający graczy dla danej gry.

Przykładowa struktura plików znajduje się poniżej:

```
.
└── <games_dir>/
    ├── migration/
    │   ├── migration.jar
    │   └── agents/
    │       ├── ExtremeRandomPlayer.jar
    │       ├── RobBanks.jar
    │       └── DonkeyKong.jar
    └── tictactoe/
        ├── TicTacToe.jar
        └── agents/
            ├── SimpleRandom.jar
            ├── NaivePlayer.jar
            └── TicTacTocker.jar
```

## Graceful Exit

Obecnie węzeł bezpiecznie zamyka wszystkie połączenia oraz zatrzymuje uruchomione wątki: 

```
$ ./coordinator
[WT]: Node 1 sent HELLO message!
[WT]: Set node 1 as REGISTERED. Sending Hello Response
---- GameList ----
-- [1]: migration
[1]: MCTSBot
[2]: RandomTestBot
--------
> nodes
[1] 127.0.0.1:51658 flag<R>
> terminate 1
> nodes
> 
```

```
$ ./node
Sending HELLO message to coordinator...
Waiting for HELLO message from the coordinator...
Server returned ACCEPT flag in hello response.
Waiting for server instructions...
[RFS]: Read from server loop broken. Returning.
[WFI]: Waiting for instruction loop broken. Returning.
[ET]: Execute tasks loop broken. Returning
[WTS]: Write to server loop broken. Returning.
$
```


## Demo

Koordynator udostępnia proste operacje zarządzania węzłami. 

```
nodes - wyświetl wszystkie podłączone węzły
games - wyświetl listę dostępnych gier oraz agentów na serwerze
notify <node_id> - wyślij zapytanie o stan węzła
terminate <node_id> - zakończ połaczenie z węzłem
task <game_id> <agent_id> <agent_id> <board_size> <round_limit_ms> <games> - rozdziel zadania na zarejestrowane węzły tak aby każdy otrzymał równą ilość gier do rozegrania
```

Węzeł umożliwia jedynie pasywne podłączenie do serwera. 

Przykład komunikacji między węzłem a serwerem:

`coordinator`:
```bash
$ ./coordinator
> nodes
>
<podłączenie węzła>
[WT]: Node 1 sent HELLO message!
[WT]: Set node 1 as REGISTERED. Sending Hello Response
---- GameList ----
-- [1]: migration
[1]: MCTSBot
[2]: RandomTestBot
--------
> nodes
[1] 127.0.0.1:45314 flag<R>
> notify 1
[WT]: Node 1 sent Notification Message !
[WT]: Reply took node [1] 0.00033152 seconds!
Node is currently idle

# wyślij zadanie dla node[1] aby rozegrał 200 gier
> task 1 1 2 50000 8 200
Node ids for given task: [1, ]
Splitting tasks to eligible nodes:
[1] -> 200
> notify 1
[WT]: Node 1 sent Notification Message !
# Czas odpowiedzi od węzła
[WT]: Reply took node [1] 0.000383893 seconds!
Node is running task 1

# Węzeł zwrócił wynik
[WT]: Node 1 sent RESULT for task id: 1
```

`node`:

```
$ ./node
Sending HELLO message to coordinator...
Waiting for HELLO message from the coordinator...
Server returned ACCEPT flag in hello response.
Waiting for server instructions...
Received NOTIFY. Sending Reply
Waiting for server instructions...
Received TASK. Appending to TaskQueue.
Waiting for server instructions...
[DT]: Task with id 1, received. Sleeping for 10 seconds
Received NOTIFY. Sending Reply
Waiting for server instructions...
[DT]: Task with id 1, done. Sending result
```
