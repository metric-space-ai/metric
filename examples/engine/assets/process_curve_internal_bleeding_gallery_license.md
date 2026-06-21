# Process Curve Internal Bleeding Gallery Slice

This directory contains a small derived gallery slice from the external UCR
Time Series Anomaly Detection datasets (2021) mirror on Figshare.

Source dataset:

- Title: UCR Time Series Anomaly Detection datasets (2021)
- DOI: https://doi.org/10.6084/m9.figshare.26410744.v1
- Figshare article: https://figshare.com/articles/dataset/UCR_Time_Series_Anomaly_Detection_datasets_2021_/26410744
- Source archive file id: `48036268`
- Source archive MD5: `4740e64e7a3242773b4570c1537095c1`
- License: CC BY 4.0, https://creativecommons.org/licenses/by/4.0/
- Citation: Lee, Daesoo (2024). UCR Time Series Anomaly Detection datasets
  (2021). figshare. Dataset. https://doi.org/10.6084/m9.figshare.26410744.v1

Derived files:

- `process_curve_internal_bleeding_gallery.csv`
- `docs/examples/assets/process-curve-internal-bleeding-gallery.svg`
- `docs/examples/assets/process-curve-internal-bleeding-report.svg`

Derivation:

- source members:
  - `AnomalyDatasets_2021/UCR_TimeSeriesAnomalyDatasets2021/FilesAreInHere/UCR_Anomaly_FullData/026_UCR_Anomaly_DISTORTEDInternalBleeding15_1700_5684_5854.txt`
  - `AnomalyDatasets_2021/UCR_TimeSeriesAnomalyDatasets2021/FilesAreInHere/UCR_Anomaly_FullData/028_UCR_Anomaly_DISTORTEDInternalBleeding17_1600_3198_3309.txt`
  - `AnomalyDatasets_2021/UCR_TimeSeriesAnomalyDatasets2021/FilesAreInHere/UCR_Anomaly_FullData/029_UCR_Anomaly_DISTORTEDInternalBleeding18_2300_4485_4587.txt`
  - `AnomalyDatasets_2021/UCR_TimeSeriesAnomalyDatasets2021/FilesAreInHere/UCR_Anomaly_FullData/031_UCR_Anomaly_DISTORTEDInternalBleeding20_2700_5759_5919.txt`
- 24 36-sample windows were extracted by source index
- values are copied without smoothing or rescaling
- labels identify the source window role for gallery narration only

The full 94 MB source archive is intentionally not vendored in this repository.
