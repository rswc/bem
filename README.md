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

Pliki wykonywalne powinny znajdować się w `./coordinator/coordinator` oraz `./node/node`

## Uruchomienie

Uruchomienie koordynatora: 

```
./build/coordinator/coordinator 
```

Uruchomienie węzła:

```
./build/node/node
```

## Demo

Koordynator udostępnia proste operacje zarządzania węzłami. 

```
list - wyświetl wszystkie podłączone węzły
hello <id> - zarejestruj wybrany węzeł z listy podłączonych 
ping <id> - wyślij wiadomość typu PING, oczekuj na PONG
task <id> - wyślij testowe zadanie na węzeł, oczekuj na wynik
```

Węzeł umożliwia jedynie pasywne podłączenie do serwera. 

Przykład komunikacji między węzłem a serwerem:

`coordinator`:
```
$ ./build/coordinator/coordinator
> list
[0] 127.0.0.1:35392 flag<0>
> hello 0
> [WT]: Node 0 sent READY message! Setting node as REGISTERED.

> ping 0
> [WT]: Node 0 sent PONG message!

> task 0
(oczekiwanie na zakończenie zadania - sleep 10s) 
> [WT]: Node 0 sent RESULT for task id: 0
```

`node`:

```
$ ./build/node/node
Waiting for HELLO message from the server...
Server returned correct protocol version.
Waiting for server instructions...
Received PING. Sending PONG.
Waiting for server instructions...
Received TASK. Appending to TaskQueue.
[DT]: Task with id 0, received. Sleeping for 10 seconds
Waiting for server instructions...
[DT]: Task with id 0, done. Sending result
```
