# METRIC — Production-Readiness-Plan

> Ziel: METRIC von „starker Kern, unbequeme Ränder" zu einer Bibliothek machen, mit der
> man **eine reale Datei mit Daten in < 15 Minuten** laden, analysieren und das Ergebnis
> herausbekommen kann — in C++ **und** Python.
>
> Grundlage: vollständiger DX-Audit über 10 Dimensionen (73 Roh-Befunde, **69 bestätigt**,
> 2 adversarial widerlegt). Kanonische Kopie liegt auf Branch `claude/m0-production-readiness`
> (der Arbeitsbaum wird vom nebenläufigen Codex-Prozess per `git clean` geleert).

## Status

- **M0 „Es baut und lügt nicht" — ERLEDIGT & verifiziert** (2026-06-22): WS-1.1 ODR-Wheel-Fix,
  WS-1.3 LAPACK über `metric::metric`, WS-1.2 Doku-Ehrlichkeit, WS-4 Python-Doc-Banner,
  WS-6 `operator<<`/`summary` für alle 9 `*Result`-Typen. Alles out-of-tree verifiziert.
- **Nebenläufiger Codex-Track committet aktiv auf `main`** und überholt Teile des Plans:
  `read_csv`/`write_csv` (WS-3), lazy distance tables / range-query hardening (WS-7),
  `discover_metrics` (WS-9), finite-space diagnosis, mixed-record validation, app-templates.
  → Vor jedem M1-Schritt aktuellen Stand neu prüfen, Kollisionen vermeiden.
- **Risiko Nebenläufigkeit:** Codex `git clean`t untracked Dateien wiederholt und committet auf
  `main`. M0 auf Schutz-Branch gesichert. Vendored `python/pybind11` (2.4.3) inkompatibel mit
  Python ≥3.11 → M1-Voraussetzung.

## 1. Definition of „Production Ready" (Abnahme-Gates)

- **G1 Build:** `pip install` erzeugt importierbares Wheel; `workflow.hpp` linkt per Rezept. *(C++-CMake ✅ M0.)*
- **G2 Python-Echtheit:** jede gezeigte Python-Op läuft oder ist als C++-only markiert; keine rohen `ModuleNotFoundError`. *(Doku ✅ M0; Wiring offen — WS-2.)*
- **G3 Daten-I/O:** getesteter CSV-Loader + Export in C++ & Python; End-to-End-Beispiel. *(C++ ✅ Codex; Python offen.)*
- **G4 Doku-Ehrlichkeit:** kein werfender Call mit Output; eine kanonische API pro Sprache. *(Python ✅ M0.)*
- **G5 Robustheit:** nicht-finite Eingaben konsistent; Default-Validierung; Fehler mit Index/Werten.
- **G6 Inspektion:** jeder `*Result` + Space ausgebbar; ein Export-zu-Plot-Pfad. *(Result-`operator<<` ✅ M0.)*
- **G7 Skalierung:** Repräsentations-Doku; n² warnt statt OOM; parallele Pfade oder ehrliches Flag.
- **G8 Hygiene:** kein konstruierbarer Always-Throw ohne Markierung; Quarantäne markiert; keine SIGSEGV.
- **G9 CI:** Beispiele ausführen, Wheel-Matrix, Header-Link-Test, numpy-2-Smoke.

## 2. Workstreams (Kurzform; Details siehe Audit-Output)

