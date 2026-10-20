## Šviesos sensorius (Laboratorinis 2)

Šiam laboratoriniam darbui sukūriau konfiguruojamą šviesos sensorių. Yra mygtukai skirti konfiguruoti leistiną šviesos limitą, kuris yra išsaugomas EEPROM. Šviesos sensorius tada aptinka, kai šviesos limitas yra pasiektas ir praneša apie limito pažeidimą.

Įvykiai, kaip mygtuko paspaudimas, periodinis matavimas veikia pertraukų būtų ir šių pertraukų metu yra užregistruojamas įvykis, kurį vėliau apdoroja pagrindinis ciklas. Jeigu Arduino neturi įvykių 2s jis yra užmigdomas 4 sekundėm ir pakeliamas pertrauka, kad galėtų atlikti periodinius šviesos kiekio matavimus.

Periodinis matavimas atliekamas kas 0.25 sekundes, kai Arduino yra aktyvus.

EEPROM yra saugoma MAGIC reikšmė nustatyti ar atmintyje esantys duomenys yra teisingi, bei vartotojo nustatytas šviesos kiekio limitas.


## "Simon" žaidimas su LCD ekranu (Laboratorinis 1)

Šiam laboratoriniam darbui sukūriau paprastą atminties žaidimo „Simon“ kopiją su LCD ekranu, skirtu žaidimo konfigūravimui ir vartotojo sąsajai.

Detaliau aprašyta LaboratorinisDarbas.pdf ir įkeltame LaboratorinisDarbas.mkv failuose.
