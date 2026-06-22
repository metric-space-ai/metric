#include <cassert>
#include <cmath>
#include <sstream>

#include <metric/numeric.hpp>

#include "numeric_test_helpers.hpp"

// Exercises the sparse mutation contracts that production code relies on but that had no smoke
// coverage: CompressedMatrix::insert / erase, CompressedVector::insert / erase, and an Archive
// round-trip for a sparse vector (the serialization test only covered the sparse matrix).
int main()
{
	using mtrc::test::close_to;

	// --- CompressedMatrix insert / erase ----------------------------------------------------
	mtrc::numeric::CompressedMatrix<double> matrix(3, 3);
	matrix.reserve(4);
	matrix.insert(0, 1, 2.0);
	matrix.insert(1, 2, -3.0);
	matrix.insert(2, 0, 4.0);
	assert(matrix.nonZeros() == 3);
	assert(close_to(matrix(0, 1), 2.0));
	assert(close_to(matrix(1, 2), -3.0));
	assert(close_to(matrix(2, 0), 4.0));

	// insert() must reject an already-present element with the documented exception.
	bool threw = false;
	try {
		matrix.insert(0, 1, 9.0);
	} catch (const std::invalid_argument &) {
		threw = true;
	}
	assert(threw);

	matrix.erase(1, 2);
	assert(matrix.nonZeros() == 2);
	assert(close_to(matrix(1, 2), 0.0));
	assert(close_to(matrix(0, 1), 2.0));

	// --- CompressedVector insert / erase ----------------------------------------------------
	mtrc::numeric::CompressedVector<double> vector(6);
	vector.reserve(3);
	vector.insert(1, 1.5);
	vector.insert(4, -2.5);
	assert(vector.nonZeros() == 2);
	assert(close_to(vector[1], 1.5));
	assert(close_to(vector[4], -2.5));

	bool vector_threw = false;
	try {
		vector.insert(1, 7.0);
	} catch (const std::invalid_argument &) {
		vector_threw = true;
	}
	assert(vector_threw);

	vector.erase(4);
	assert(vector.nonZeros() == 1);
	assert(close_to(vector[4], 0.0));
	assert(close_to(vector[1], 1.5));

	// --- Archive round-trip for a sparse vector ---------------------------------------------
	mtrc::numeric::CompressedVector<double> source(8);
	source.reserve(3);
	source.append(0, 3.0);
	source.append(3, -1.0);
	source.append(7, 5.0);

	std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
	{
		mtrc::numeric::Archive<std::stringstream> writer(stream);
		writer << source;
		assert(writer);
	}
	stream.clear();
	stream.seekg(0);

	mtrc::numeric::CompressedVector<double> restored;
	{
		mtrc::numeric::Archive<std::stringstream> reader(stream);
		reader >> restored;
		assert(reader);
	}
	assert(restored.size() == source.size());
	assert(restored.nonZeros() == source.nonZeros());
	assert(close_to(restored[0], 3.0));
	assert(close_to(restored[3], -1.0));
	assert(close_to(restored[7], 5.0));
	assert(close_to(restored[1], 0.0));

	return 0;
}
