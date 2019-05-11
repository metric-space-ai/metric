#pragma once


#include <stdlib.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <math.h>
#include <time.h>
#include <limits>


namespace metric {

	namespace mapping {
		namespace SOM_details {

			class Graph {
				public:
					explicit Graph(size_t nodesNumber);
					~Graph();

					size_t getNodesNumber();
					bool isValid();

				std::vector<std::vector<size_t>> getNeighbours(const size_t nodeIndex, const size_t maxDeep);

				protected:
					size_t nodesNumber;
					bool valid;
					std::vector<std::vector<size_t>> edges;

					size_t modularPow(const size_t base, const size_t exponent, const size_t modulus);

					void buildEdges(const std::vector<std::pair<size_t, size_t>> &edgesPairs);

					void neighboursWalk(const size_t index, std::unordered_map<size_t, size_t> &indexes, size_t deep,
										const size_t maxDeep);
			};

			class Grid4 : public Graph {
				public:
					explicit Grid4(size_t nodesNumber);
					Grid4(size_t width, size_t height);
				private:
					void construct(size_t width, size_t height);
			};

			class Grid6 : public Graph {
			public:
				explicit Grid6(size_t nodesNumber);
				Grid6(size_t width, size_t height);
			private:
				void construct(size_t width, size_t height);
			};

			class Grid8 : public Graph {
				public:
					explicit Grid8(size_t nodesNumber);
					Grid8(size_t width, size_t height);
				private:
					void construct(size_t width, size_t height);
			};

			class LPS : public Graph {
				public:
					explicit LPS(size_t nodesNumber);
				private:
					bool miller_rabin_pass(const size_t a, const size_t s,
										const size_t d, const size_t nodesNumber);

					bool millerRabin(const size_t nodesNumber);
			};

			class Paley : public Graph {
				public:
					explicit Paley(size_t nodesNumber);
			};

			class Margulis : public Graph {
				public:
					explicit Margulis(size_t nodesNumber);
			};
		}
	}
}


namespace metric {

	namespace mapping {
		namespace SOM_details {

			Graph::Graph(size_t nodesNumber) : nodesNumber(nodesNumber), valid(false) {};

			Graph::~Graph() = default;

			size_t Graph::getNodesNumber()
			{
				return nodesNumber;
			}

			bool Graph::isValid() {
				return valid;
			}

			std::vector<std::vector<size_t>> Graph::getNeighbours(const size_t nodeIndex, const size_t maxDeep)
			{
				std::vector<std::vector<size_t>> neighboursList(maxDeep + 1);			
				
				std::unordered_map<size_t, size_t> indexes;
				neighboursWalk(nodeIndex, indexes, 0, maxDeep);

				for (const auto& [index, deep]: indexes) {
					neighboursList[deep].push_back(index);
				}

				return neighboursList;
			}

			size_t Graph::modularPow(const size_t base, const size_t exponent, const size_t modulus)
			{
				if (modulus == 1) {
					return 1;
				}
				size_t c = 1;
				for (size_t e = 0; e < exponent; ++e) {
					c = (c * base) % modulus;
				}

				return c;
			}

			void Graph::buildEdges(const std::vector<std::pair<size_t, size_t>> &edgesPairs)
			{
				std::vector<std::unordered_set<size_t>> edgesSets(nodesNumber);

				for (const auto& [i, j]: edgesPairs) {
					if (i != j) {
						edgesSets[i].insert(j);
						edgesSets[j].insert(i);
					}
				}

				edges.resize(nodesNumber);

                for (auto i = 0; i < edgesSets.size(); ++i) {
					for (const auto& j: edgesSets[i]) {
						edges[i].push_back(j);
					}
				}
			}

