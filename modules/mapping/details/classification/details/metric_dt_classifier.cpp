
#include "metric_dt_classifier.hpp"
#include "dimension.hpp"
#include "3rdparty/metric_clustering/clustering.hpp"
#include <cassert>

namespace clustering
{



template <typename T>
std::tuple<std::vector<int>,std::vector<int>,std::vector<int>>
kmedoids_(
          const std::vector<std::vector<T>> & D, //data,
          // T Dsum,
          int k,
          // std::string distance_measure = distance_functions::default_measure()
          int iters // added by Max Filippov
          )
{

    // can be optimized: TODO operate on indices and global dataset instead of copied subset table

    // check arguments
    // int n = data.size();
    int n = D.size();

    assert(n >= 2); // error("There must be at least two points.")
    assert(k <= n); // Attempt to run PAM with more clusters than data.


    // build the (pairwaise) distance matrix
    // auto [D,Dsum] = kmedoids_functions::distance_matrix(data,distance_measure);

    T Dsum = 0; // sum all distances // added by Max Filippov // TODO optimize
    for (auto el1 : D)
        for (auto el2 : el1)
            Dsum += el2;


    std::vector<int> seeds(k);
    std::vector<int> counts(k,0);
    std::vector<int> assignments(n,0);
    std::vector<int> sec_nearest(n,0);      /// Index of second closest medoids.  Used by PAM.
    T total_distance;                       /// Total distance tp their medoid
    T epsilon =  1e-15;                     /// Normalized sensitivity for convergence


    // set initianl medoids
    kmedoids_functions::init_medoids(k, D,seeds,assignments,sec_nearest,counts);

    T tolerance = epsilon * Dsum / (D[0].size() * D.size());

    // std::cout << "\nkmedoids_ called \n";

    while (true) {
        iters--; // added by Max Filippov
        // initial cluster
        for (std::size_t i=0; i<counts.size();++i){
            counts[i]=0;
        }
        total_distance = kmedoids_functions::update_cluster(D,seeds,assignments,sec_nearest,counts);

        //vars to keep track of minimum
        T minTotalCost = std::numeric_limits<T>::max();
        int minMedoid = 0;
        int minObject = 0;

        //iterate over each medoid
        for (int i=0; i < k; i++) {
            //iterate over all non-medoids
            for (std::size_t h = 0; h < assignments.size(); h++) {
                if (static_cast<std::size_t>(seeds[assignments[h]]) == h) continue;

                //see if the total cost of swapping i & h was less than min
                T curCost = kmedoids_functions::cost(i, h, D,seeds,assignments,sec_nearest);
                if (curCost < minTotalCost) {
                    minTotalCost = curCost;
                    minMedoid = i;
                    minObject = h;
                }
            }
        }

        // convergence check
        if (minTotalCost >= -tolerance) break;
        if (iters < 0)
        {
            std::cout << "\nWarning: exiting kmedoids_ due to exceeding max number of iterations\n";
            break;
        }

        // install the new medoid if we found a beneficial swap
        seeds[minMedoid] = minObject;
        assignments[minObject] = minMedoid;
    }


    return {assignments, seeds, counts};
}

} // namespace clustering








