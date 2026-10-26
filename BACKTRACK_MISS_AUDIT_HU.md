# Backtrack miss: reprodukált időzítési hibák és javítások

2026-10-26. A vizsgálat a jelenlegi RyzeX kódot és a helyi `cstrike15_src` szerveroldali lagcomp-számítását vetette össze. A bizonyított hibákat javítottam; élő szerveres hitrate-mérés nem történt.

## Miért lehet jó a GetDamage, és mégis miss a lövés?

A scanner a kiválasztott rekord mátrixán vizsgál. A szerver a kiküldött `tick_count`, a szerveren tárolt lerp és a lövés feldolgozásának ideje alapján választ visszaállítási időpontot. Ha ez eltér a scannerben vizsgált időtől, a sikeres geometriai/GetDamage-vizsgálat másik pózra vonatkozhat. A tíz yaw-hipotézis közös pontja is ugyanannak a kiválasztott rekordnak a pózait ellenőrzi; időbélyeg-eltérést önmagában nem old meg.

## 1. cl_interpolate=0 esetén eltérő lerp — reprodukált

A régi `GetClientInterpAmount()` rögtön nullát adott vissza, ha a kliens `cl_interpolate` értéke nulla volt.

A helyi szerver `game/server/gameinterface.cpp:3291` sora emberi játékosnál ettől függetlenül bekapcsolja az interp-számítást. A `cl_interpolate` szerveroldali kikapcsoló ága ebben a kódban botokra vonatkozik. A szerver a ratio/updaterate és a cl_interp maximumából számol, a szerveres limitekkel.

Reprodukció, 64 tick, ratio 2, cl_interp 0:

| | Régi kliens | Szerver |
|---|---:|---:|
| Lerp | 0 ms | 31,25 ms |
| Kért rekordidő | 99,875 s | A küldött tickből 99,84375 s adódik |

Ez két tick eltérés. 250 unit/s mozgásnál 7,8125 unit helyzetkülönbséget jelenthet, ami fejre célzásnál jelentős.

Javítás: a lerpet a szerver emberi játékosra alkalmazott képletével számoljuk. A cvarok aktuális értékei futáskor kerülnek kiolvasásra; a cl_interpolate renderkapcsoló nem nullázza a lövési lerpet.

## 2. A rekord jóváhagyása nem a ténylegesen küldött tickre történt — reprodukált

A régi érvényességvizsgálat a nyers `flSimulationTime` alapján ellenőrizte a 200 ms-os eltérést. A lövés azonban egész tickre kerekített időbélyeget küld. A szerver ebből újra másodpercet számol, majd levonja a lerpet.

A `game/server/player_lagcompensation.cpp:287–296` alapján, 200 ms-nál nagyobb eltérésnél a szerver a kliens által kért idő helyett latency-alapú időre áll vissza.

Konkrét reprodukció:

- 64 tick, feldolgozási idő 100 s, lerp 100 ms, kontrollált latency 14 ms.
- Kiválasztott rekord: 99,6875 s.
- A régi, nyers idő szerinti eltérés 198,5 ms: a kliens elfogadta.
- A küldött tick 6386; ebből a szerver kért ideje 99,68125 s.
- A szerver által ellenőrzött eltérés már 204,75 ms, ezért átáll 99,886 s-ra.

A szerver így a kért visszaállításhoz képest kb. 205 ms-mal más időpontot használhatott.

Javítás: a közös `BacktrackTiming.h` a ténylegesen kódolt tickből számolja vissza az ellenőrzött időt. A rekord kiválasztása, szűrése és a lövés időbélyege ugyanazt a számítást használja. A várhatóan átállított vagy már eldobott történeti rekordra nem küldünk automatikus lövést.

## 3. Tick és másodperc keveredett a history cutoffban — reprodukált

A régi kód egy egész másodpercben számolt `nDeadTime` értéket hasonlított egy `TIME_TO_TICKS(...)` eredményhez. Például a 125 másodperces határ mellett egy kb. 8000-es tickérték szerepelt az összehasonlítás másik oldalán. Ez az ág nem a tervezett elavultsági szűrést végezte.

A helyi szerver a history törlésénél egész másodpercre alakított határt használ (`player_lagcompensation.cpp:700–709`). Az új kód ehhez a határhoz a rekord másodpercben tárolt simulation time-ját hasonlítja.

Ez különösen akkor számít, amikor a játékos parancsához tartozó tickbase és az utoljára megfigyelt szerveróra eltér. Az erre készített tesztben a régi kód átengedte a history határán kívüli rekordot.

