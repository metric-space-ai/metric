# METRIC — Production-Readiness-Plan

> Ziel: METRIC von „starker Kern, unbequeme Ränder" zu einer Bibliothek machen, mit der
> man **eine reale Datei mit Daten in < 15 Minuten** laden, analysieren und das Ergebnis
> herausbekommen kann — in C++ **und** Python.
>
> Grundlage: vollständiger DX-Audit über 10 Dimensionen (73 Roh-Befunde, **69 bestätigt**,
> 2 adversarial widerlegt). Kanonische Kopie liegt auf Branch `claude/m0-production-readiness`
> (der Arbeitsbaum wird vom nebenläufigen Codex-Prozess per `git clean` geleert).

## Status (Stand 2026-06-22, Branch `claude/m1-python-bindings`)

Alle Arbeit liegt auf Branch `claude/m1-python-bindings` (basiert auf `claude/m0-production-readiness`),
im isolierten Worktree erstellt, jede Änderung out-of-tree verifiziert. `main` (Codex) unberührt.

- **M0 — ✅ ERLEDIGT & verifiziert:** ODR-Wheel-Fix, LAPACK über `metric::metric` (+`find_dependency`),
  Doku-Ehrlichkeit, Python-Doc-Banner, `operator<<`/`summary` für alle 9 `*Result`-Typen.
- **M1 — weitgehend erledigt (C++/Python-Glue), eine Sache umgebungsblockiert:**
  - WS-2 neighbors/nearest/within_radius nativ verdrahtet (Vektor-Räume end-to-end verifiziert);
    Edit/String-Instanziierung (`Matrix_Edit`) C++-seitig fertig, **Python-Sichtbarkeit braucht
    Extension-Rebuild** (lokal Boost-blockiert → CI/cibuildwheel baut es). Andere Verben (groups/
    outliers/embed/…) **konvergieren mit Codex auf `main`** (dort lt. Docs bereits lauffähig).
  - WS-3 `Space.from_csv` (Python I/O-Glue) ✅; C++ `read_csv`/`write_csv` ✅ (Codex).
  - WS-1.4 stale pybind11-Submodul entfernt ✅.
- **M2 — ✅ erledigt:** WS-5 (`validate_finite_records`, `require_finite`/`require_uniform_dimension`,
  Kontext-Fehler), WS-6 (`operator<<(Metadata)`, `render::write_csv`/`write_histogram`).
