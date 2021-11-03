
#include "modules/transform/energy_encoder.hpp"

#include <vector>
#include <iostream>
#include <fstream>



// ----- csv reader


template <typename T> T convert_to(const std::string & str)
{
    std::istringstream s(str);
    T num;
    s >> num;
    return num;
} // templated version of stof, stod, etc., thanks to https://gist.github.com/mark-d-holmberg/862733


template <class ValueType>
std::vector<std::vector<ValueType>> read_csv_num(std::string filename, std::string sep=",")
{
    typedef typename std::vector<ValueType> LINE;
    std::string line;
    int pos;
    std::vector<std::vector<ValueType>> array = {};
    std::ifstream in(filename);
    if(!in.is_open()) {
        std::cout << "Failed to open file" << std::endl;
        return array;
    }
    while( getline(in,line) ) {
        LINE ln;
        while( (pos = line.find(sep)) >= 0) {
            std::string field = line.substr(0, pos);
            line = line.substr(pos+1);
            ln.push_back(convert_to<ValueType>(field));
        }
        ln.push_back(convert_to<ValueType>(line));
        array.push_back(ln);
    }
    return array;
}


template <class ContainerType>
void vv_to_csv(ContainerType data, std::string filename, std::string sep=",")  // container
{
    std::ofstream outputFile;
    outputFile.open(filename);
        for (auto i = 0; i < data.size(); ++i) {
            for (auto j = 0; j < data[i].size(); j++) {
                outputFile << std::to_string(data[i][j]);
                if (j < data[i].size() - 1)
                    outputFile << sep;
            }
            outputFile << std::endl;
        }
        outputFile.close();
}


// -----





using T = double;




int main() {


    std::cout << "computing energies..." << std::endl;

    size_t split_wnd_sz = 48000/10; //384; // max window size of anomaly detector //128;


    //auto ds_raw = read_csv_num<T>("anomaly_detector_data_1/script/Versuch_1.6.5.a.csv");
    //auto ds_raw = read_csv_num<T>("anomaly_detector_data_1/script/sound50000.csv");
    //auto ds_raw = read_csv_num<T>("anomaly_detector_data_1/script/sound40000.csv");

//    auto ds_raw = read_csv_num<T>("anomaly_detector_data_1/script/sound1.csv");
//    auto ds_raw = read_csv_num<T>("anomaly_detector_data_1/script/sound2.csv");
//    auto ds_raw = read_csv_num<T>("anomaly_detector_data_1/script/sound3.csv");
//    auto ds_raw = read_csv_num<T>("anomaly_detector_data_1/script//sound4.csv");
    auto ds_raw = read_csv_num<T>("anomaly_detector_data_1/script/realsnd/sound_crop_TS1_Norm_10rpm_60min.wav.csv");
//    auto ds_raw = read_csv_num<T>("anomaly_detector_data_1/script/realsnd/sound_crop_TS1_Norm_30rpm_60min.wav.csv");
//    auto ds_raw = read_csv_num<T>("anomaly_detector_data_1/script/realsnd/sound_crop_TS1_Norm_60rpm_60min.wav.csv");
//    auto ds_raw = read_csv_num<T>("anomaly_detector_data_1/script/realsnd/sound_crop_TS1_Ruhe.wav.csv");

    std::vector<T> ts;
    ts.reserve(ds_raw.size());
    for (auto el : ds_raw) {
        ts.push_back(el[2]);
    }

    std::vector<std::vector<T>> out;
    out.reserve(ts.size());

    //auto e = metric::EnergyEncoder(4, 8);
    auto e = metric::EnergyEncoder(4, 4, false);

    //for (size_t wnd_pos = split_wnd_sz; wnd_pos < ts.size(); ++wnd_pos) { // slide over dataset
    for (size_t wnd_pos = split_wnd_sz; wnd_pos < ts.size(); wnd_pos += split_wnd_sz) { // slide over dataset

        std::vector<T> wnd = std::vector<T>(ts.begin() + wnd_pos - split_wnd_sz, ts.begin() + wnd_pos);

        T sum = 0;
        for (size_t idx = 0; idx < wnd.size(); ++idx) {
            sum += wnd[idx];
        }
        T avg = sum / (T)wnd.size();
        for (size_t idx = 0; idx < wnd.size(); ++idx) {
            wnd[idx] -= avg;
        } // shifted to average

        std::vector<T> subbands = e(wnd);

        out.push_back(subbands);
    }

//    vv_to_csv(out, "anomaly_detector_data_1/script/real_energies_100ms_sp8_1.csv");
//    vv_to_csv(out, "anomaly_detector_data_1/script/real_energies_100ms_sp8_2.csv");
//    vv_to_csv(out, "anomaly_detector_data_1/script/real_energies_100ms_sp8_3.csv");
//    vv_to_csv(out, "anomaly_detector_data_1/script/real_energies_100ms_sp8_4.csv");
    vv_to_csv(out, "anomaly_detector_data_1/script/realsnd/energies_TS1_Norm_10rpm_60min.wav.csv");
//    vv_to_csv(out, "anomaly_detector_data_1/script/realsnd/energies_TS1_Norm_30rpm_60min.wav.csv");
//    vv_to_csv(out, "anomaly_detector_data_1/script/realsnd/energies_TS1_Norm_60rpm_60min.wav.csv");
//    vv_to_csv(out, "anomaly_detector_data_1/script/realsnd/energies_TS1_Ruhe.wav.csv");


    return 0;
}

