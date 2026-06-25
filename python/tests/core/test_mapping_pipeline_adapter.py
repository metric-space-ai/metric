from metric import mapping_pipeline
from metric._impl import metric as native_metric


def test_mapping_pipeline_native_exports_are_present():
    assert hasattr(native_metric, "_metric_space_mapping_pipeline_plan")
    assert hasattr(native_metric, "_metric_space_mapping_pipeline_derive")


def test_pipeline_components_delegate_to_native_engine():
    components = mapping_pipeline.pipeline_components()

    assert components[0]["role"] == "space"
    assert any(component["role"] == "target_generator" for component in components)
    assert any(component["role"] == "coordinate_calibration" for component in components)


def test_derive_mapping_pipeline_delegates_to_native_engine():
    records = [
        [0.0, 0.0],
        [0.2, 0.1],
        [0.8, 0.3],
        [1.0, 0.5],
    ]

    artifact = mapping_pipeline.derive_mapping_pipeline(records, dimensions=1, calibration_steps=2)

    assert artifact.mapping == "parametric_diffusion_coordinates"
    assert artifact.strategy == "native_metric_diffusion_coordinate_solver"
    assert artifact.source_record_count == len(records)
    assert artifact.latent_dimension == 1
    assert artifact.neighbor_recall(1) == 1.0
    assert artifact.transform(records)
    assert artifact.inverse_transform([[0.0], [0.1]])


def test_mapping_pipeline_is_deterministic_seed_has_no_effect():
    # The native parametric coordinate derivation is deterministic by construction.
    # The reserved `seed` therefore cannot change the result.
    records = [
        [0.0, 0.0],
        [0.2, 0.1],
        [0.8, 0.3],
        [1.0, 0.5],
    ]

    first = mapping_pipeline.derive_mapping_pipeline(records, dimensions=1, calibration_steps=3, seed=1)
    second = mapping_pipeline.derive_mapping_pipeline(records, dimensions=1, calibration_steps=3, seed=987654321)

    assert first.transform(records) == second.transform(records)