- **WS-1 Build/Dist:** [x] ODR-Fix · [x] workflow-Link-Ehrlichkeit · [x] LAPACK-Target · [ ] pybind11-Submodul ≥3.0 (M1-Voraussetzung) · [ ] Versions-Matrix · [ ] Package-Manager.
- **WS-2 Python-Parität (M1, größter Hebel):** [ ] Kern-Verben an native Bindings (neighbors/groups/outliers/embed/reduce/compress/representatives/describe/compare) · [ ] Signatur-Metriken (TWED/EMD/SSIM/Kohonen/Sorensen) · [ ] correlation im Wheel oder OptionalDependencyError · [ ] `metric.available()`/`__all__` · [ ] numpy-first Rückgaben · [ ] numpy lazy-import.
- **WS-3 Daten-I/O:** [x] C++ read_csv/write_csv (Codex) · [ ] Python from_csv/from_numpy_file · [ ] Daten-raus (distance_matrix→np, to_csv, serialize) · [ ] Pickle · [ ] Beispiel mit realer CSV.
- **WS-4 Doku:** [x] Python-Banner + falsche Claims · [x] README-LAPACK · [ ] eine kanonische C++-Oberfläche · [ ] TUTORIAL.md · [ ] tutorial.cpp · [ ] generierte API-Referenz.
- **WS-5 Robustheit:** [ ] nicht-finite-Konsistenz (`require_finite`) · [ ] Default-Validierung · [ ] Fehler-Kontext · [ ] Entropy-Sentinel · [ ] EntropyResult-Wrapper · [ ] exact-Flag.
- **WS-6 Inspektion:** [x] `operator<<`/`summary` alle Results · [ ] Metadata-Adapter · [ ] Space/Matrix-Print · [ ] Viz/Export-Helfer · [ ] Progress-Callback · [ ] Python describe.
- **WS-7 Skalierung:** [ ] gepackte symm. Tabelle · [ ] Parallelität oder ehrliches Flag · [ ] inkrement. Index · [ ] KnnGraph-Traversierung · [ ] scale-Doku · [ ] out-of-core. *(range/lazy-table teilweise Codex.)*
- **WS-8 API-Konsistenz:** [ ] Verben nach `mtrc::` · [ ] Options re-exportieren · [ ] eine kanonische Oberfläche · [ ] Beispiele auf workflow.hpp/materialize · [ ] quickstart.hpp · [ ] compose/custom-Stubs.
- **WS-9 Metrik-Auswahl:** [ ] Custom-Metrik-Law + Doc · [ ] from_records-Warnung · [ ] Admission-Status/`[[deprecated]]` · [x] runtime discover_metrics (Codex) · [ ] Advisory-Helfer.
- **WS-10 Hygiene:** [ ] SOM/KOC/DSPCC compile-block · [ ] KOC aus engine.hpp · [ ] Quarantäne `[[deprecated]]` · [ ] Python Nicht-Metriken + RandomEMD-Guards · [ ] mapping.hpp-Banner · [ ] Scope-Doku.

## 3. Phasen-Roadmap

- **M0 — ✅ erledigt:** WS-1.1/1.2/1.3, WS-4, WS-6.
- **M1 — Python echt + I/O (Gates G2,G3):** WS-1.4 (pybind11), WS-2, WS-3 (Python). Exit: Python-Doc-Beispiele laufen.
- **M2 — Robust & lesbar (G5,G6):** WS-5, WS-6-Rest.
- **M3 — Skaliert & konsistent (G7,G4-C++):** WS-7, WS-8.
- **M4 — Poliert (G8,G9):** WS-9, WS-10, WS-1-Rest, CI-Gates.

## 4. CI-Gates
[ ] Beispiele ausführen · [ ] numpy-2-Smoke · [ ] Wheel-Matrix + import-Test · [ ] Header-Link-Test · [ ] Doc-Snippet-Test.

## 5. Risiko-Register
- Codex-Concurrency (clean + commits auf main) → Schutz-Branch, temp-index-Snapshots, vor jedem Schritt neu prüfen, Kollisionen meiden.
- WS-2 ist L und blockiert das Kern-Versprechen → verb-für-verb mergen.
- RandomEMD/Kohonen SIGSEGV in Python → WS-10 vor jedem Wheel-Release.
- Stale pybind11 2.4.3 → WS-1.4 zuerst.

## 6. Vollständigkeits-Nachweis
69 bestätigte Befunde abgedeckt: Python-Parität 8→WS-2, I/O 7→WS-3, Build 6→WS-1, Doku 6→WS-4,
Fehler 6→WS-5, Skalierung 6→WS-7, Inspektion 7→WS-6, Ergonomie 5→WS-8, Metrik-Auswahl 6→WS-9,
Abdeckung 11→WS-10. Widerlegt 2 (kein Task). Ohne Verdict 2 (in WS-7/WS-4).

*M0 ausgeführt & verifiziert 2026-06-22. Kanonische Kopie: Branch `claude/m0-production-readiness`.*
