# Process-Curve Real Data Inventory

Date: 2026-06-26
Scope: read-only inspection, except this report.

## Result

The local machine does not currently have enough distinct real UCR source
windows to clear `missing-real-source-windows-for-500-record-hero`.

Inspectable local source evidence is still the checked-in 48-window derived
slice:

- 24 Power Demand source windows
- 24 Internal Bleeding source windows
- 16 derived downsampled query windows

The required 500 real source windows are not available locally from the searched
locations. The sibling metric worktrees contain duplicate copies of the same two
24-row gallery slice CSVs; they are not additional real source windows.

No datasets were synthesized, copied, extracted, or changed.

## Search Roots

Inspected required locations:

- `/Users/michaelwelsch/Documents/metric/examples/engine/assets/`
- `/Users/michaelwelsch/Downloads/`
- `/Users/michaelwelsch/Documents/`
- `/Users/michaelwelsch/Library/CloudStorage/`

## Candidate Files Found

### Current repo source slices

These are derived gallery slices, not the full UCR source archive.

| Path | Size | Inspectable count | Classification |
| --- | ---: | ---: | --- |
| `/Users/michaelwelsch/Documents/metric/examples/engine/assets/process_curve_power_demand_gallery.csv` | 10,622 bytes | 24 rows, 24 source windows, 36 samples/window | Derived UCR gallery slice |
| `/Users/michaelwelsch/Documents/metric/examples/engine/assets/process_curve_internal_bleeding_gallery.csv` | 9,940 bytes | 24 rows, 24 source windows, 36 samples/window | Derived UCR gallery slice |
| `/Users/michaelwelsch/Documents/metric/examples/engine/assets/process_curve_power_demand_gallery_license.md` | 1,918 bytes | metadata only | Attribution/license sidecar |
| `/Users/michaelwelsch/Documents/metric/examples/engine/assets/process_curve_internal_bleeding_gallery_license.md` | 1,881 bytes | metadata only | Attribution/license sidecar |

The sidecars cite UCR Time Series Anomaly Detection datasets (2021), Figshare
file id `48036268`, MD5 `4740e64e7a3242773b4570c1537095c1`, CC BY 4.0, and
state that the full roughly 94 MB source archive is intentionally not checked
into this repository.

### Current repo derived gallery/evidence CSVs

These files are generated evidence from the current 48 source windows and 16
queries. They are useful for inspection but do not add source windows.

| Path | Size | Inspectable count | Classification |
| --- | ---: | ---: | --- |
| `/Users/michaelwelsch/Documents/metric/docs/examples/assets/process-curve-external/summary.csv` | 327 bytes | 2 domain rows | Derived gallery summary |
| `/Users/michaelwelsch/Documents/metric/docs/examples/assets/process-curve-external/records.csv` | 17,872 bytes | 48 source records, 36 samples/record | Derived source-window export |
| `/Users/michaelwelsch/Documents/metric/docs/examples/assets/process-curve-external/queries.csv` | 5,855 bytes | 16 query rows, 18 samples/query | Derived query export |
| `/Users/michaelwelsch/Documents/metric/docs/examples/assets/process-curve-external/distances.csv` | 277,625 bytes | 2,304 pair-distance rows | Derived metric/baseline evidence |
| `/Users/michaelwelsch/Documents/metric/docs/examples/assets/process-curve-external/query-winners.csv` | 18,718 bytes | 48 curve rows; query, metric winner, baseline winner per query | Derived query-winner evidence |
| `/Users/michaelwelsch/Documents/metric/docs/examples/assets/process-curve-external/metric.visual.json` | 411,768 bytes | 64 total records: 48 source, 16 query; 1 relation over 48 source ids; 2,304 relation values | Derived native visual evidence |

Native diagnostic payload in `metric.visual.json` reports:

- `source_record_count`: 48
- `query_record_count`: 16
- `relation_pair_count`: 2304
- `metric_correct`: 16
- `vector_baseline_mismatches`: 16
- `power_demand`: 24 records, 8 queries
- `internal_bleeding`: 24 records, 8 queries

### Sibling metric worktrees

These are duplicate derived slices with the same sizes and row counts. They do
not increase the available source-window inventory.

