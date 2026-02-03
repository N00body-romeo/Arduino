# ESP8266 Sunrise Alarm Clock

Questo progetto implementa una Sveglia effetto Alba basata su microcontrollore ESP8266 (es. NodeMCU o Wemos D1 Mini).
La sveglia si sincronizza automaticamente via internet (NTP) e simula il sorgere del sole tramite LED RGB prima che suoni l'allarme sonoro.

## Funzionalità Principali
Il sistema opera su una "macchina a stati" (State Machine) che gestisce le seguenti fasi:
- **IDLE (Riposo)**: Il sistema è in attesa, tutto è spento.
- **SUNRISE (Alba)**: Inizia 30 minuti prima dell'orario impostato. La luce si accende gradualmente virando dal rosso al bianco/blu per simulare l'alba.
- **ALARM (Sveglia)**: All'orario X, la luce è al 100% e il Buzzer inizia a suonare.
- **DAYLIGHT (Luce Giorno)**: Se l'utente preme il pulsante durante l'allarme, il suono smette ma la luce rimane accesa per altri 30 minuti (utile per vestirsi/prepararsi), poi si spegne automaticamente.

## Configurazione (Cosa devi modificare)
Prima di caricare il codice, è obbligatorio compilare la sezione ! CONFIGURAZIONE UTENTE all'inizio dello sketch:
- **Wi-Fi Credentials:**
Inserisci il nome e la password della tua rete.

```C
const char *ssid     = "IL_TUO_WIFI";
const char *password = "LA_TUA_PASSWORD";
```

- **Orario Sveglia:**
Imposta l'ora e i minuti desiderati (formato 24h).

```C
const int ALARM_HOUR   = 7;
const int ALARM_MINUTE = 30;
```

- **Fuso Orario (UTC Offset):**
Il codice usa un offset in secondi.

```C
3600 = UTC+1 (Italia Orario Solare / Inverno)
7200 = UTC+2 (Italia Orario Legale / Estate)
const long UTC_OFFSET = 3600; 
```


## Guida all'uso del Pulsante

| Stato Attuale      | Risultato |
| ----------- | ----------- |
| Durante l'Alba      | Annulla l'alba e torna a dormire (IDLE)       |
| Mentre Suona   | Ferma il suono, mantene la luce accesa (DAYLIGHT)        |
| Luca Fissa (DAYLIGHT) | Spegne tutto manualmente (IDLE) |


## Cosa manca / Da implementare
Il codice è funzionale ma "base".
- **Gestione Ora Legale Automatica:** Attualmente l'offset (3600) è fisso. Se cambia l'ora, devi ricaricare il codice o implementare una funzione che calcoli se l'ora legale è attiva.
- **Interfaccia Web:** L'orario della sveglia è "hardcoded" (scritto nel codice). Sarebbe ideale aggiungere un piccolo WebServer per impostare l'ora da smartphone senza riprogrammare il chip.
- **Gestione WiFi Persa:** Se il WiFi cade, l'orario potrebbe non aggiornarsi. Manca una logica di riconnessione robusta nel `loop()`.

## Librerie Richieste
Assicurati di installare tramite il Library Manager di Arduino IDE:
`NTPClient by Fabrice Weinberg`
