# FORMA Studio — Implementation Plan

## Obiettivo

Trasformare il fork di Bambu Studio in un'applicazione per chioschi di stampa 3D con due modalità distinte:

- **FORMA Mode**: interfaccia semplificata per il cliente.
- **Admin Mode**: interfaccia tecnica per operatore e manutenzione.

Il cliente deve poter completare un ordine, inviarlo in coda e lasciare immediatamente il chiosco disponibile per il cliente successivo. La preparazione, lo slicing e la stampa proseguono in background.

## Repository

- UI attuale: `fnordjoeart-art/forma`
- Motore desktop: `fnordjoeart-art/forma-studio`
- Ramo di sviluppo iniziale: `forma-kiosk-prototype`
- Ramo stabile protetto: `master`

## Principio architetturale

FORMA Studio sarà distribuito come un unico software, anche se il codice sorgente continuerà inizialmente a vivere in due repository.

```text
React FORMA UI
      |
      | JSON commands/events
      v
FormaBridge (C++)
      |
      v
Bambu Studio engine
      |
      +--> project/3MF
      +--> slicing
      +--> printer connection
      +--> print queue
```

La build React sarà esportata come file statici e incorporata nelle risorse locali di FORMA Studio.

## Componenti da realizzare

### 1. FormaWebHost

Nuovo componente C++ ispirato a `DeviceWebHost`.

Responsabilità:

- creare la WebView;
- caricare la build locale React;
- impedire la navigazione verso origini non autorizzate;
- collegare la WebView al bridge C++;
- supportare modalità kiosk a schermo intero.

Percorso proposto:

```text
src/slic3r/GUI/FormaWeb/FormaWebHost.hpp
src/slic3r/GUI/FormaWeb/FormaWebHost.cpp
```

### 2. FormaBridge

Nuovo bridge JSON ispirato a `DeviceWebBridge`.

Percorso proposto:

```text
src/slic3r/GUI/FormaWeb/FormaBridge.hpp
src/slic3r/GUI/FormaWeb/FormaBridge.cpp
```

Comandi iniziali:

```text
ping
open_product
slice_project
get_estimate
enqueue_print
get_job_status
cancel_job
open_admin_mode
```

Eventi iniziali:

```text
engine_ready
product_loaded
slice_started
slice_completed
job_queued
job_started
job_completed
job_failed
printer_status_changed
```

### 3. Risorse React

Percorso proposto:

```text
resources/web/forma/dist/
```

La prima build dovrà funzionare:

- dentro Bambu Studio tramite bridge reale;
- nel browser tramite bridge simulato;
- senza dipendere dalla preview Base44;
- senza contenere credenziali o token permanenti.

### 4. Queue Manager

La coda deve essere indipendente dalla sessione cliente.

Stati minimi:

```text
created
validating
slicing
queued
sending
printing
completed
failed
cancelled
```

Ogni lavoro deve contenere almeno:

```text
job_id
order_id
product_id
project_path
printer_id
status
created_at
updated_at
retry_count
error_code
error_message
```

## Flusso cliente

```text
Splash
  -> Welcome
  -> Account oppure Guest
  -> Catalog
  -> Product Detail
  -> Customize
  -> Confirm
  -> Payment
  -> Order Accepted
  -> Reset della sessione
  -> Welcome per il cliente successivo
```

Il reset dell'interfaccia non deve cancellare il lavoro di stampa.

## Flusso tecnico

```text
Ordine confermato
  -> verifica file e stampante
  -> caricamento 3MF
  -> applicazione profilo approvato
  -> slicing
  -> calcolo stima
  -> inserimento in coda
  -> invio alla stampante quando disponibile
  -> aggiornamento stato
```

## Modalità amministratore

Accesso protetto da PIN o credenziale locale.

Funzioni previste:

- visualizzazione coda;
- annullamento e nuovo tentativo;
- stato stampanti;
- manutenzione;
- catalogo e associazione dei file 3MF;
- accesso controllato all'interfaccia tecnica nativa;
- log diagnostici.