| Path | Size | Inspectable count | Classification |
| --- | ---: | ---: | --- |
| `/Users/michaelwelsch/Documents/metric-wt-level1-modify/examples/engine/assets/process_curve_power_demand_gallery.csv` | 10,622 bytes | 24 rows, 24 source windows, 36 samples/window | Duplicate derived UCR gallery slice |
| `/Users/michaelwelsch/Documents/metric-wt-level1-modify/examples/engine/assets/process_curve_internal_bleeding_gallery.csv` | 9,940 bytes | 24 rows, 24 source windows, 36 samples/window | Duplicate derived UCR gallery slice |
| `/Users/michaelwelsch/Documents/metric-push-docs/examples/engine/assets/process_curve_power_demand_gallery.csv` | 10,622 bytes | 24 rows, 24 source windows, 36 samples/window | Duplicate derived UCR gallery slice |
| `/Users/michaelwelsch/Documents/metric-push-docs/examples/engine/assets/process_curve_internal_bleeding_gallery.csv` | 9,940 bytes | 24 rows, 24 source windows, 36 samples/window | Duplicate derived UCR gallery slice |

## Full UCR Source Search

No full local UCR source archive was found by filename patterns for:

- `*_UCR_Anomaly_*`
- `*UCR_Time_Series_Anomaly_Detection*`
- `*UCR_TimeSeriesAnomaly*`
- `*UCR*Anomaly*2021*`
- `*48036268*`

No extracted UCR source files were found for the eight source members referenced
by the checked-in slices, including examples such as:

- `044_UCR_Anomaly_DISTORTEDPowerDemand1_9000_18485_18821.txt`
- `026_UCR_Anomaly_DISTORTEDInternalBleeding15_1700_5684_5854.txt`

Archive member inspection checked 307 local `.zip`, `.tar`, `.tar.gz`, and
`.tgz` files under the required roots using `zipinfo -1` and `tar -tf`. No
archive member matched UCR anomaly source paths, Figshare id `48036268`, or
process-curve gallery CSV names.

The initial broad searches also found many unrelated CSV/ZIP examples under
Downloads, Documents, and CloudStorage. They did not contain UCR/process-curve
candidate filenames and were not counted.

## Blocker Decision

`missing-real-source-windows-for-500-record-hero` cannot be cleared locally.

Reason: only 48 distinct real source windows are inspectable. The hero needs at
least 500 real source windows in the native source relation. The current
evidence is short by at least 452 distinct real source windows.

Do not clear acceptance, mark the hero scale-ready, pad with queries, duplicate
existing windows, or synthesize curves.

## Next Step If Enough Data Exists Later

If the full UCR archive or extracted source tree becomes available locally, the
next implementation step is:

1. Update the native process-curve visual exporter to load/sample at least 500
   distinct licensed real source windows from the real UCR source files while
   preserving source file id, start/end indices, role label, and raw values.
2. Generate held-out/downsampled query windows from real source windows.
3. Keep the metric, padded-vector baseline, nearest-neighbor winner, margin,
   mismatch, dense source relation, kNN graph, coordinate, and diagnostics
   computations in native C++.
4. Rerun the existing native visual export and validation checks.

The primary code target for that step is:

```text
/Users/michaelwelsch/Documents/metric/examples/engine/process_curve_external_visual_export.cpp
```

## External Data Requirement

To unblock the hero, provide one of:

- the full licensed UCR Time Series Anomaly Detection datasets (2021) archive
  from Figshare article `UCR_Time_Series_Anomaly_Detection_datasets_2021_`,
  source file id `48036268`, MD5 `4740e64e7a3242773b4570c1537095c1`, CC BY 4.0;
  or
- an extracted equivalent source tree containing
  `AnomalyDatasets_2021/UCR_TimeSeriesAnomalyDatasets2021/FilesAreInHere/UCR_Anomaly_FullData/`;
  or
- another properly licensed real process-curve corpus with enough records.

The external data must support at least 500 distinct real source windows and
must preserve enough provenance for each window:

- source file identifier
- start/end indices
- raw time-series values
- role/condition labels or a defensible labeling derivation
- enough held-out/downsampled query windows to prove metric winner vs vector
  baseline mismatch

## Commands Run

All commands were read-only except the final report write.

```bash
pwd && sed -n '1,240p' docs/visual/agent-tasks/process-curve-real-data-inventory.md
```

```bash
find examples/engine/assets -maxdepth 4 -type f \( -iname '*ucr*' -o -iname '*anomaly*' -o -iname '*process*curve*' -o -iname '*.csv' -o -iname '*.zip' -o -iname '*.tar' -o -iname '*.tar.gz' -o -iname '*.tgz' \) -print
```

