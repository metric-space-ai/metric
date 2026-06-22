from metric import mapping_pipeline
from metric._impl import metric as native_metric


def test_mapping_pipeline_native_exports_are_present():
    assert hasattr(native_metric, "_metric_space_mapping_pipeline_plan")
    assert hasattr(native_metric, "_metric_space_mapping_pipeline_fit")


def test_pipeline_components_delegate_to_native_engine():
    components = mapping_pipeline.pipeline_components()

    assert components[0]["role"] == "space"
    assert any(component["role"] == "target_generator" for component in components)
    assert any(component["name"] == "native_dnn_autoencoder_trainer" for component in components)


def test_fit_mapping_pipeline_delegates_to_native_engine():
    records = [
        [0.0, 0.0],
        [0.2, 0.1],
        [0.8, 0.3],
        [1.0, 0.5],
    ]

    model = mapping_pipeline.fit_mapping_pipeline(records, dimensions=1, epochs=2)

    assert model.mapping == "native_phate_autoencoder"
    assert model.strategy == "native_dnn_phate_ae"
    assert model.source_record_count == len(records)
    assert model.latent_dimension == 1
    assert model.neighbor_recall(1) == 1.0
    assert model.transform(records)
    assert model.inverse_transform([[0.0], [0.1]])


def test_mapping_pipeline_is_deterministic_seed_has_no_effect():
    # The native PHATE-AE map is deterministic by construction (closed-form weight
    # initialization, no batch shuffling). The advertised `seed` therefore cannot
    # change the result -- pin that contract so the no-op knob is documented AND
    # tested rather than silently advertised as a reproducibility control.
    records = [
        [0.0, 0.0],
        [0.2, 0.1],
        [0.8, 0.3],
        [1.0, 0.5],
    ]

    first = mapping_pipeline.fit_mapping_pipeline(records, dimensions=1, epochs=3, seed=1)
    second = mapping_pipeline.fit_mapping_pipeline(records, dimensions=1, epochs=3, seed=987654321)

    assert first.transform(records) == second.transform(records)
