#include "knn.h"

#include <algorithm>

Knn::Knn() {
	// green,red,orange,blue,white
	weights = {0.7,1,1,1.3,3.5};
};

void Knn::upload_dataset(std::vector<Point> &new_dataset) {
	dataset = new_dataset;
	normalize_dataset();
}


float Knn::distance(std::vector<float> point_1, std::vector<float> point_2) {
	float sum = 0;
	if (point_1.size() != point_2.size()) return -1;

	for (int i = 0; i < point_1.size(); i++) {
		sum += std::pow(point_1.at(i) - point_2.at(i), 2);
	}

	return std::sqrt(sum);
}

std::string Knn::predict(std::vector<float> new_point) {
	normalize_new_point(new_point);

	std::vector<Neighbor> neighbors;
	std::vector<Neighbor> nearest_neighbors;
	std::vector<NeighborOccurrency> neighbor_occurrencies;
	neighbors.reserve(dataset.size());
	nearest_neighbors.resize(KNN_K_VAL);

	for (const auto& point : dataset) {
		neighbors.push_back({
			point.label,
			distance(point.values, new_point)
		});
	}

	std::sort(
		neighbors.begin(), neighbors.end(),
		[](const Neighbor& a, const Neighbor& b) {
			return a.distance < b.distance;
		});

	nearest_neighbors.assign(neighbors.begin(), neighbors.begin() + KNN_K_VAL);

	for (const auto& nearest_neighbor : nearest_neighbors) {
		std::optional<NeighborOccurrency*> neighbor_occurrency = exists_neighbor_occurency(nearest_neighbor.label, neighbor_occurrencies);
		if (neighbor_occurrency.has_value()) {
			neighbor_occurrency.value() -> occurrencies += 1;
		} else {
			neighbor_occurrencies.push_back({
				nearest_neighbor.label,
				1,
			});
		}
	}

	std::sort(
		neighbor_occurrencies.begin(), neighbor_occurrencies.end(),
		[](const NeighborOccurrency& a, const NeighborOccurrency& b) {
			return a.occurrencies > b.occurrencies;
		}
	);
	if (neighbor_occurrencies.empty()) return "";
	if (neighbor_occurrencies.size() > 1 && neighbor_occurrencies.at(0).occurrencies == neighbor_occurrencies.at(1).occurrencies) {
		return "";
	}

	return neighbor_occurrencies.at(0).label;
}

std::optional<Knn::NeighborOccurrency*> Knn::exists_neighbor_occurency(const std::string& label, std::vector<NeighborOccurrency>& neighbor_occurrencies) {
	for (auto& neighbor_occurrency : neighbor_occurrencies) {
		if (neighbor_occurrency.label == label) return &neighbor_occurrency;
	}

	return std::nullopt;
}



void Knn::normalize_dataset() {
	if (dataset.empty()) return;

	mins.resize(dataset.at(0).values.size());
	maxs.resize(dataset.at(0).values.size());
	std::fill(mins.begin(), mins.end(), std::numeric_limits<int>::infinity());
	std::fill(maxs.begin(), maxs.end(), 0);

	for (const auto& point : dataset) {
		for (int i = 0; i < point.values.size(); i++) {
			if (mins.at(i) > point.values.at(i)) mins.at(i) = point.values.at(i);
			if (maxs.at(i) < point.values.at(i)) maxs.at(i) = point.values.at(i);
		}
	}

	for (auto& point : dataset) {
		for (int i = 0; i < point.values.size(); i++) {
			const float hundred_percent = maxs.at(i) - mins.at(i);
			const float new_value = point.values.at(i) - mins.at(i);

			const float normalized_value = new_value * 100 / hundred_percent;
			point.values.at(i) = normalized_value*weights.at(i);
		}
	}
}

void Knn::normalize_new_point(std::vector<float>& new_values) {
	if (new_values.size() != mins.size()) return;

	for (int i = 0; i < new_values.size(); i++) {
		const float hundred_percent = maxs.at(i) - mins.at(i);
		const float new_value = new_values.at(i) - mins.at(i);

		const float percentage = new_value * 100 / hundred_percent;
		new_values.at(i) = percentage*weights.at(i);
	}
}