## 4. Célpont nélküli támadás tick_count=0 értéket kaphatott — reprodukált

Az `iDesiredTickcount` alapértéke 0 volt. A `PostPrediction()` minden IN_ATTACK parancsnál átírhatta a ticket, ha ez nem -1 volt; a ClearTarget sem törölte a korábban előkészített időbélyeget.

Így célpont nélküli kézi támadásnál is 0 kerülhetett a parancsba. Visszavont támadás után pedig megmaradhatott egy előkészített lövési állapot. Ezek a backtrack-időbélyeg kezelésének hibái; a kézi támadás esete külön útvonal, nem magyaráz minden automatikus backtrack misst.

Javítás:

- Az alapérték -1, és a ClearTarget törli a pending időbélyeget.
- Csak a ténylegesen előkészített, megfelelő célponttal rendelkező automatikus lövés írhatja át a ticket.
- Visszavont támadás törli az előkészített küldési állapotot.
- A végső tickshift-terv után újraellenőrizzük az időzítést és a fegyver lövési készenlétét. Érvényes óraeltolás továbbra is megengedett; érvénytelen párra az adott automatikus támadás kimarad.

## 5. A shot regisztráció a támadás visszavonása előtt történt

A számláló és az Experimental shot queue korábban a CreateMove kezdeti lövési döntésénél frissült. Ezután például az auto-pistol még visszavonhatta az IN_ATTACK-et. Egy ilyen bejegyzés összezavarhatta a következő weapon_fire/impact esemény hozzárendelését.

A regisztráció most a PostPredictionben, a megmaradt támadás és a végső időzítés ellenőrzése után történik. A célzott teszt ellenőrzi, hogy visszavont vagy a végső fegyveridő szerint nem végrehajtható támadásból nem keletkezik shot bejegyzés.

## Új miss-diagnosztika

Az Experimental misslog a lövéskor eltárolt adatokat írja ki:

`bt ... | tick ... | lerp ... ms | rounding ... ms | timing delta ... ms`

A bt értéket is a lövéskor mentjük; az események feldolgozásakor megváltozott interp cvar nem írja át visszamenőleg. A timing delta kliensoldali latency-becslést használ, nem egy szerverről visszaigazolt mérés.

A rounding önmagában lehet az egész tick kódolásának természetes maradéka. Az elfogadott időbélyeg nem bizonyítja a rekonstruált resolver-/animációs póz teljes szerveroldali azonosságát. A logger korábbi összevont safe/backtrack/spread/occlusion címkéje sem volt egyetlen konkrét szerveroldali miss-ok bizonyítéka.

## Tesztek és build

A `tests/run_backtrack_regressions.ps1` a production függvényeket fordítja le, és a helyi szerverforrásból ténylegesen kivett interp-/target-time számítással hasonlítja össze. A javítás előtti első 98 ellenőrzésből **52 elbukott**; a javítás utáni kibővített futás **53 622 ellenőrzést teljesített, nulla hibával**.

Lefedés: 64/128 tick, cl_interpolate 0/1, egész és tört interp ratio, cl_interp, változó latency, history-határ, hiányzó kapcsolat/játékos, kézi és visszavont támadás, későn módosított cvar, érvényes/érvénytelen tickbase-eltolás és megváltozott végső fegyverkészenlét. A kontrollált latency-egyezés az időzítési képletet izolálja; a távoli peer valós latencyje kliensoldalról továbbra is becslés.

A korábbi hitscan-fixture az interp/időzítés egy részét helyettesítette, ezért nem fedte le ezeket az eltéréseket. Az új ellenőrzés közvetlenül a szerveres képlettel tesztel.

További sikeres ellenőrzések:

- 1 987 159 Experimental/geometria/hitscan ellenőrzés.
- 13 789 tickshift és 1 492 controller ellenőrzés.
- 17 737 render-/csontcache ellenőrzés.

Win32 Release kimenet: `RyzeXTR/output/backtrack/RyzeXTR.dll`, mellette PDB. A build sikeres; a korábban is meglévő LNK4098 figyelmeztetés megmaradt.

A következő élő összehasonlításhoz ezt a backtrack mappában lévő buildet kell használni. Az eredmények a konkrét időzítési hibák javítását igazolják; egyedi játékbeli miss okát a lövés pillanatának adataival és az új loggal lehet tovább szűkíteni.
