## Šviesos sensorius+ (Laboratorinis 3)

Šiam laboratoriniuam darbui tęsiau darbą su šviesos sensoriumi suskurtu 2 laboratorinio darbo metu.

Išlieka vienodas funkcionalumas su papildymais:
1. GPIO, LCD, I2C, timerių valdymo kodas parašytas ranka. Nėra naudojama jokių papildomų bibliotekų (kaip LiquidCrystal) ar pagalbinių funkcijų iš Arduino.h. Pinai, timer'iai ir t.t. valdomi tiesiogiai rašant į ATmega328P procesoriaus registrus. Pertraukimai (ISR) apibžrėžti su avr-gcc kompilatoriaus pagalba.
2. Pridėtas LCD pranešti vartotojui apie dabartinę sensoriaus būsena, matomas konfiguravimo langas, pranešimas, kad arduino šiuo metu "miega".
3. Pridėtas garso signalas, kuris aktyvuojasi, kai šviesos limitas yra viršijamas. 

## Šviesos sensorius (Laboratorinis 2)

Šiam laboratoriniam darbui sukūriau konfiguruojamą šviesos sensorių. Yra mygtukai skirti konfiguruoti leistiną šviesos limitą, kuris yra išsaugomas EEPROM. Šviesos sensorius tada aptinka, kai šviesos limitas yra pasiektas ir praneša apie limito pažeidimą.

Įvykiai, kaip mygtuko paspaudimas, periodinis matavimas veikia pertraukų būtų ir šių pertraukų metu yra užregistruojamas įvykis, kurį vėliau apdoroja pagrindinis ciklas. Jeigu Arduino neturi įvykių 2s jis yra užmigdomas 4 sekundėm ir pakeliamas pertrauka, kad galėtų atlikti periodinius šviesos kiekio matavimus.

Periodinis matavimas atliekamas kas 0.25 sekundes, kai Arduino yra aktyvus.

EEPROM yra saugoma MAGIC reikšmė nustatyti ar atmintyje esantys duomenys yra teisingi, bei vartotojo nustatytas šviesos kiekio limitas.


## "Simon" žaidimas su LCD ekranu (Laboratorinis 1)

Šiam laboratoriniam darbui sukūriau paprastą atminties žaidimo „Simon“ kopiją su LCD ekranu, skirtu žaidimo konfigūravimui ir vartotojo sąsajai.

Detaliau aprašyta LaboratorinisDarbas.pdf ir įkeltame LaboratorinisDarbas.mkv failuose.