```bash
find /Users/michaelwelsch/Downloads -maxdepth 5 -type f \( -iname '*ucr*' -o -iname '*anomaly*' -o -iname '*time*series*' -o -iname '*.csv' -o -iname '*.zip' -o -iname '*.tar' -o -iname '*.tar.gz' -o -iname '*.tgz' \) -print
```

```bash
find /Users/michaelwelsch/Documents -maxdepth 6 -type f \( -iname '*ucr*' -o -iname '*anomaly*' -o -iname '*time*series*' -o -iname '*process*curve*' -o -iname '*.csv' -o -iname '*.zip' -o -iname '*.tar' -o -iname '*.tar.gz' -o -iname '*.tgz' \) -print
```

```bash
find /Users/michaelwelsch/Library/CloudStorage -maxdepth 6 -type f \( -iname '*ucr*' -o -iname '*anomaly*' -o -iname '*time*series*' -o -iname '*process*curve*' -o -iname '*.csv' -o -iname '*.zip' -o -iname '*.tar' -o -iname '*.tar.gz' -o -iname '*.tgz' \) -print
```

```bash
rg -n "UCR|Time Series Anomaly|TimeSeriesAnomaly|missing-real-source-windows|500|source windows|process-curve-external" docs examples visual -S
```

This command was too broad and matched large generated JSON; follow-up
inspection used targeted commands instead.

```bash
find /Users/michaelwelsch/Downloads /Users/michaelwelsch/Documents /Users/michaelwelsch/Library/CloudStorage -maxdepth 8 \( -iname '*UCR*' -o -iname '*TimeSeriesAnomaly*' -o -iname '*Time*Series*Anomaly*' -o -iname '*AnomalyDatasets*' -o -iname '*AnomalyDetection*' -o -iname '*process_curve*gallery*.csv' -o -iname '*process-curve*gallery*.csv' -o -iname 'records.csv' -o -iname 'queries.csv' \) -print
```

```bash
find examples/engine/assets docs/examples/assets/process-curve-external -maxdepth 2 -type f -print0 | xargs -0 stat -f '%N\t%z bytes'
```

```bash
find docs/visual/reports docs/visual/agent-tasks -maxdepth 1 -type f -name 'process-curve-*.md' -print
```

```bash
sed -n '1,240p' docs/visual/reports/process-curve-external-scale-exporter.md
```

```bash
sed -n '1,220p' docs/examples/process-curve-external-gallery.md
```

```bash
sed -n '1,140p' examples/engine/process_curve_external_gallery.cpp
```

```bash
head -n 5 examples/engine/assets/process_curve_power_demand_gallery.csv && head -n 5 examples/engine/assets/process_curve_internal_bleeding_gallery.csv && head -n 5 docs/examples/assets/process-curve-external/records.csv && head -n 5 docs/examples/assets/process-curve-external/queries.csv && cat docs/examples/assets/process-curve-external/summary.csv
```

```bash
find /Users/michaelwelsch/Downloads /Users/michaelwelsch/Documents /Users/michaelwelsch/Library/CloudStorage \( -name node_modules -o -name .git -o -name build -o -name target -o -name .next \) -prune -o -type f \( -name '*_UCR_Anomaly_*' -o -name '*UCR_Time_Series_Anomaly_Detection*' -o -name '*UCR_TimeSeriesAnomaly*' -o -name '*UCR*Anomaly*2021*' -o -name '*48036268*' \) -print
```

Result: no output.

```bash
find /Users/michaelwelsch/Downloads /Users/michaelwelsch/Documents /Users/michaelwelsch/Library/CloudStorage \( -name node_modules -o -name .git -o -name build -o -name target -o -name .next \) -prune -o -type f \( -name '044_UCR_Anomaly_DISTORTEDPowerDemand1_9000_18485_18821.txt' -o -name '045_UCR_Anomaly_DISTORTEDPowerDemand2_14000_23357_23717.txt' -o -name '046_UCR_Anomaly_DISTORTEDPowerDemand3_16000_23405_23477.txt' -o -name '047_UCR_Anomaly_DISTORTEDPowerDemand4_18000_24005_24077.txt' -o -name '026_UCR_Anomaly_DISTORTEDInternalBleeding15_1700_5684_5854.txt' -o -name '028_UCR_Anomaly_DISTORTEDInternalBleeding17_1600_3198_3309.txt' -o -name '029_UCR_Anomaly_DISTORTEDInternalBleeding18_2300_4485_4587.txt' -o -name '031_UCR_Anomaly_DISTORTEDInternalBleeding20_2700_5759_5919.txt' \) -print
```

Result: no output.

