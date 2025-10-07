#ifndef KNN_H
#define KNN_H

#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <optional>
#include <iostream>

#define KNN_K_VAL 5

class Knn {
public:
	struct Point {
		std::string label;
		std::vector<float> values;
	};

	struct Neighbor {
		std::string label;
		float distance;
	};

	struct NeighborOccurrency {
		std::string label;
		int occurrencies;
	};

	Knn();
	std::string predict(std::vector<float> new_point);
	void upload_dataset(std::vector<Point>& new_dataset);
	std::vector<Point> dataset;

private:
	std::vector<float> mins;
	std::vector<float> maxs;
	std::vector<float> weights;

	void normalize_new_point(std::vector<float>& new_values);
	void normalize_dataset();
	float distance(std::vector<float> values_1, std::vector<float> values_2);
	static std::optional<NeighborOccurrency*> exists_neighbor_occurency (const std::string& label, std::vector<NeighborOccurrency>& neighbor_occurrencies);
};

#endif //KNN_H