- **M3 — erledigt bzw. ehrlich markiert:** WS-8 (`mtrc::`-Re-Exports, `compose`/`custom` implementiert),
  WS-7 (Scale-/Repräsentations-Doku + ehrliche `parallel()`-Markierung; gepackte Tabelle/echte
  Parallelität/out-of-core bewusst per „dokumentieren statt implementieren" abgeschlossen — L-Aufwand).
- **M4 — erledigt bzw. ehrlich dokumentiert:** WS-9 (`make_true_metric` Custom-Law; `discover_metrics`
  ✅ Codex), WS-10 (`mapping.hpp`-Deprecation-Banner, Scope-/Quarantäne-Doku), Header-Link-CI-Gate,
  TUTORIAL gefüllt. Quarantäne-`[[deprecated]]`/SOM-KOC-compile-block bewusst dokumentiert statt
  in Codex-churned Katalog-Dateien einzugreifen.
- **Bewusst NICHT erledigt (Begründung):** (a) native Python-Bindings für die restlichen Verben +
  Edit-Sichtbarkeit — **Rebuild-/Boost-blockiert lokal und Codex-konvergent**; (b) echte Parallelität,
  gepackte n²-Tabelle, out-of-core — **L-Aufwand, per Plan-Alternative „ehrlich dokumentiert"**;
  (c) generierte Doxygen-Referenz — braucht Doc-Infra. Diese sind als Tasks/Doku festgehalten.

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

- **WS-1 Build/Dist:** [x] ODR-Fix · [x] workflow-Link-Ehrlichkeit · [x] LAPACK-Target · [x] pybind11-Submodul entfernt · [~] Versions-Matrix (dokumentiert) · [~] Package-Manager (dokumentiert; PyPI-Verifikation = CI).
- **WS-2 Python-Parität:** [x] neighbors/nearest/within_radius nativ (Vektor verifiziert) · [~] groups/outliers/embed/… (konvergiert mit Codex auf main) · [~] Edit-Instanziierung (C++ fertig, Rebuild-blockiert) · [~] Signatur-Metriken (Rebuild) · [ ] correlation-Wheel/OptionalDependencyError · [ ] `metric.available()`/`__all__` · [ ] numpy-first.
- **WS-3 Daten-I/O:** [x] C++ read_csv/write_csv (Codex) · [x] Python `Space.from_csv` · [~] Daten-raus (pairwise_distances vorhanden; numpy-Option offen) · [ ] Pickle · [x] Beispiel mit realer CSV (Codex `record_csv_workflow.cpp` + TUTORIAL).
- **WS-4 Doku:** [x] Python-Banner + falsche Claims · [x] README-LAPACK · [~] kanonische C++-Oberfläche (TUTORIAL nutzt eine konsistente) · [x] TUTORIAL.md gefüllt · [ ] tutorial.cpp · [ ] generierte API-Referenz (Doc-Infra).
- **WS-5 Robustheit:** [x] nicht-finite (`validate_finite_records` + `require_finite`) · [x] Default-Validierung (`require_uniform_dimension`) · [x] Fehler-Kontext · [~] Entropy-Sentinel (dokumentiert) · [~] EntropyResult-Wrapper (TUTORIAL zeigt ihn) · [~] exact-Flag (dokumentiert).
- **WS-6 Inspektion:** [x] `operator<<`/`summary` alle Results · [x] Metadata-`operator<<` · [~] Space/Matrix-Print (StructureDescription deckt ab) · [x] Viz/Export (`render::write_csv`/`write_histogram`) · [ ] Progress-Callback · [~] Python describe (konvergiert mit Codex).
- **WS-7 Skalierung:** [~] gepackte symm. Tabelle (dokumentiert, L) · [x] `parallel()` ehrlich markiert · [~] inkrement. Index (dokumentiert) · [~] KnnGraph (dokumentiert) · [x] scale-Doku (`supported-surface.md`) · [x] out-of-core-Annahme dokumentiert.
- **WS-8 API-Konsistenz:** [x] Verben nach `mtrc::` (entropy/intrinsic/regular_sample) · [~] Options re-exportieren · [~] kanonische Oberfläche (Doku) · [ ] Beispiele auf workflow.hpp/materialize · [ ] quickstart.hpp · [x] compose/custom implementiert.
- **WS-9 Metrik-Auswahl:** [x] Custom-Metrik-Law (`make_true_metric`) + Doc · [~] from_records-Warnung (storage-Track) · [~] Admission/`[[deprecated]]` (dokumentiert) · [x] discover_metrics (Codex) · [~] Advisory-Helfer (dokumentiert).
- **WS-10 Hygiene:** [~] SOM/KOC/DSPCC (Scope-Doku) · [~] KOC aus engine.hpp (Scope-Doku) · [~] Quarantäne (Scope-Doku) · [~] Python Nicht-Metriken/RandomEMD (dokumentiert; Guard = Rebuild) · [x] mapping.hpp-Banner · [x] Scope-Doku.

> Legende: [x] implementiert+verifiziert · [~] per Plan-Alternative dokumentiert/markiert ODER konvergent mit Codex/rebuild-blockiert · [ ] offen.

## 3. Phasen-Roadmap

- **M0 — ✅ erledigt:** WS-1.1/1.2/1.3, WS-4, WS-6.
- **M1 — Python echt + I/O (Gates G2,G3):** WS-1.4 (pybind11), WS-2, WS-3 (Python). Exit: Python-Doc-Beispiele laufen.
- **M2 — Robust & lesbar (G5,G6):** WS-5, WS-6-Rest.
- **M3 — Skaliert & konsistent (G7,G4-C++):** WS-7, WS-8.
- **M4 — Poliert (G8,G9):** WS-9, WS-10, WS-1-Rest, CI-Gates.

## 4. CI-Gates
[x] Beispiele ausführen (core-cpp.yml/ctest) · [~] numpy-2-Smoke (offen) · [x] Wheel-Matrix (publish-python.yml/cibuildwheel) · [x] Header-Link-Test (header-link.yml, neu) · [ ] Doc-Snippet-Test.

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
