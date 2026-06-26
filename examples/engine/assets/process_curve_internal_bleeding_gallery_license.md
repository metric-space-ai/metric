# Process Curve Internal Bleeding Hero-Scale Slice

This directory contains a derived hero-scale window slice from the external UCR
Time Series Anomaly Detection datasets (2021) mirror on Figshare.

Source dataset:

- Title: UCR Time Series Anomaly Detection datasets (2021)
- DOI: https://doi.org/10.6084/m9.figshare.26410744.v1
- Figshare article: https://figshare.com/articles/dataset/UCR_Time_Series_Anomaly_Detection_datasets_2021_/26410744
- Source archive file id: `48036268`
- Source archive MD5: `4740e64e7a3242773b4570c1537095c1`
- Direct archive used for expansion:
  https://www.cs.ucr.edu/~eamonn/time_series_data_2018/UCR_TimeSeriesAnomalyDatasets2021.zip
- Direct archive SHA-256:
  `ac4b991c701e620ae9cc5ebd57ae45593a36cc9c0b6ed5e3c4b7e466cf4783d4`
- Direct archive MD5: `11d539351d37c6fe3013e828ab89574a`
- License: CC BY 4.0, https://creativecommons.org/licenses/by/4.0/
- Citation: Lee, Daesoo (2024). UCR Time Series Anomaly Detection datasets
  (2021). figshare. Dataset. https://doi.org/10.6084/m9.figshare.26410744.v1

Derived files:

- `process_curve_internal_bleeding_gallery.csv`
- `docs/examples/assets/process-curve-internal-bleeding-gallery.svg`
- `docs/examples/assets/process-curve-internal-bleeding-report.svg`

Derivation:

- source members:
  - all `*InternalBleeding*.txt` members under
    `AnomalyDatasets_2021/UCR_TimeSeriesAnomalyDatasets2021/FilesAreInHere/UCR_Anomaly_FullData/`
- 28 source files were used
- 448 36-sample windows were extracted by deterministic source index
- values are copied without smoothing or rescaling
- labels identify source-window role for visual evidence and query contrast

The full source archive is intentionally not checked into this repository.