namespace metric
{

namespace classification
{




template <class Record>
MetricDT<Record>::MetricDT(double entropy_threshold_, double gain_threshold_)
{
    entropy_threshold = entropy_threshold_;
    gain_threshold = gain_threshold_;
}





template <class Record>
//template <typename ConType, class DimSet> // code for DimensionSet
template <typename ConType, typename VariantType>
void MetricDT<Record>::train(
        ConType & payments,
        // std::vector<std::function<NumType(Record)>> & features,  // (old) code for accessors
        // DimSet & dimensions, // code for DimensionSet
        std::vector<VariantType> dimensions,
        std::function<int(const Record&)> & response)
{
    //typedef typename DimSet::DistanceType NumType; // code for DimensionSet
    typedef double NumType; // TODO replace hardcode

    if (dimensions.size() == 0)
        return;

    std::vector<std::vector<std::vector<NumType>>> distances = {}; // first by feature, then record against record matrix
    for (size_t f=0; f<dimensions.size(); f++) // field (dimension) loop
    {
        std::vector<std::vector<NumType>> matrix(payments.size(), std::vector<NumType>(payments.size()));
        for (size_t i=0; i<payments.size(); i++)
        {
            //auto d_vis = [&, payments, i](auto & d) { return d(payments[i]); };
            //auto field1 = std::visit(d_vis, dimensions[f]); // call Dimension functor for current Record
            for (size_t j=i; j<payments.size(); j++)
            {
                //matrix[i][j] = matrix[j][i] = dimensions.get_distance(payments[i], payments[j], f); // code for DimensionSet
                //matrix[i][j] = matrix[j][i] =  dimensions[f](payments[i], payments[j]);
                //auto d_vis = [&, payments, j](auto & d) { return d(payments[j]); };
                //auto field2 = std::visit(d_vis, dimensions[f]); // call Dimension functor for current Record
                //auto d_dist_vis = [&field1, &field2](auto & d)
                //{
                //    return d.DistanceFunctor(field1, field2);
                //};
                //matrix[i][j] = matrix[j][i] = (NumType)std::visit(d_dist_vis, dimensions[f]);
                auto d_dist_vis = [&payments, i, j](auto & d)
                {
                    return (NumType)d.get_distance(payments[i], payments[j]);
                };
                matrix[i][j] = matrix[j][i] = std::visit(d_dist_vis, dimensions[f]);

            }
            bool found = false; // also count unique labels
            int r = response(payments[i]);
            if (r < 0)
            {
                std::cout << "\nError in input dataset: negative labels are not allowed. Learning cancelled\n";
                return;
            }
            for (int l : unique_labels)
            {
                if (l == r)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
                unique_labels.push_back(r);
        }
        distances.push_back(matrix);
    }
    // (new) code for dimensions - end


    // here distances table is fullfilled for each feature

    // now we can start growing the tree from the root
    std::queue<NodeDataUnit> subset_queue; // this is instead of recursion
    std::vector<int> root_subset; // first node subset
    for (size_t i=0; i<payments.size(); i++)
        root_subset.push_back((int)i); // indices of subset: initially all records in dataset are selected


    std::shared_ptr<Node> current_node = std::make_shared<Node>();
    root = current_node;
    NodeDataUnit root_unit;
    root_unit.node = current_node;
    root_unit.subset = std::make_shared<std::vector<int>>(root_subset); // TODO make ctor from data size, if needed
    root_unit.entropy = std::numeric_limits<double>::max(); // root element will be processed anyway
    root_unit.debug_id = 0; // for debug purpose, TODO remove
    subset_queue.push(root_unit);

    int unit_debug_id = 0;

    while (!subset_queue.empty())
    {
        NodeDataUnit unit = subset_queue.front();
        subset_queue.pop();

        // old style code
        //std::tuple<std::vector<std::shared_ptr<std::vector<int>>>, std::vector<double>, std::vector<int>, int, double>
        //        tuple_subsets_entropy = split_subset(*unit.subset, distances, payments, response);
        //auto new_subsets = std::get<0>(tuple_subsets_entropy);
        //auto new_entropies = std::get<1>(tuple_subsets_entropy);
        //auto new_medoids = std::get<2>(tuple_subsets_entropy);
        //auto field = std::get<3>(tuple_subsets_entropy);
        //double entropy_weighted_sum = std::get<4>(tuple_subsets_entropy);
        // new style code
        auto [new_subsets, new_entropies, new_medoids, field, entropy_weighted_sum] = split_subset(*unit.subset, distances, payments, response);

        //        std::cout << "\nStart processing unit " << unit.debug_id << ", queue length is " << subset_queue.size();
        //        std::cout << "\nObtained subsets: " << new_subsets.size() << ", " << new_entropies.size() << ", " << new_medoids.size() << "\n";

        // gain-based condition check
        double gain = unit.entropy - entropy_weighted_sum;
        //        std::cout << "entropy: " << unit.entropy << ", w sum: " << entropy_weighted_sum << ", gain: " << gain << "\n";
        if (gain <= gain_threshold)
        {
            // add leaf without processing subsets
            add_distribution_to_node(payments, response, unit.subset, unit.node); // changes *unit.node
            std::cout << "\ncurrent node turned to leaf due to gain criterion\n";
            continue;
        }

        for (size_t i=0; i<new_subsets.size(); i++) // loop through subsets obtained from clustering function via process_node
        { // we assume sizes of all vectors in tuple are eqeual (to the number of classes in train dataset)
            // unit.node->medoid_indices.push_back(new_medoids[i]); // add related medoid index to the current node
            unit.node->medoid_records.push_back(payments[ new_medoids[i] ]); // add medoid raw record
            unit.node->field_index = field; // field the division is made by
            if (new_entropies[i] <= entropy_threshold || new_subsets[i]->size() < unique_labels.size())
            {
                // add leaf node
                auto new_node = std::make_shared<Node>();
                if (!(new_entropies[i] > 0))
                {
                    if ((*(new_subsets[i])).size() > 0)
                    {
                        // add non-stohastic leaf node
                        //                        std::cout << "\nAdding leaf node. Now " << unit.node->children.size() << " childten in current node\n";
                        new_node->predicted_class = response(payments[ (*(new_subsets[i]))[0] ]); // call label accessor for the first element
                        // assume subset is not empty and all labels are equal (because of zero entropy)
                    }
                    else
                    {
                        // empty subset ocurred, add stohastic equally-distributed node
                        //                        std::cout << "\nAdding stohastic equally-distributed leaf node due to EMPTY SUBSET. Now " << unit.node->children.size() << " childten in current node\n";
                        for (size_t l=0; l<unique_labels.size(); l++)
                            new_node->prediction_distribution.push_back(1);
                    }

                }
                else
                {
                    //                    std::cout << "\nAdding stohastic leaf node. Now " << unit.node->children.size() << " childten in current node\n";
                    add_distribution_to_node(payments, response, new_subsets[i], new_node); // changes *new_node
                }
                unit.node->children.push_back(new_node); // set pointer to the newly created child in the current node
            }
            else
            {
                // enqueue regular node (add subset to queue and node to tree)
                //                std::cout << "\nAdding regular node. Now " << unit.node->children.size() << " childten in current node\n";
                NodeDataUnit new_unit;
                new_unit.node = std::make_shared<Node>(); // create new empty node within queue unit
                new_unit.subset = new_subsets[i]; // pointer to vector of subset indices
                new_unit.entropy = new_entropies[i]; // in order to compute gain on the next step
                unit.node->children.push_back(new_unit.node); // set pointer to the newly created child in the current node
                new_unit.debug_id = ++unit_debug_id; // for debuf porpose, TODO disable
                subset_queue.push(new_unit); // enqueue new data unit;
                //                std::cout << "\nenqueued subset " << unit_debug_id << "\n";
            }
        }
        // std::cout << "\n - last in queue: " << subset_queue.back().debug_id;
        // std::cout << "\n - first in queue: " << subset_queue.front().debug_id << "\n";
    }
}



template <class Record>
//template <typename ConType, typename DimSet>  // code for DimensionSet
template <typename ConType, typename VariantType>
void MetricDT<Record>::predict(
        ConType & input_data,
        // std::vector<std::function<NumType(Record)> > & features, // (old) code for accessors
        //DimSet & dimensions, // code for DimensionSet
        std::vector<VariantType> dimensions,
        std::vector<int> & predictions)
{
    //typedef typename DimSet::DistanceType NumType; // code for DimensionSet
    typedef double NumType; // TODO replace hardcode

    if (root == nullptr || input_data.size() == 0)
        return;

    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());

    // (old) code for accessors
    //        std::function<int(Record)> f = [](Record r){return -1;}; // TEMPORARY dummy, required by read_data signature

    //        // old style code
    //        std::tuple< std::vector<std::vector<std::vector<NumType>>>, std::vector<int> > data_tuple = read_data(input_data, features, f); // TEMPORARY function call, TODO replace with correct input processing
    //        std::vector<std::vector<std::vector<NumType>>> data = std::get<0>(data_tuple); // user input in (temporary?) numerical representation
    //        // we do not need unique labels here // std::get<1>(data_tuple);
    //        // new style code
    //        //auto [data, ul] = read_data(input_data, features, f);

    //        std::string distance_measure = "euclidian"; // TODO move to the API!!


    //for (size_t i=0; i<data[0].size(); i++) // input data item loop  (old) code for accessors
    for (size_t i=0; i<input_data.size(); i++) // input data item loop  (new) code for dimensions
    {
        //        std::cout << "\nstart prediction for record " << i << "\n";
        std::shared_ptr<Node> current_node = root;
        while (current_node->children.size() > 0) //predicted_class == -1) // go through tree until leaf is reached
        {
            if (current_node->predicted_class != -1)
                std::cout << "\nError: children found, but predicted class is defined!\n"; // this should never happen
            double min_distance = std::numeric_limits<double>::max();
            int nearese_medoid_index = -1;
            for (size_t m=0; m<current_node->medoid_records.size(); m++) // medoid loop
            {
                Record r = current_node->medoid_records[m];
                //std::vector<NumType> medoid_record_numeric = {features[ current_node->field_index ](r)};  // read field into vector // here one single value id read, TODO generalize!!  // (old) code for accessors
                //NumType distance = clustering::distance_functions::distance(data[current_node->field_index][i], medoid_record_numeric, distance_measure); // (old) code for accessors
                //NumType distance = dimensions.get_distance(input_data[i], r, current_node->field_index);// dimensions[current_node->field_index](input_data[i], r); // code for DimensionSet
                //auto d_vis = [&, input_data, i](auto & d) { return d(input_data[i]); };
                //auto field1 = std::visit(d_vis, dimensions[current_node->field_index]); // call Dimension functor for current Record
                //auto d_vis2 = [&, r](auto & d) { return d(r); };
                //auto field2 = std::visit(d_vis2, dimensions[current_node->field_index]); // call Dimension functor for current Record
                //auto d_dist_vis = [&field1, &field2](auto & d)
                //{
                //    return d.DistanceFunctor(field1, field2);
                //};
                //NumType distance = (NumType)std::visit(d_dist_vis, dimensions[current_node->field_index]);

                auto d_dist_vis = [&input_data, i, r](auto & d)
                {
                    return (NumType)d.get_distance(input_data[i], r);
                };
                NumType distance = std::visit(d_dist_vis, dimensions[current_node->field_index]);



                if (distance < min_distance)
                {
                    min_distance = distance;
                    nearese_medoid_index = m;
                }
            }
            // here we have nearest medoid index, so we can chose the child, assuming order of childs is same as order of medoids

            current_node = current_node->children[nearese_medoid_index];
            //            std::cout << "processind node with predicted_class " << current_node->predicted_class << "\n";
        }
        if (current_node->predicted_class != -1)
            predictions.push_back(current_node->predicted_class);
        else
        {
            // montecarlo prediction generation based on leaf subset label distribution
            int r_max = 0;
            std::vector<int> roulette = {};
            for (int el : current_node->prediction_distribution)
            {
                r_max += el;
                roulette.push_back(r_max);
            }
            std::uniform_int_distribution<int>  distr(0, r_max - 1);
            int random_val = (int)distr(generator);
            int r_idx = 0;
            while (random_val >= roulette[r_idx])
                r_idx++;
            // here r_idx is randomely chosen index in current_node->prediction_distribution

            predictions.push_back(unique_labels[r_idx]);

        }

    }
}


template <class Record>
template <typename NumType>
std::tuple< std::vector<std::vector<NumType>>, std::vector<int> > MetricDT<Record>::distance_matrix_of_subset(
        const std::vector<int> & subset,
        const std::vector<std::vector<NumType>> & feature_dist
        )
{
    // output - distance subtable ready for kmedoids_
    std::vector<std::vector<NumType>> result(subset.size(), std::vector<NumType>(subset.size()));
    std::vector<int> orig_idx(subset.size());
    for (size_t i=0; i<subset.size(); i++)
    {
        for (size_t j=i; j<subset.size(); j++)
        {
            result[i][j] = result[j][i] = feature_dist[subset[i]][subset[j]];
        }
        orig_idx[i] = subset[i];
    }
    return std::tuple(result, orig_idx);
}

template <class Record>
template <typename ConType, typename NumType>
std::tuple<std::vector<std::shared_ptr<std::vector<int>>>, std::vector<double>, std::vector<int>, int, double>
MetricDT<Record>::split_subset(
        const std::vector<int> & subset,
        const std::vector<std::vector<std::vector<NumType>>> & distances,
        const ConType & data,
        const std::function<int(Record)> & response
        )
{
    // input - indices of subset, full distance matrices for all features, raw data, label acceessor
    // output - for each new subset: pointer to vector of element indices, entropy value, medoid index, and single values: index of the field used for classification and entropy weighted sum

    std::vector<std::vector<std::vector<int>>> new_subsets(
                distances.size(),
                std::vector<std::vector<int>>(unique_labels.size(), std::vector<int>(0))
                );
    // by field, then by nearest medoid, then global indices of new subset elements

    std::vector<std::vector<double>> split_entropy(
                distances.size(),
                std::vector<double>(unique_labels.size(), 0));
    // by field, then by nearest medoid (one value per new subset for each field)
    // we keep entropy values of all fields in order to return it for the winning one

    // std::vector<double> entropy_weighted_sum(distances.size(), 0);
    // one value per field

    std::vector<std::vector<int>> medoid_global_idx(
                distances.size(),
                std::vector<int>(unique_labels.size(), -1)
                );
    // by field, then by nearest medoid
    // global indices of medoids for each field

    double min_entropy_weighted_sum = std::numeric_limits<double>::max(); // criterion for the best field selection
    int greatest_information_gain_idx = -1;

    for (size_t f=0; f<distances.size(); f++) // field loop
    {
        //  clustering via kmedoids_

        // old style code
        //auto feature_dist_tuple = distance_matrix_of_subset(subset, distances[f]); // preparing data table for kmedoids_
        //auto feature_dist_table = std::get<0>(feature_dist_tuple); // this table is created only in order to reuse kmedoids code
        //auto feature_dist_orig_idx = std::get<1>(feature_dist_tuple); // this is needed to decode indices returned by kmedoid
        // new code style
        auto [feature_dist_table, feature_dist_orig_idx] = distance_matrix_of_subset(subset, distances[f]);

        auto results = clustering::kmedoids_(feature_dist_table, unique_labels.size());
        auto medoids = std::get<1>(results); // medoid indices in feature_dist_table; original indices can be obtained via feature_dist_orig_idx

        // for (auto medoid : medoids)
        // std::cout << "\nseeds for feature " << f << ": " << medoid << " | " << feature_dist_orig_idx[medoid] << "\n";

        std::vector<std::vector<int>> label_count(unique_labels.size(), std::vector<int>(medoids.size(), 0));
        // one value per each class for each new subset

        for (auto el : subset) // record loop
        {
            // find and compare distances from the current record to medoids
            int chosen_medoid = -1;
            NumType min_dist;
            for (size_t medoid_idx = 0; medoid_idx<medoids.size(); medoid_idx++) // medoid loop
            { // compare distance to medoids. We assume medoids.size() MUST NOT be greater than unique_labels.size()
                NumType d = distances[f][el][ feature_dist_orig_idx[medoids[medoid_idx]] ]; // between each el and each medoid
                // std::cout << "\nelement " << el << " to medoid " << feature_dist_orig_idx[medoids[medoid_idx]] << " distance: " << d;
                if (chosen_medoid == -1) // first time
                {
                    min_dist = d;
                    chosen_medoid = medoid_idx;
                }
                else
                {
                    if (d < min_dist)
                    {
                        min_dist = d;
                        chosen_medoid = medoid_idx;
                    }
                }
            } // end of medoid loop
            // here we have index of the nearest mediod in chosen medoid; this index is related to 'medoids' vector, where indices in feature_dist_table are saved

            // add el to new subset related to the nearest medoid and count labels for each new subset
            // std::cout << "\nnearest medoid is " << chosen_medoid << "\n";
            new_subsets[f][chosen_medoid].push_back(el); // add the current element to appropriate new subset
            for (size_t l=0; l<unique_labels.size(); l++)
            {
                if (unique_labels[l] == response(data[el]))
                {
                    label_count[l][chosen_medoid]++; // query response and count elements of each class (by labels) in each new subset
                    break; // values in label_count are unique
                }
            }
            // old code //label_count[response(data[el])][chosen_medoid]++; // query response and count elements of each class (by labels) in each new subset
        } // end of record loop
        // here we have subsets for the current field based on distance to medoids listed in 'medoids' vector
        // and counts of elements of each class

        double current_entropy_weighted_sum = 0;
        for (size_t medoid_idx = 0; medoid_idx<medoids.size(); medoid_idx++) // medoid loop: collecting outpot data for each new subset
        { // now we do not know which field will win, so we collect data for each field
            // find entropy of each new subset and weighted sum of them (gain subtrahend)
            double new_entropy = 0;
            // old code //for (auto l : unique_labels)
            for (size_t l=0; l<unique_labels.size(); l++)
            {
                double probability = (double)label_count[l][medoid_idx] / new_subsets[f][medoid_idx].size(); // share of each class by labels
                if (probability > 0)
                    new_entropy = new_entropy - probability * std::log2(probability);
                // else do nothing, because lim(share*log2(share))==0
                // TODO add zero entropy "leaf reached" flag if needed
                if (new_entropy < 0)
                    std::cout << "\nSomething went wrong: negative entropy\n";
                if (isnan(new_entropy)) // TODO remove
                    std::cout << "\nnan entropy\n";
            }
            split_entropy[f][medoid_idx] = new_entropy;
            current_entropy_weighted_sum = current_entropy_weighted_sum + new_entropy * new_subsets[f][medoid_idx].size() / subset.size();

            // also fill global medoid indices (in order to output them for winning field)
            medoid_global_idx[f][medoid_idx] = feature_dist_orig_idx[medoids[medoid_idx]];
        } // end of medoid loop
        // entropy_weighted_sum[f] = current_entropy_weighted_sum;
        if (current_entropy_weighted_sum < min_entropy_weighted_sum) // update winning field
        {
            min_entropy_weighted_sum = current_entropy_weighted_sum;
            greatest_information_gain_idx = f;
        }
        // here we also have entropy of each subset and gain subtrahend for all subsets (filled for the current field)

    } // end of field loop


    // here we have new subsets, entropy of each subset and gain subtrahends - for all fields
    // and index of the winning field

    // make output ptrs
    std::vector<std::shared_ptr<std::vector<int>>> output_subset_ptrs = {};
    for (size_t medoid_idx = 0; medoid_idx<unique_labels.size(); medoid_idx++) // medoid loop: making ptrs: doing it only for winning field mast be faster than creating ptrs for all field inside record loop
        output_subset_ptrs.push_back(std::make_shared<std::vector<int>>(new_subsets[greatest_information_gain_idx][medoid_idx]));

    return std::tuple(
                output_subset_ptrs, // new_subsets[best_field_idx],
                split_entropy[greatest_information_gain_idx],
                medoid_global_idx[greatest_information_gain_idx],
                greatest_information_gain_idx,
                min_entropy_weighted_sum
                );
}




// // this function is for test purpous only
//template <class Record>
//template <typename ConType, typename NumType>
//std::tuple< std::vector<std::vector<std::vector<NumType>>>, std::vector<int> > MetricDT<Record>::read_data( // TODO return without copying!!
//                                                                                                            ConType & payments,
//                                                                                                            std::vector<std::function<NumType(Record)>> & features,
//                                                                                                            std::function<int(Record)> & response)
//{
//    std::vector<std::vector<std::vector<NumType>>> data(features.size()); // first by features, then by records, then by elements of feature object
//    std::vector<int> labels = {};

//    for (size_t i=0; i<payments.size(); i++ ) // read data using accessors, temporary
//    {
//        for (size_t j=0; j<features.size(); j++ )
//        {
//            std::vector v = {features[j](payments[i])};  // read data into vector (in this case 1 datum per feature)
//            data[j].push_back(v); // save data by columns
//        }

//        bool found = false; // also count unique labels
//        int r = response(payments[i]);
//        for (int l : labels)
//        {
//            if (l == r)
//            {
//                found = true;
//                break;
//            }
//        }
//        if (!found)
//            labels.push_back(r);
//    }
//    // here we have raw data table and list of unique labels


//    return std::tuple(data, labels);
//}



template <class Record>
template <typename ConType>
inline void MetricDT<Record>::add_distribution_to_node( // mutates *new_node!
                                                        const ConType & payments,
                                                        const std::function<int(Record)> & response,
                                                        const std::shared_ptr<std::vector<int>> & new_subset,
                                                        const std::shared_ptr<Node> & new_node // subject to change
                                                        )
{
    if (new_subset->size() < 1)
        std::cout << "\nSomething went wrong: empty dataset obtained!\n"; // should never happen
    // describe distribution in order to generate random values in predict function
    for (size_t l=0; l<unique_labels.size(); l++)
        new_node->prediction_distribution.push_back(0); // init distribution
    for (size_t r=0; r < new_subset->size(); r++) // elements in subset
    { // fullfill distribution vector
        for (size_t l=0; l<unique_labels.size(); l++)
            if (response(payments[ (*new_subset)[r] ]) == unique_labels[l])
                new_node->prediction_distribution[l]++; // simply count occurences
    }
}





} // namespace classification
} // namespace metric

