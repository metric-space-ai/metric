

class HierarchicalClustering {
      private:

      protected: 
          
      public:
		  template <typename T>
		  HierarchicalClustering(
			  const std::vector<std::vector<T>> &data,
			  const int &k)
		  {
          }

		  void initialize()
		  {
		  }

		  template <typename T>
		  std::vector<std::vector<T>> hierarchical_clustering()
		  {
		  }

};

template <typename T>
std::vector<std::vector<T>> HierarchicalClustering::hierarchical_clustering()
{
	dataset = self.dataset;
	current_clusters = self.clusters;
	old_clusters = [];
	heap = hc.compute_pairwise_distance(dataset);
	heap = hc.build_priority_queue(heap);

	while (len(current_clusters) > self.k)
	{
		dist, min_item = heapq.heappop(heap);
		// pair_dist = min_item[0]
		pair_data = min_item[1];

		// judge if include old cluster
		if( not self.valid_heap_node(min_item, old_clusters))
		{
			continue;
		}

		new_cluster = {};
		new_cluster_elements = sum(pair_data, []);
		new_cluster_cendroid = self.compute_centroid(dataset, new_cluster_elements);
		new_cluster_elements.sort();
		new_cluster.setdefault("centroid", new_cluster_cendroid);
		new_cluster.setdefault("elements", new_cluster_elements);
		for (pair_item in pair_data )
		{
			old_clusters.append(pair_item);
			del current_clusters[str(pair_item)];
		}
		self.add_heap_entry(heap, new_cluster, current_clusters);
		current_clusters[str(new_cluster_elements)] = new_cluster;
	}

	current_clusters.sort();
	return current_clusters;
}