			void Graph::neighboursWalk(const size_t index, std::unordered_map<size_t, size_t> &indexes, size_t deep,
									  const size_t maxDeep)
			{
				if (deep > maxDeep) {
					return;
				}

				auto iterator = indexes.find(index);
				if (iterator != indexes.end()) {
					if (iterator->second <= deep) {
						return;
					}
				}
				
				indexes[index] = deep;

				for (const auto& i: edges[index]) {
					neighboursWalk(i, indexes, deep + 1, maxDeep);
				}
			}

			Grid4::Grid4(size_t nodesNumber) : Graph(nodesNumber)
			{
				int s = sqrt(nodesNumber);
				if ((s * s) != nodesNumber) {
					valid = false;
				} else {
					construct(s, s);
				}
			}

			Grid4::Grid4(size_t width, size_t height) : Graph(width * height)
			{
				construct(width, height);
			}

			void Grid4::construct(size_t width, size_t height)
			{
				edges.resize(width * height);

				for (size_t i = 0; i < height; ++i) {
					for (size_t j = 0; j < width; ++j) {

						int ii0 = 0, ii1 = 0;
						int jj0 = 0, jj1 = 0;

						if (i > 0) {
							ii0 = -1;
						}
						if (j > 0) {
							jj0 = -1;
						}

						if (i < height - 1) {
							ii1 = 1;
						}
						if (j < width - 1) {
							jj1 = 1;
						}

						for (int ii = ii0; ii <= ii1; ++ii) {
							for (int jj = jj0; jj <= jj1; ++jj) {
								if ((ii == 0) or (jj == 0)) {
									edges[i * width + j].push_back((i + ii) * width + (j + jj));
								}
							}
						}
					}
				}

				valid = true;
			}

			Grid6::Grid6(size_t nodesNumber) : Graph(nodesNumber)
			{
				int s = sqrt(nodesNumber);
				if ((s * s) != nodesNumber) {
					valid = false;
				} else {
					construct(s, s);
				}
			}

			Grid6::Grid6(size_t width, size_t height) : Graph(width * height)
			{
				construct(width, height);
			}

			void Grid6::construct(size_t width, size_t height)
			{
				edges.resize(width * height);

				for (size_t i = 0; i < height; ++i) {
					for (size_t j = 0; j < width; ++j) {

						bool odd = true;
						if (i % 2 == 0) {
                            odd = false;
						}

						bool up = true;
						if (i == 0) {
							up = false;
						}
						bool down = true;
						if (i == height - 1) {
							down = false;
						}
						bool left = true;
						if (j == 0) {
							left = false;
						}

						bool right = true;
                        if (j == width - 1) {
                        	right = false;
                        }

						if (up) {
							edges[i * width + j].push_back((i - 1) * width + (j + 0));
						}
						if (down) {
							edges[i * width + j].push_back((i + 1) * width + (j + 0));
						}
						if (left) {
							edges[i * width + j].push_back((i + 0) * width + (j - 1));
						}
						if (right) {
							edges[i * width + j].push_back((i + 0) * width + (j + 1));
						}

						if (!odd and left) {
							if (up) {
								edges[i * width + j].push_back((i - 1) * width + (j - 1));
							}
							if (down) {
								edges[i * width + j].push_back((i + 1) * width + (j - 1));
							}
						}

						if (odd and right) {
							if (up) {
								edges[i * width + j].push_back((i - 1) * width + (j + 1));
							}
							if (down)
								edges[i * width + j].push_back((i + 1) * width + (j + 1));
						}
					}
				}

				valid = true;
			}

			Grid8::Grid8(size_t nodesNumber) : Graph(nodesNumber)
			{
				int s = sqrt(nodesNumber);
				if ((s * s) != nodesNumber) {
					valid = false;
				} else {
					construct(s, s);
				}
			}

			Grid8::Grid8(size_t width, size_t height) : Graph(width * height)
			{
				construct(width, height);
			}