```bash
find /Users/michaelwelsch/Downloads /Users/michaelwelsch/Documents /Users/michaelwelsch/Library/CloudStorage \( -name node_modules -o -name .git -o -name build -o -name target -o -name .next \) -prune -o -type f \( -iname '*process_curve*gallery*.csv' -o -iname '*process-curve*gallery*.csv' \) -print0 | xargs -0 stat -f '%N\t%z bytes'
```

```bash
find /Users/michaelwelsch/Downloads /Users/michaelwelsch/Documents /Users/michaelwelsch/Library/CloudStorage \( -name node_modules -o -name .git -o -name build -o -name target -o -name .next \) -prune -o -type f \( -iname '*.zip' -o -iname '*.tar' -o -iname '*.tar.gz' -o -iname '*.tgz' \) -print | wc -l
```

Result: `307`.

```bash
bash -lc 'find /Users/michaelwelsch/Downloads /Users/michaelwelsch/Documents /Users/michaelwelsch/Library/CloudStorage \( -name node_modules -o -name .git -o -name build -o -name target -o -name .next \) -prune -o -type f \( -iname "*.zip" -o -iname "*.tar" -o -iname "*.tar.gz" -o -iname "*.tgz" \) -print0 | while IFS= read -r -d "" archive; do case "$archive" in *.[Zz][Ii][Pp]) entries=$(zipinfo -1 "$archive" 2>/dev/null | rg -i "(^|/)[0-9]{3}_UCR_Anomaly_|UCR[_ -]?Time[_ -]?Series[_ -]?Anomaly|Anomaly[_ -]?Detection.*2021|48036268|process[_-]curve.*gallery\\.csv" || true) ;; *.[Tt][Aa][Rr]|*.[Tt][Aa][Rr].[Gg][Zz]|*.[Tt][Gg][Zz]) entries=$(tar -tf "$archive" 2>/dev/null | rg -i "(^|/)[0-9]{3}_UCR_Anomaly_|UCR[_ -]?Time[_ -]?Series[_ -]?Anomaly|Anomaly[_ -]?Detection.*2021|48036268|process[_-]curve.*gallery\\.csv" || true) ;; *) entries="" ;; esac; if [ -n "$entries" ]; then printf "ARCHIVE\t%s\n%s\n" "$archive" "$entries"; fi; done'
```

Result: no output.

```bash
find /Users/michaelwelsch/Documents/metric /Users/michaelwelsch/Documents/metric-wt-level1-modify /Users/michaelwelsch/Documents/metric-push-docs \( -name node_modules -o -name .git -o -name build -o -name target -o -name .next \) -prune -o -type f \( -path '*/docs/examples/assets/process-curve-external/*.csv' -o -iname '*process_curve*gallery*.csv' -o -iname '*process-curve*gallery*.csv' \) -print0 | xargs -0 stat -f '%N\t%z bytes'
```

