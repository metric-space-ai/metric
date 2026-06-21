# Process Curve Power Demand Gallery Slice

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

- `process_curve_power_demand_gallery.csv`
- `docs/examples/assets/process-curve-power-demand-gallery.svg`
- `docs/examples/assets/process-curve-power-demand-report.svg`
- `docs/examples/assets/process-curve-power-demand-query-winners.svg`

Derivation:

- source members:
  - `AnomalyDatasets_2021/UCR_TimeSeriesAnomalyDatasets2021/FilesAreInHere/UCR_Anomaly_FullData/044_UCR_Anomaly_DISTORTEDPowerDemand1_9000_18485_18821.txt`
  - `AnomalyDatasets_2021/UCR_TimeSeriesAnomalyDatasets2021/FilesAreInHere/UCR_Anomaly_FullData/045_UCR_Anomaly_DISTORTEDPowerDemand2_14000_23357_23717.txt`
  - `AnomalyDatasets_2021/UCR_TimeSeriesAnomalyDatasets2021/FilesAreInHere/UCR_Anomaly_FullData/046_UCR_Anomaly_DISTORTEDPowerDemand3_16000_23405_23477.txt`
  - `AnomalyDatasets_2021/UCR_TimeSeriesAnomalyDatasets2021/FilesAreInHere/UCR_Anomaly_FullData/047_UCR_Anomaly_DISTORTEDPowerDemand4_18000_24005_24077.txt`
- 24 36-sample windows were extracted by source index
- values are copied without smoothing or rescaling
- labels identify the source window role for gallery narration only

The full 94 MB source archive is intentionally not vendored in this repository.