			void Grid8::construct(size_t width, size_t height)
			{
				edges.resize(width * height);

				for (size_t i = 0; i < height; ++i) {
					for (size_t j = 0; j < width; ++j) {

						int ii0 = 0, ii1 = 0;
						int jj0 = 0, jj1 = 0;

						if (i > 0) {
							ii0 = -1;
						}
						if (j > 0) {
							jj0 = -1;
						}

						if (i < height - 1) {
							ii1 = 1;
						}
						if (j < width - 1) {
							jj1 = 1;
						}

						for (int ii = ii0; ii <= ii1; ++ii) {
							for (int jj = jj0; jj <= jj1; ++jj) {
								if ((ii != 0) or (jj != 0)) {
									edges[i * width + j].push_back((i + ii) * width + (j + jj));
								}
							}
						}
					}
				}

				valid = true;
			}

			LPS::LPS(size_t nodesNumber) : Graph(nodesNumber)
			{
				if (!millerRabin(nodesNumber)) {
					return;
				}

				std::vector<std::pair<size_t, size_t>> edgesPairs;

				for (size_t i = 0; i < nodesNumber; ++i) {
					if (i == 0) {
						edgesPairs.emplace_back(0, nodesNumber - 1);
						edgesPairs.emplace_back(0, 1);
					} else {
						edgesPairs.emplace_back(i, i - 1);
						edgesPairs.emplace_back(i, (i + 1) % nodesNumber);
						edgesPairs.emplace_back(i, modularPow(i, nodesNumber - 2, nodesNumber));
					}
				}

				buildEdges(edgesPairs);

				valid = true;
			}

			bool LPS::millerRabin(const size_t nodesNumber)
			{
				srand(time(NULL));

				auto d = nodesNumber - 1;
				auto s = 0;

				while (d % 2 == 0) {
					d >>= 1;
					s += 1;
				}

				for (int repeat = 0; repeat < 20; ++repeat) {
					size_t a = 0;
					while (a == 0) {
						a = rand() % nodesNumber;
					}

					if (!miller_rabin_pass(a, s, d, nodesNumber)) {
						return false;
					}
				}
				return true;
			}

			bool LPS::miller_rabin_pass(const size_t a, const size_t s,
										const size_t d, const size_t nodesNumber)
			{
				auto p = size_t(pow(a, d)) % nodesNumber;
				if (p == 1) {
					return true;
				}

				for (auto i = 0; i < s - 1; ++i) {
					if (p == nodesNumber - 1) {
						return true;
					}
					p = (p * p) % nodesNumber;
				}

				return p == nodesNumber - 1;
			}

			Paley::Paley(size_t nodesNumber) : Graph(nodesNumber)
			{
				if (nodesNumber % 4 != 1) {
					return;
				}

				std::vector<std::pair<size_t, size_t>> edgesPairs;

				std::vector<size_t> squareList;

				size_t l = (nodesNumber - 1) / 2;
				squareList.reserve(l);

				for (auto i = 0; i < l; ++i) {
					squareList.push_back(i * i % nodesNumber);
				}

				for (auto i = 0; i < nodesNumber; ++i) {
					for (auto j: squareList) {
						edgesPairs.emplace_back(i, (i + j) % nodesNumber);
					}
				}

				buildEdges(edgesPairs);

				valid = true;
			}

			Margulis::Margulis(size_t nodesNumber) : Graph(nodesNumber) {
				int s = sqrt(nodesNumber);
				if ((s * s) != nodesNumber) {
					valid = false;
				} else {
					std::vector<std::pair<size_t, size_t>> edgesPairs;

					for (size_t i = 0; i < s; ++i) {
						for (size_t j = 0; j < s; ++j) {
							edgesPairs.emplace_back(i * s + j, ((i + 2 * j) % s) * s + j);
							edgesPairs.emplace_back(i * s + j, i * s + (2 * i + j) % s);
							edgesPairs.emplace_back(i * s + j, i * s + (2 * i + j + 1) % s);
							edgesPairs.emplace_back(i * s + j, ((i + 2 * j + 1) % s) * s + j);
						}
					}

					buildEdges(edgesPairs);

					valid = true;
				}
			}
		}
	}
}