## Fasi di sviluppo

### Fase 0 — Baseline

- compilare il fork senza modifiche;
- registrare sistema operativo e dipendenze;
- verificare avvio e funzioni principali;
- conservare un riferimento alla build funzionante.

**Criterio di uscita:** il fork compila e si avvia senza regressioni introdotte da FORMA.

### Fase 1 — WebView FORMA

- creare `FormaWebHost`;
- aggiungere pagina React minimale;
- implementare `ping` e `engine_ready`;
- aggiungere accesso temporaneo a FORMA Mode.

**Criterio di uscita:** un pulsante React comunica realmente con il C++.

### Fase 2 — Prodotto pilota

- usare un solo prodotto e un solo file 3MF;
- implementare `open_product`;
- validare file, piatto e profilo;
- restituire errori leggibili alla UI.

**Criterio di uscita:** il prodotto viene caricato dal motore senza mostrare pannelli tecnici al cliente.

### Fase 3 — Slicing e stima

- implementare slicing controllato;
- restituire tempo e materiale stimati;
- gestire timeout e fallimenti;
- impedire combinazioni non approvate.

**Criterio di uscita:** la UI riceve una stima reale generata dal motore.

### Fase 4 — Coda persistente

- creare Queue Manager;
- separare sessione cliente e lavoro tecnico;
- ripristinare la coda dopo riavvio;
- aggiungere log ed error handling.

**Criterio di uscita:** dopo l'ordine la UI torna alla home mentre il lavoro resta attivo.

### Fase 5 — Stampa reale

- collegare una stampante pilota;
- inviare un lavoro approvato;
- seguire lo stato fino al completamento;
- implementare cancellazione e recupero errore.

**Criterio di uscita:** ciclo completo ordine-stampa completato in ambiente controllato.

### Fase 6 — UI FORMA completa

- integrare accesso, catalogo e personalizzazione;
- aggiungere pagamento in modalità test;
- collegare schermata stato ordine;
- proteggere modalità amministratore.

### Fase 7 — Hardening del chiosco

- watchdog e avvio automatico;
- recovery dopo crash o blackout;
- pulizia dati tra utenti;
- sicurezza WebView;
- test touchscreen e accessibilità;
- telemetria e diagnostica.

## Primo prototipo

Il primo prototipo non deve stampare.

Deve dimostrare soltanto:

1. avvio di FORMA Studio;
2. apertura di FORMA Mode;
3. visualizzazione della UI React locale;
4. comando `ping` inviato dalla UI;
5. risposta `engine_ready` ricevuta dal C++;
6. accesso alla modalità tecnica tramite controllo amministratore.

## Regole di sviluppo

- non lavorare direttamente su `master`;
- una modifica logica per commit;
- pull request piccole e revisionabili;
- nessun token o segreto nella repository;
- nessun invio in stampa senza validazione esplicita;
- nessuna funzione demo deve essere confusa con una funzione operativa;
- mantenere separati codice FORMA e codice upstream dove possibile;
- documentare ogni modifica che complica l'allineamento con Bambu Studio upstream.

## Licensing

Il fork deriva da Bambu Studio e deve rispettare la licenza AGPL v3 e le attribuzioni applicabili. Prima della distribuzione commerciale dovrà essere svolta una verifica legale dedicata, inclusi eventuali componenti di rete non liberi o distribuiti separatamente.

## Prossime attività

1. compilare il fork stock;
2. mappare il punto di inserimento in `MainFrame` o nel sistema delle pagine;
3. creare cartella `FormaWeb` e relativo `CMakeLists.txt`;
4. implementare `FormaWebHost` minimale;
5. creare una pagina HTML locale di test;
6. implementare il bridge `ping`;
7. mostrare `engine_ready` nella pagina;
8. sostituire la pagina test con la build React minimale;
9. preparare il prodotto pilota 3MF;
10. iniziare `open_product`.