```bash
node - <<'NODE'
const fs = require('fs');
const files = [
  '/Users/michaelwelsch/Documents/metric/examples/engine/assets/process_curve_power_demand_gallery.csv',
  '/Users/michaelwelsch/Documents/metric/examples/engine/assets/process_curve_internal_bleeding_gallery.csv',
  '/Users/michaelwelsch/Documents/metric/docs/examples/assets/process-curve-external/summary.csv',
  '/Users/michaelwelsch/Documents/metric/docs/examples/assets/process-curve-external/records.csv',
  '/Users/michaelwelsch/Documents/metric/docs/examples/assets/process-curve-external/queries.csv',
  '/Users/michaelwelsch/Documents/metric/docs/examples/assets/process-curve-external/distances.csv',
  '/Users/michaelwelsch/Documents/metric/docs/examples/assets/process-curve-external/query-winners.csv',
  '/Users/michaelwelsch/Documents/metric-wt-level1-modify/examples/engine/assets/process_curve_power_demand_gallery.csv',
  '/Users/michaelwelsch/Documents/metric-wt-level1-modify/examples/engine/assets/process_curve_internal_bleeding_gallery.csv',
  '/Users/michaelwelsch/Documents/metric-push-docs/examples/engine/assets/process_curve_power_demand_gallery.csv',
  '/Users/michaelwelsch/Documents/metric-push-docs/examples/engine/assets/process_curve_internal_bleeding_gallery.csv',
];
function parseCsv(text) {
  const rows = [];
  let row = [], field = '', inQuotes = false;
  for (let i = 0; i < text.length; i++) {
    const ch = text[i];
    if (inQuotes) {
      if (ch === '"') {
        if (text[i + 1] === '"') { field += '"'; i++; }
        else inQuotes = false;
      } else field += ch;
    } else if (ch === '"') inQuotes = true;
    else if (ch === ',') { row.push(field); field = ''; }
    else if (ch === '\n') { row.push(field); rows.push(row); row = []; field = ''; }
    else if (ch !== '\r') field += ch;
  }
  if (field.length || row.length) { row.push(field); rows.push(row); }
  return rows.filter(r => r.length > 1 || r[0] !== '');
}
for (const file of files) {
  const stat = fs.statSync(file);
  const rows = parseCsv(fs.readFileSync(file, 'utf8'));
  const header = rows[0] || [];
  const data = rows.slice(1);
  const valueIdx = header.indexOf('values');
  const queryValuesIdx = header.indexOf('query_values');
  const sampleCountIdx = header.indexOf('sample_count');
  const domainIdx = header.indexOf('domain');
  const domains = domainIdx >= 0 ? [...new Set(data.map(r => r[domainIdx]))].join('|') : '';
  const sampleCounts = new Set();
  for (const row of data) {
    if (sampleCountIdx >= 0 && row[sampleCountIdx]) sampleCounts.add(row[sampleCountIdx]);
    const idx = valueIdx >= 0 ? valueIdx : queryValuesIdx;
    if (idx >= 0 && row[idx]) sampleCounts.add(String(row[idx].trim().split(/\s+/).filter(Boolean).length));
  }
  console.log(JSON.stringify({file, bytes: stat.size, header, dataRows: data.length, domains, sampleCounts: [...sampleCounts].sort((a,b)=>Number(a)-Number(b))}));
}
NODE
```

```bash
node - <<'NODE'
const fs = require('fs');
const file = '/Users/michaelwelsch/Documents/metric/docs/examples/assets/process-curve-external/metric.visual.json';
const doc = JSON.parse(fs.readFileSync(file, 'utf8'));
const records = Array.isArray(doc.records) ? doc.records : [];
const relations = Array.isArray(doc.relations) ? doc.relations : [];
const diagnostics = Array.isArray(doc.diagnostics) ? doc.diagnostics : [];
const sourceRecords = records.filter(r => r.record_type === 'process_window' || (r.id && !String(r.id).startsWith('downsampled_')));
const queryRecords = records.filter(r => String(r.id || '').startsWith('downsampled_'));
console.log(JSON.stringify({file, bytes: fs.statSync(file).size, totalRecords: records.length, sourceRecords: sourceRecords.length, queryRecords: queryRecords.length, relations: relations.length, relationRecordIds: relations.map(r => (r.record_ids || []).length), relationValues: relations.map(r => (r.values || []).length), diagnostics: diagnostics.map(d => d.payload || d)}));
NODE
```

```bash
sed -n '1,200p' examples/engine/assets/process_curve_power_demand_gallery_license.md && sed -n '1,200p' examples/engine/assets/process_curve_internal_bleeding_gallery_license.md
```

```bash
rg -l -i "UCR_TimeSeriesAnomalyDatasets2021|UCR_Time_Series_Anomaly_Detection_2021|UCR Time Series Anomaly Detection|48036268|4740e64e|UCR_Anomaly_DISTORTED" /Users/michaelwelsch/Downloads /Users/michaelwelsch/Documents /Users/michaelwelsch/Library/CloudStorage --glob '!node_modules/**' --glob '!.git/**' --glob '!build/**' --glob '!target/**' --glob '!.next/**' --glob '!output/**' --glob '!runtime/**' --glob '!*.png' --glob '!*.jpg' --glob '!*.jpeg' --glob '!*.svg' --glob '!*.o' --glob '!*.a' --glob '!*.dylib' --glob '!*.zip' --glob '!*.tar' --glob '!*.tgz' --glob '!*.tar.gz'
```

This command was aborted after CloudStorage/Documents timeout noise and was not
used for the decision.

```bash
find /Users/michaelwelsch/Downloads /Users/michaelwelsch/Documents /Users/michaelwelsch/Library/CloudStorage \( -name node_modules -o -name .git -o -name build -o -name target -o -name .next -o -name output -o -name runtime \) -prune -o -type f \( -iname '*.csv' -o -iname '*.txt' \) -print0 | xargs -0 rg -l "UCR_Anomaly_DISTORTED"
```

This command was also aborted after timeout noise and was not used for the
decision.

```bash
git status --short
```

This showed many pre-existing unrelated changes before this report was written.
