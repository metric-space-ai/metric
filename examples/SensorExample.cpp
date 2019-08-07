/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include <variant>
#include <chrono>
#include <deque> // for Record test
#include <vector>
#include <thread>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <codecvt>
#include <random>
#include <math.h>  
#include <string.h>
#include <algorithm>
#include <numeric>      // std::iota

#include <libpq-fe.h>

#include "ThreadPool.cpp"
#include "Semaphore.h"

#include "../modules/crossfilter.hpp"
#include "../modules/mapping/metric_mapping.hpp"
#include "../modules/distance/details/k-related/entropy.hpp"
#include "../modules/distance/details/k-related/Standards.hpp"
#include "../modules/mapping/details/hierarchClustering.hpp"
#include "../modules/correlation/metric_correlation.hpp"


//#include "modules/mapping/details/classification/metric_classification.hpp"

#include "../utils/poor_mans_quantum.hpp"

template <typename T0, typename ... Ts>
std::ostream & operator<< (std::ostream & s,
	std::variant<T0, Ts...> const & v)
{
	std::visit([&](auto && arg) { s << arg; }, v); return s;
}

template <typename T>
void vector_print(const std::vector<T> &vec, int maxRows = -1)
{

	std::cout << "[";
	for (auto i = 0; i < vec.size(); i++)
	{
		if (i < vec.size() - 1)
		{
			std::cout << vec[i] << ", ";
		}
		else
		{
			std::cout << vec[i] << "]" << std::endl;
		}

		if (maxRows >= 0 && i >= maxRows - 1)
		{
			if (vec.size() > maxRows)
			{
				std::cout << " ... " << "]" << std::endl;
			}
			break;
		}
	}
}


template <typename T>
void matrix_print(const std::vector<std::vector<T>> &mat, int maxRows = -1, int maxCols = -1)
{

	std::cout << "[" << std::endl;
	for (auto i = 0; i < mat.size(); i++)
	{
		std::cout << "  [ ";
		for (auto j = 0; j < mat[i].size(); j++)
		{
			std::cout << mat[i][j];
			if (j < mat[i].size() - 1)
			{
				std::cout << ", ";
			}
			if (maxCols >= 0 && j >= maxCols - 1)
			{
				if (mat[i].size() > maxCols)
				{
					std::cout << " ... ";
				}
				break;
			}
		}
		std::cout << " ]" << std::endl;

		if (maxRows >= 0 && i >= maxRows - 1)
		{
			if (mat.size() > maxRows)
			{
				std::cout << "  ... ";
			}
			break;
		}
	}
	std::cout << "]" << std::endl;
}

void query_print(PGresult *res, int maxRows = -1)
{
	int ncols = PQnfields(res);
	printf("There are %d columns:", ncols);
	printf("\n");
	printf("\n");
	for (int i = 0; i < ncols; i++)
	{
		char *name = PQfname(res, i);
		printf("%s | ", name);
	}
	printf("\n");
	printf("\n");

	int nrows = PQntuples(res);
	for (auto i = 0; i < nrows; i++)
	{
		for (auto j = 0; j < ncols; j++)
		{
			char* v = PQgetvalue(res, i, j);
			printf("%s | ", v);
		}
		printf("\n");

		if (maxRows >= 0 && i >= maxRows - 1)
		{
			if (nrows > maxRows)
			{
				std::cout << "  ... ";
			}
			break;
		}
	}
	printf("\n");

	printf("Total: %d rows\n", nrows);
}

std::wstring stringToWstring(const std::string& t_str)
{
	//setup converter
	typedef std::codecvt_utf8<wchar_t> convert_type;
	std::wstring_convert<convert_type, wchar_t> converter;

	//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
	return converter.from_bytes(t_str);
}

// Converts UTC time string to a time_t value.
std::time_t getEpochTime(const char* dateTime)
{
	std::string dateTime_str(dateTime);

	// Let's consider we are getting all the input in
	// this format: '2014-07-25 20:17:22'
	// A better approach would be to pass in the format as well.
	static const std::wstring dateTimeFormat{ L"%Y-%m-%d %H:%M:%S" };

	// Create a stream which we will use to parse the string,
	// which we provide to constructor of stream to fill the buffer.
	std::wistringstream ss{ stringToWstring(dateTime_str) };

	// Create a tm object to store the parsed date and time.
	std::tm dt;

	// Now we read from buffer using get_time manipulator
	// and formatting the input appropriately.
	ss >> std::get_time(&dt, dateTimeFormat.c_str());

	// Convert the tm structure to time_t value and return.
	return std::mktime(&dt);
}

/* Close connection to database */
void CloseConn(PGconn *conn)
{
	PQfinish(conn);
	printf("Connection closed\n");
	//getchar();
	//exit(1);
}

/* Establish connection to database */
PGconn *ConnectDB()
{
	std::fstream fin;
	fin.open("db_creditinals.csv", std::ios::in);

	std::vector<std::string> creds;
	std::string line, word;

	getline(fin, line);
	std::stringstream s(line);
	while (getline(s, word, ','))
	{
	}

	while (getline(fin, line)) {

		std::stringstream s(line);
		while (getline(s, word, ',')) {

			creds.push_back(word);
		}
	}

	std::string m_dbhost = creds[0];
	int         m_dbport = atoi(creds[1].c_str());
	std::string m_dbname = creds[2];
	std::string m_dbuser = creds[3];
	std::string m_dbpass = creds[4];

	////

	PGconn *conn = NULL;

	// Make a connection to the database
	std::string m_connectionString = "user=" + m_dbuser + " password=" + m_dbpass + " dbname=" + m_dbname + " hostaddr=" + m_dbhost + " port=" + std::to_string(m_dbport);
	conn = PQconnectdb(m_connectionString.c_str());

	// Check to see that the backend connection was successfully made 
	if (PQstatus(conn) != CONNECTION_OK)
	{
		printf("Connection to database failed");
		CloseConn(conn);
	}

	printf("Connection to database - OK\n");

	return conn;
}

//typedef std::vector< std::variant<bool, int, long int, double, std::string> > Record;
typedef std::vector< double > Record;

struct Feature
{
	int index;
	std::string id;
	std::string bezeichnung;
};

int lookupFeatureIndex(std::string id, const std::vector<Feature> &features)
{
	for (auto j = 0; j < features.size(); j++)
	{
		if ((id.compare(features[j].id)) == 0)
		{
			return features[j].index;
		}
	}

	for (auto j = 0; j < features.size(); j++)
	{
		if ((id.compare(features[j].bezeichnung)) == 0)
		{
			return features[j].index;
		}
	}

	return -1;
}

int countFeatures(PGconn *conn)
{
	// Execute with sql statement
	PGresult *res = NULL;

	res = PQexec(conn, "SELECT COUNT(*) FROM public.sensormetadata");
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
		CloseConn(conn);

	int numFeatures = atoi(PQgetvalue(res, 0, 0));

	// Clear result
	PQclear(res);

	return numFeatures;
}

std::vector<std::string> getTimestamps(PGconn *conn, const int limit = 100)
{
	// Execute with sql statement
	PGresult *res = NULL;

	std::string queryString = "SELECT DISTINCT date FROM public.sensordata ORDER BY date ASC LIMIT " + std::to_string(limit);

	res = PQexec(conn, queryString.c_str());
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
		CloseConn(conn);

	std::vector<std::string> stamps;

	int nrows = PQntuples(res);
	for (int i = 0; i < nrows; i++)
	{
		stamps.push_back(std::string(PQgetvalue(res, i, 0)));
	}

	// Clear result
	PQclear(res);

	return stamps;
}

std::vector<std::string> getTargetTimestamps(PGconn *conn, std::string metaid, const int limit = 0)
{
	// Execute with sql statement
	PGresult *res = NULL;

	std::string queryString = "SELECT * FROM ( SELECT DISTINCT date, metaid FROM public.sensordata WHERE metaid @> '" + 
		metaid + 
		"'::int[]) AS extended WHERE metaid::varchar(" + 
		std::to_string(metaid.size()) + ") = '" + 
		metaid + 
		"' ORDER BY date ASC";

	if (limit > 0)
	{
		queryString += " LIMIT " + std::to_string(limit);
	}
	std::cout << queryString << std::endl;

	res = PQexec(conn, queryString.c_str());
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
		CloseConn(conn);

	std::vector<std::string> stamps;

	int nrows = PQntuples(res);
	for (int i = 0; i < nrows; i++)
	{
		stamps.push_back(std::string(PQgetvalue(res, i, 0)));
	}

	// Clear result
	PQclear(res);

	return stamps;
}

std::vector<Feature> getFeatures(PGconn *conn, const int limit = 0)
{
	// Execute with sql statement
	PGresult *res = NULL;

	std::string queryString = "SELECT id, bezeichnung FROM public.sensormetadata";
	if (limit > 0)
	{
		queryString += " LIMIT " + std::to_string(limit);
	} 

	res = PQexec(conn, queryString.c_str());
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
		CloseConn(conn);

	std::vector<Feature> features;

	int nrows = PQntuples(res);
	for (int i = 0; i < nrows; i++)
	{
		features.push_back( { i, std::string(PQgetvalue(res, i, 0)), std::string(PQgetvalue(res, i, 1)) } );
	}

	// Clear result
	PQclear(res);

	return features;
}

Record getSensorRecord(PGconn *conn, const std::string date, const std::vector<Feature> &features)
{
	// Execute with sql statement
	PGresult *res = NULL;

	std::string queryString = "SELECT * FROM public.sensordata WHERE date = '" + date + "'";

	res = PQexec(conn, queryString.c_str());
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
		CloseConn(conn);

	int value_fnum = PQfnumber(res, "value");
	int feature_fnum = PQfnumber(res, "metaid");

	int hasFeatures = PQntuples(res);

	Record row(features.size(), -1);

	double value;
	std::string feature;
	int featureIndex;

	for (int i = 0; i < hasFeatures; ++i)
	{
		value = atof(PQgetvalue(res, i, value_fnum));
		feature = std::string(PQgetvalue(res, i, feature_fnum));
		featureIndex = lookupFeatureIndex(feature, features);
		if (featureIndex >= 0)
		{
			row[featureIndex] = value;
		}
	}

	return row;
}

std::vector<Record> getAllSensorRecords(PGconn *conn, const std::vector<Feature> &features, const std::vector<std::string> &dates)
{
	// Execute with sql statement
	PGresult *res = NULL;
	std::string queryString;

	/*std::string queryString = "SELECT * FROM public.sensordata WHERE date IN (";
	for (int i = 0; i < dates.size(); ++i)
	{
		if (i == dates.size() - 1) {
			queryString += "'" + dates[i] + "'";
		}
		else
		{
			queryString += "'" + dates[i] + "', ";
		}
	}
	queryString += ") ORDER BY date ASC";*/

	if (dates.size() > 0) {
		queryString = "SELECT * FROM public.sensordata WHERE date BETWEEN '" + dates[0] + "' AND '" + dates[dates.size() - 1] + "' ORDER BY date ASC";
	}
	else {
		std::string queryString = "SELECT * FROM public.sensordata ORDER BY date ASC";
	}

	std::cout << queryString << std::endl;

	res = PQexec(conn, queryString.c_str());
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
		CloseConn(conn);

	int date_fnum = PQfnumber(res, "date");
	int value_fnum = PQfnumber(res, "value");
	int feature_fnum = PQfnumber(res, "metaid");

	int nRows = PQntuples(res);

	Record row(features.size() + 1, -INFINITY);
	std::vector<Record> rows;
	//std::vector<std::string> existDates;

	double value;
	std::string feature;
	std::string date;
	std::string currentDate = "";
	int featureIndex;

	for (int i = 0; i < nRows; ++i)
	{
		date = std::string(PQgetvalue(res, i, date_fnum));
		if (currentDate.compare("") == 0) {
			currentDate = date;
		}

		if (currentDate.compare(date) != 0) {
			row[row.size() - 1] = double(getEpochTime(currentDate.c_str()));
			//existDates.push_back(currentDate);
			currentDate = date;
			rows.push_back(row);
			row.clear();
			row.assign(features.size() + 1, -INFINITY);
		}
		value = atof(PQgetvalue(res, i, value_fnum));
		feature = std::string(PQgetvalue(res, i, feature_fnum));
		featureIndex = lookupFeatureIndex(feature, features);
		if (featureIndex >= 0)
		{
			row[featureIndex] = value;
		}
	}
	// add last row, it is not added when for cycle ends
	//existDates.push_back(currentDate);
	row[row.size() - 1] = double(getEpochTime(currentDate.c_str()));
	rows.push_back(row);

	return rows;
}

struct CrossRecord
{
	std::string date;
	int quantity;
	int total;
	int tip;
	std::string type;
	std::vector<std::string> productIDS;
};

void printCrossRecord(const std::vector<CrossRecord> &vec)
{
	for (int i = 0; i < vec.size(); ++i)
	{
		std::cout << "\"" << vec[i].date << "\"" << " " << vec[i].quantity << " " << vec[i].total << " " << vec[i].tip << " " << vec[i].type << " "
			<< "{\"";
		for (auto j = 0; j < vec[i].productIDS.size() - 1; ++j)
		{
			std::cout << vec[i].productIDS[j] << "\", \"";
		}
		std::cout << vec[i].productIDS[vec[i].productIDS.size() - 1] << "\"}" << std::endl;
	}
}

void printRecords(const std::vector<Record> &mat, const std::vector<Feature> &features, int maxRows = -1, int maxCols = -1, int maxStringSize = 10)
{
	std::string str;
	maxStringSize = maxStringSize - 3;
	int startStrSize = ceil(maxStringSize/2) + 1;
	int endStrSize = floor(maxStringSize / 2);
	for (auto i = 0; i < features.size(); ++i)
	{
		str = features[i].bezeichnung;
		std::cout << str.replace(str.begin() + startStrSize, str.end() - endStrSize, "...");
		if (i < features.size() - 1)
		{
			std::cout << " | ";
		}

		if (maxCols >= 0 && i >= maxCols - 1)
		{
			if (features.size() > maxCols)
			{
				std::cout << " ... ";
			}
			break;
		}
	}
	std::cout << std::endl;


	for (auto i = 0; i < mat.size(); ++i)
	{
		// last item in the mat is date
		char buff[20];
		time_t date = mat[i][mat[i].size() - 1];
		std::cout << date << std::endl;
		strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&date));
		std::cout << buff << " -> { ";
		for (auto j = 0; j < mat[i].size() - 1; j++)
		{
			std::cout << mat[i][j];
			if (j < mat[i].size() - 1)
			{
				std::cout << " | ";
			}
			if (maxCols >= 0 && j >= maxCols - 1)
			{
				if (mat[i].size() > maxCols)
				{
					std::cout << " ... ";
				}
				break;
			}
		}
		std::cout << " }" << std::endl;

		if (maxRows >= 0 && i >= maxRows - 1)
		{
			if (mat.size() > maxRows)
			{
				std::cout << " ... " << std::endl;
			}
			break;
		}
	}
	std::cout << " Total rows: " << mat.size() << ", total features: " << features.size() << std::endl;
}

void printFeatures(const std::vector<Feature> &vec, int maxRows = -1)
{
	for (auto i = 0; i < vec.size(); ++i)
	{
		std::cout << "{ id: " << vec[i].id << ", bezeichnung: " << vec[i].bezeichnung << " }" << std::endl;

		if (maxRows >= 0 && i >= maxRows - 1)
		{
			if (vec.size() > maxRows)
			{
				std::cout << " ... " << std::endl;
			}
			break;
		}
	}
}

void saveToCsv(std::string filename, const std::vector<Record> &mat, const std::vector<Feature> &features)
{
	std::ofstream outputFile;

	// create and open the .csv file
	outputFile.open(filename);

	// write the file headers
	for (auto i = 0; i < features.size(); ++i)
	{
		outputFile << features[i].bezeichnung;
		outputFile << ";";
	}
	outputFile << "date";
	outputFile << std::endl;

	// last item in the mat is date
	for (auto i = 0; i < mat.size(); ++i)
	{
		//outputFile << dates[i] << ";";
		for (auto j = 0; j < mat[i].size(); j++)
		{
			if (j < mat[i].size() - 1)
			{
				outputFile << mat[i][j] << ";";
			}
			else
			{
				time_t date = mat[i][j];
				outputFile << date;
			}
		}
		outputFile << std::endl;
	}

	// close the output file
	outputFile.close();
}

std::mutex mu;

template <typename T>
std::vector<T> getFeatureVector(std::vector<std::vector<T>> source, int featureIndex, bool withNorm = false)
{
	std::vector<T> resampled;

	for (auto i = 0; i < source.size(); ++i)
	{
		if (source[i][featureIndex] == -INFINITY) {
			continue;
		}
		resampled.push_back(source[i][featureIndex]);
	}

	if(withNorm)
	{
		double min = resampled[0];
		double max = resampled[0];
		for (auto i = 0; i < resampled.size(); ++i)
		{
			if (resampled[i] > max)
			{
				max = resampled[i];
			}
			if (resampled[i] < min)
			{
				min = resampled[i];
			}
		}
		double range = max - min;
		if (range == 0)
		{
			range = 1;
		}
		for (auto i = 0; i < resampled.size(); ++i)
		{
			resampled[i] = (resampled[i] - min) / range;
		}
	}

	return resampled;
}

double runOOC(int featureIndex, std::vector<Record> dataset_0, std::vector<Record> dataset_1)
{
	double significantDifferent;

	auto t1 = std::chrono::steady_clock::now();
	auto featureVector_0 = getFeatureVector<double>(dataset_0, featureIndex, true);
	auto featureVector_1 = getFeatureVector<double>(dataset_1, featureIndex, true);

	utils::PMQ set0(featureVector_0);
	utils::PMQ set1(featureVector_1);
	significantDifferent = (set1 != set0);

	auto t2 = std::chrono::steady_clock::now();
	mu.lock();
	std::wcout << "feature #" << featureIndex << ": PMQ = " << significantDifferent << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << " s)" << std::endl;
	mu.unlock();

	return significantDifferent;
}

template <class Function>
double time_call(Function&& f)
{
	auto begin = std::chrono::steady_clock::now();
	f();
	auto end = std::chrono::steady_clock::now();
	return double(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
}

std::tuple < std::vector<Record>, std::vector<Feature> > readCsvData(std::string filename)
{
	// File pointer 
	std::fstream fin;

	// Open an existing file 
	fin.open(filename, std::ios::in);
	
	// Read the Data from the file 
	// as String Vector 
	Record row;
	std::string line, word;

	std::vector<Feature> features;
	std::vector<Record> rows;

	getline(fin, line);
	std::stringstream s(line);
	// omit 'date' feature
	//getline(s, word, ';');
	int i = 0;
	while (getline(s, word, ';')) 
	{
		Feature feature {i++, "", word};
		features.push_back(feature);
	}
	// omit 'date' feature
	features.erase(features.end() - 1);

	// read an entire row and 
	// store it in a string variable 'line' 
	while (getline(fin, line)) {

		row.clear();

		// used for breaking words 
		std::stringstream s(line);

		//getline(s, currentDate, ';');

		// read every column data of a row and 
		// store it in a string variable, 'word' 
		while (getline(s, word, ';')) {

			// add all the column data 
			// of a row to a vector 
			row.push_back(std::stod(word));
		}

		rows.push_back(row);
	}

	return std::make_tuple(rows, features);
}

template <typename T>
std::vector<std::vector<T>> resample(std::vector<std::vector<T>> source, int destinationRows)
{
	auto columns = source[0].size();
	std::vector<T> x;
	auto xi = linspace<T>(source[0][source[0].size() - 1], source[source.size() - 1][source[0].size() - 1], destinationRows);
	std::vector<T> y;
	std::vector<T> yi;
	std::vector<std::vector<T>> resampled(destinationRows, std::vector<T>(columns));

	for (auto f = 0; f < columns; ++f)
	{
		y.clear();
		x.clear();
		yi.clear();
		for (auto i = 0; i < source.size(); ++i)
		{
			if (source[i][f] == -INFINITY) {
				//std::cout << "omit " << i << " = " << source[i][featureIndex] << std::endl;
				continue;
			}
			// last feature is the date
			x.push_back(source[i][source[i].size() - 1]);
			y.push_back(source[i][f]);
		}
		if (y.size() > 0)
		{
			yi = akimaInterp1(x, y, xi);
			for (auto i = 0; i < yi.size(); ++i)
			{
				resampled[i][f] = yi[i];
			}
		}
		else
		{
			for (auto i = 0; i < xi.size(); ++i)
			{
				resampled[i][f] = 0;
			}
		}
	}

	return resampled;
}

template <typename T>
std::vector<std::vector<T>> resampleFeature(std::vector<std::vector<T>> source, int featureIndex, int destinationRows)
{
	std::vector<T> x;
	auto xi = linspace<T>(source[0][source[0].size() - 1], source[source.size() - 1][source[0].size() - 1], destinationRows);
	std::vector<T> y;
	std::vector<T> yi;
	std::vector<std::vector<T>> resampled(destinationRows, std::vector<T>(1));

	y.clear();
	yi.clear();
	for (auto i = 0; i < source.size(); ++i)
	{
		if (source[i][featureIndex] == -INFINITY) {
			//std::cout << "omit " << i << " = " << source[i][featureIndex] << std::endl;
			continue;
		}
		// last feature is the date
		x.push_back(source[i][source[i].size() - 1]);
		y.push_back(source[i][featureIndex]);
	}
	yi = akimaInterp1(x, y, xi);
	for (auto i = 0; i < yi.size(); ++i)
	{
		resampled[i][0] = yi[i];
	}

	return resampled;
}

double runVOI(int featureIndex, std::vector<Record> dataset_0, std::vector<Record> dataset_1)
{
	auto t1 = std::chrono::steady_clock::now();
	auto featureVector_0 = getFeatureVector<double>(dataset_0, featureIndex, true);
	auto featureVector_1 = getFeatureVector<double>(dataset_1, featureIndex, true);

	std::vector<std::vector<double>> featureVector_resh_0(featureVector_0.size(), std::vector<double>(1));
	std::vector<std::vector<double>> featureVector_resh_1(featureVector_1.size(), std::vector<double>(1));
	//std::vector<std::vector<double>> featureVector_resh_both(featureVector_0.size() + featureVector_1.size(), std::vector<double>(1));
	auto last_i = 0;
	for (auto i = 0; i < featureVector_0.size(); ++i)
	{
		last_i = i;
		featureVector_resh_0[i][0] = featureVector_0[i];
		//featureVector_resh_both[last_i][0] = featureVector_0[i];
	}
	for (auto i = 0; i < featureVector_1.size(); ++i)
	{
		featureVector_resh_1[i][0] = featureVector_1[i];
		//featureVector_resh_both[last_i + 1 + i][0] = featureVector_1[i];
	}

	//auto eX = metric::distance::entropy<double, metric::distance::Euclidian<double>>(featureVector_resh_0, 3, 2, metric::distance::Euclidian<double>());
	//auto eY = metric::distance::entropy<double, metric::distance::Euclidian<double>>(featureVector_resh_1, 3, 2, metric::distance::Euclidian<double>());
	//auto eXY = metric::distance::entropy<double, metric::distance::Chebyshev<double>>(featureVector_resh_both, 3, 2, metric::distance::Chebyshev<double>());

	//auto mi = metric::distance::mutualInformation<double>(featureVector_resh_0, featureVector_resh_1);

	//auto voi = eX + eY - 2 * mi;

	auto voi = metric::distance::variationOfInformation<double, metric::distance::Euclidian<double>>(featureVector_resh_0, featureVector_resh_1);

	//auto voi = sqrt(0.5 * (eXY - (eX + eY)));

	auto t2 = std::chrono::steady_clock::now();
	mu.lock();
	/*for (auto i = 0; i < featureVector_resh_0.size(); ++i)
	{
		std::wcout << featureVector_resh_0[i][0] << " ";
	}
	std::wcout << std::endl;
	std::wcout << std::endl;
	for (auto i = 0; i < featureVector_resh_1.size(); ++i)
	{
		std::wcout << featureVector_resh_1[i][0] << " ";
	}
	std::wcout << std::endl;*/
	std::wcout << "feature #" << featureIndex << ": VOI = " << voi << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << " s)" << std::endl;
	mu.unlock();

	return voi;
}


struct simple_user_euclidian
{
	double operator()(const std::vector<double> &a, const std::vector<double> &b) const
	{
		double sum = 0;
		for (size_t i = 0; i < a.size(); ++i)
		{
			sum += (a[i] - b[i]) * (a[i] - b[i]);
		}
		return std::sqrt(sum);
	}
};

double runCorrelation(int featureIndex1, int featureIndex2, std::vector<Record> dataset)
{
	auto t1 = std::chrono::steady_clock::now();
	auto featureVector_0 = getFeatureVector<double>(dataset, featureIndex1, true);
	auto featureVector_1 = getFeatureVector<double>(dataset, featureIndex2, true);

	std::vector<std::vector<double>> featureVector_resh_0(featureVector_0.size(), std::vector<double>(1));
	std::vector<std::vector<double>> featureVector_resh_1(featureVector_1.size(), std::vector<double>(1));
	//std::vector<std::vector<double>> featureVector_resh_both(featureVector_0.size() + featureVector_1.size(), std::vector<double>(1));
	auto last_i = 0;
	for (auto i = 0; i < featureVector_0.size(); ++i)
	{
		last_i = i;
		featureVector_resh_0[i][0] = featureVector_0[i];
		//featureVector_resh_both[last_i][0] = featureVector_0[i];
	}
	for (auto i = 0; i < featureVector_1.size(); ++i)
	{
		featureVector_resh_1[i][0] = featureVector_1[i];
		//featureVector_resh_both[last_i + 1 + i][0] = featureVector_1[i];
	}


	/* Set up the correlation function */
	using RecType = std::vector<double>;
	/* Build functors (function objects) with user types and metrics */
	typedef simple_user_euclidian Met;
	auto mgc_corr = metric::correlation::MGC<RecType, Met, RecType, Met>();
	auto result = mgc_corr.estimate(featureVector_resh_0, featureVector_resh_1, 100, 1.0, 100);


	auto t2 = std::chrono::steady_clock::now();
	//mu.lock();
	//std::wcout << "features #" << featureIndex1 << " <-> #" << featureIndex2 << " correlation = " << result << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << " s)" << std::endl;
	//mu.unlock();

	return result;
}

std::tuple <double, double> runImportance(int featureIndex, std::vector<Record> dataset_0, std::vector<Record> dataset_1)
{
	mu.lock();
	std::cout << "feature #" << featureIndex << ": started" << std::endl;
	mu.unlock();

	auto voi = runVOI(featureIndex, dataset_0, dataset_1);

	auto ooc = runOOC(featureIndex, dataset_0, dataset_1);

	mu.lock();
	std::cout << "feature #" << featureIndex << ": importance = " << voi * ooc << std::endl;
	mu.unlock();

	return std::make_tuple(voi, ooc);
}

template <typename T>
std::vector<T> getEvents(std::vector<Record> dataset, int featureIndex)
{
	std::vector<T> events;

	bool waitForMailfunction = false;
	for (auto i = 0; i < dataset.size(); ++i)
	{
		if (waitForMailfunction && dataset[i][featureIndex] == 1)
		{
			// last feature is date
			events.push_back(dataset[i - 1][dataset[i].size() - 1]);
			waitForMailfunction = false;
		}
		if (dataset[i][featureIndex] == 0)
		{
			waitForMailfunction = true;
		}
	}

	return events;
}

std::tuple <std::vector<std::vector<Record>>, std::vector<std::vector<Record>>, std::vector<Record>, std::vector<Record>> 
splitMailfunctionValues(std::vector<Record> dataset, std::vector<double> events, int malfunctedSeconds, int omitLastNumber, int allSeconds)
{
	int currentEventIndex = 0;
	auto currentEventDateTime = events[currentEventIndex];
	std::vector<Record> mailfuncted;
	std::vector<Record> valid;
	std::vector<std::vector<Record>> validByEvents(events.size(), std::vector<Record>());
	std::vector<std::vector<Record>> mailfunctedByEvents(events.size(), std::vector<Record>());
	for (auto i = 0; i < dataset.size(); ++i)
	{
		// last feature is date
		if (dataset[i][dataset[i].size() - 1] > currentEventDateTime - malfunctedSeconds)
		{
			//mailfuncted.push_back(dataset[i]);
			mailfunctedByEvents[currentEventIndex].push_back(dataset[i]);
		}
		else
		{
			valid.push_back(dataset[i]);
		}

		if (dataset[i][dataset[i].size() - 1] > currentEventDateTime - allSeconds)
		{
			validByEvents[currentEventIndex].push_back(dataset[i]);
		}

		if (dataset[i][dataset[i].size() - 1] >= currentEventDateTime)
		{
			currentEventIndex++;
			if (currentEventIndex < events.size())
			{
				currentEventDateTime = events[currentEventIndex];
			}
			else
			{
				break;
			}
		}
	}

	for (auto i = 0; i < mailfunctedByEvents.size(); ++i)
	{
		std::cout << "Observed variables for event #" << i << ": " << mailfunctedByEvents[i].size() << std::endl;
		if (mailfunctedByEvents[i].size() > omitLastNumber)
		{
			mailfuncted.insert(mailfuncted.end(), mailfunctedByEvents[i].begin(), mailfunctedByEvents[i].end() - omitLastNumber);
			mailfunctedByEvents[i] = std::vector<Record>(mailfunctedByEvents[i].begin(), mailfunctedByEvents[i].end() - omitLastNumber);
		}
	}
	std::cout << "Total observed variables: " << mailfuncted.size() << std::endl;

	return std::make_tuple(mailfunctedByEvents, validByEvents, mailfuncted, valid);
}

template <typename T>
std::vector<size_t> sort_indexes(const std::vector<T> &v) {

	// initialize original index locations
	std::vector<size_t> idx(v.size());
	iota(idx.begin(), idx.end(), 0);

	// sort indexes based on comparing values in v
	std::sort(idx.begin(), idx.end(),
		[&v](size_t i1, size_t i2) {return v[i1] > v[i2]; });

	return idx;
}

void calculateVariances(const std::vector<Feature> &features, std::vector<double> events, std::vector<Record> dataset, std::string name) {


	// select Düsenbefeuchter Stellglied
	// and calculate variances along time

	//std::cout << "Düsenbefeuchter Stellglied features: " << std::endl;
	cross::filter<Feature> featuresFilter(features);
	auto ds_featureFilter = featuresFilter.dimension([](Feature f) { return f.bezeichnung; });
	ds_featureFilter.filter([](auto d) { return d.rfind("Düsenbefeuchter Stellglied", 0) == 0; });
	auto ds_features = featuresFilter.all_filtered();

	std::vector<std::vector<double>> dsAlongTime;
	std::vector<std::vector<Record>> variancesByEvents(events.size(), std::vector<Record>());
	std::vector<std::vector<double>> variances;
	int currentEventIndex = 0;
	auto currentEventDateTime = events[currentEventIndex];
	for (auto r = 0; r < dataset.size(); ++r)
	{
		std::vector<double> timeVec;
		for (auto i = 0; i < ds_features.size(); ++i)
		{
			//std::cout << ds_features[i].bezeichnung << std::endl;
			timeVec.push_back(dataset[r][ds_features[i].index]);
		}
		dsAlongTime.push_back(timeVec);

		utils::PMQ pmq(timeVec);
		auto variance = pmq.variance();

		if (!std::isnan(variance)) {
			// 1 if observations in range 300 seconds before event
			double isBeforeEvent = 0;
			if (dataset[r][dataset[r].size() - 1] > currentEventDateTime - 300)
			{
				isBeforeEvent = 1;
			}
			variances.push_back({ variance,  isBeforeEvent, dataset[r][dataset[r].size() - 1] });
			

			if (dataset[r][dataset[r].size() - 1] > currentEventDateTime - 4 * 3600)
			{
				variancesByEvents[currentEventIndex].push_back({ variance,  isBeforeEvent, dataset[r][dataset[r].size() - 1] });
			}

		}

		if (dataset[r][dataset[r].size() - 1] >= currentEventDateTime)
		{
			currentEventIndex++;
			if (currentEventIndex < events.size())
			{
				currentEventDateTime = events[currentEventIndex];
			}
			else
			{
				break;
			}
		}
	}

	Feature varianceName = { 0, "0", "variance" };
	Feature isEventName = { 1, "1", "isBeforeEvent" };
	saveToCsv(name, variances, { varianceName, isEventName });


	for (auto r = 0; r < variancesByEvents.size(); ++r)
	{
		saveToCsv("event_" + std::to_string(r) + "_" + name, variancesByEvents[r], { varianceName, isEventName });
	}
}

int main(int argc, char *argv[])
{
	bool FROM_CSV = false;
	bool CLUSTERING_FROM_VOI_OOC = false;
	std::string sensorDataCsvFilename = "sensorsData.csv";
	std::string VOIxOOCcsvFilename = "PMQxVOI_result.csv";
	std::cout << argc << std::endl;
	if (argc > 1)
	{
		FROM_CSV = true;
		sensorDataCsvFilename = argv[1];
		if (argc > 2)
		{
			CLUSTERING_FROM_VOI_OOC = true;
			VOIxOOCcsvFilename = argv[2];
			std::cout << "we have started, start clustering on the data from: " << VOIxOOCcsvFilename << std::endl;
		}
		else
		{
			std::cout << "we have started, load from csv: " << sensorDataCsvFilename << std::endl;
		}
	}
	else
	{
		std::cout << "we have started, load from postgres" << std::endl;
	}
	std::cout << '\n';

	std::vector<std::string> timestamps;
	std::vector<Feature> features;

	std::vector<Record> records;
	//std::vector<std::string> recordDates;

	static int targetFeatureIndex;
	std::string featureName = "Sammelabriss";
	//std::string featureName = "Gutproduktion";


	if (FROM_CSV)
	{
		auto t1 = std::chrono::steady_clock::now();

		std::tie(records, features) = readCsvData(sensorDataCsvFilename);

		auto t2 = std::chrono::steady_clock::now();

		std::cout << '\n';
		std::cout << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << " s)" << std::endl;
		std::cout << '\n';


		std::cout << "num features: " << features.size() << std::endl;

		targetFeatureIndex = lookupFeatureIndex(featureName, features);
	}
	else
	{

		PGconn     *conn = NULL;

		conn = ConnectDB();

		auto numFeatures = countFeatures(conn);

		std::cout << "num features: " << numFeatures << std::endl;

		features = getFeatures(conn);

		printFeatures(features, 5);

		targetFeatureIndex = lookupFeatureIndex(featureName, features);

		std::cout << "feature " << featureName << " id: " << features[targetFeatureIndex].id << std::endl;

		timestamps = getTargetTimestamps(conn, features[targetFeatureIndex].id);

		vector_print(timestamps, 5);

		//

		auto t1 = std::chrono::steady_clock::now();
		/*for (int i = 0; i < timestamps.size(); ++i)
		{
			records.push_back(getSensorRecord(conn, timestamps[i], features));
		}*/
		std::vector<std::string> cuttimestamps(timestamps.begin() + 2000, timestamps.begin() + 2170);
		records = getAllSensorRecords(conn, features, cuttimestamps);
		//records = getAllSensorRecords(conn, features, timestamps);
		auto t2 = std::chrono::steady_clock::now();

		std::cout << '\n';
		std::cout << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << " s)" << std::endl;
		std::cout << '\n';

		saveToCsv(sensorDataCsvFilename, records, features);

		//auto dataset_0 = getSensorData(conn, "SELECT * FROM public.sensordata WHERE date = '2018-12-10 23:43:15' LIMIT 100", features);
		//auto dataset_0 = getSensorData(conn, "SELECT * FROM public.sensordata WHERE metaid @> '{1,7,8}'::int[] LIMIT 10000");
		//auto dataset_1 = getSensorData(conn, "SELECT * FROM public.sensordata WHERE metaid @> '{1,7,8}'::int[] AND value = '1' LIMIT 100");
		CloseConn(conn);
	}

	printRecords(records, features, 10, 10, 15);


	std::cout << '\n';
	std::cout << '\n';

	std::cout << "feature " << featureName << " index: " << targetFeatureIndex << std::endl;
	std::cout << "feature " << featureName << " label: " << features[targetFeatureIndex].bezeichnung << std::endl;

	cross::filter<Record> recordsFilter(records);
	std::vector<Record> filtered_results;
	auto featureFilter = recordsFilter.dimension([](Record r) { return r[targetFeatureIndex]; });
	featureFilter.filter(0);
	//featureFilter.filter([](auto d) { return d >= 80; });
	auto dataset_0 = recordsFilter.all_filtered();
	std::cout << '\n';
	std::cout << '\n';
	std::cout << featureName << " == 0" << std::endl;
	printRecords(dataset_0, features, 10, 10, 15);

	//featureFilter.filter();
	//featureFilter.filter(1);
	////featureFilter.filter([](auto d) { return d < 80; });
	//auto dataset_1 = recordsFilter.all_filtered();
	//std::cout << '\n';
	//std::cout << '\n';
	//std::cout << featureName << " == 1" << std::endl;
	//printRecords(dataset_1, features, 10, 10, 15);

	//////////////////////

	std::cout << '\n';
	std::cout << '\n';
	auto events = getEvents<double>(records, targetFeatureIndex);
	std::cout << "mailfunction events: " << events.size() << std::endl;

	//////////////////////

	std::vector<Record> mailfunctedDataset;
	std::vector<Record> validDataset;
	std::vector<std::vector<Record>> mailfunctedDatasetByEvent;
	std::vector<std::vector<Record>> validDatasetByEvent;

	std::tie(mailfunctedDatasetByEvent, validDatasetByEvent, mailfunctedDataset, validDataset) = splitMailfunctionValues(records, events, 300, 6, 4 * 3600);

	for (int i = 0; i < mailfunctedDatasetByEvent.size(); ++i)
	{
		saveToCsv("mailfunctedDataset_event_" + std::to_string(i) + ".csv", mailfunctedDatasetByEvent[i], features);
		saveToCsv("allDataset_event_" + std::to_string(i) + ".csv", validDatasetByEvent[i], features);
	}
	saveToCsv("allDataset.csv", records, features);
	saveToCsv("mailfunctedDataset.csv", mailfunctedDataset, features);

	////////////////////

	std::cout << '\n';
	std::cout << '\n';
	std::cout << "Resampled:" << std::endl;
	auto dataset_0_i = resample<double>(mailfunctedDataset, 100);
	auto dataset_1_i = resample<double>(validDataset, 100);
	auto dataset_all_i = resample<double>(validDataset, 10000);
	saveToCsv("allDataset_10000.csv", dataset_all_i, features);

	//auto records_i = resample<double>(records, 5000);
	//saveToCsv("sensorsRecords_1000.csv", records_i, features);


	/*auto e = entropy<double, metric::distance::P_norm<double>>(dataset_0_i, 3, 2, metric::distance::P_norm<double>(3));
	std::cout << "H(X) General Minkowsky, 3: " << e << std::endl;

	e = metric::distance::entropy<double, metric::distance::P_norm<double>>(dataset_0_i, 3, 2, metric::distance::P_norm<double>(2));
	std::cout << "H(X) General Minkowsky, 2: " << e << std::endl;

	e = metric::distance::entropy<double, metric::distance::Euclidian<double>>(dataset_0_i, 3, 2, metric::distance::Euclidian<double>());
	std::cout << "H(X) Euclidean: " << e << std::endl;

	e = metric::distance::entropy<double, metric::distance::P_norm<double>>(dataset_0_i, 3, 2, metric::distance::P_norm<double>(1));
	std::cout << "H(X) General Minkowsky, 1: " << e << std::endl;

	e = metric::distance::entropy<double, metric::distance::Manhatten<double>>(dataset_0_i, 3, 2, metric::distance::Manhatten<double>());
	std::cout << "H(X) Manhatten: " << e << std::endl;

	e = metric::distance::entropy<double, metric::distance::Chebyshev<double>>(dataset_0_i, 3, 2, metric::distance::Chebyshev<double>());
	std::cout << "H(X) Chebyshev: " << e << std::endl;

	e = metric::distance::entropy<double, metric::distance::Chebyshev<double>>(dataset_1_i, 3, 2, metric::distance::Chebyshev<double>());
	std::cout << "H(Y) Chebyshev: " << e << std::endl;


	std::cout << "\n";*/


	//std::cout << "I(X,X) " << mutualInformation<double>(dataset_0_i, dataset_0_i) << std::endl;
	//std::cout << "I(X,X) " << mutualInformation<double, metric::distance::Chebyshev<double>>(dataset_0, dataset_0, 3, metric::distance::Chebyshev<double>(), 1) << std::endl;
	//std::cout << "I(Y,Y) " << mutualInformation<double>(dataset_1_i, dataset_1_i) << std::endl;
	//std::cout << "I(Y,Y) " << mutualInformation<double, metric::distance::Chebyshev<double>>(dataset_1, dataset_1, 3, metric::distance::Chebyshev<double>(), 1) << std::endl;


	//auto total_t1 = std::chrono::steady_clock::now();

	//auto eX = entropy<double, metric::distance::Chebyshev<double>>(dataset_0_i, 3, 2, metric::distance::Chebyshev<double>());
	//auto eY = entropy<double, metric::distance::Chebyshev<double>>(dataset_1_i, 3, 2, metric::distance::Chebyshev<double>());

	//auto mi = mutualInformation<double>(dataset_0_i, dataset_1_i);
	//std::cout << "I(X,Y) " << mi << std::endl;
	////std::cout << "I(X,Y) " << mutualInformation<double, metric::distance::Chebyshev<double>>(dataset_0, dataset_1, 3, metric::distance::Chebyshev<double>(), 1) << std::endl;

	//auto voi = eX + eY - 2 * mi;
	//std::cout << "\n";
	//std::cout << "\n";
	//std::cout << "VOI = " << voi << std::endl;

	//auto total_t2 = std::chrono::steady_clock::now();
	//auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(total_t2 - total_t1).count();
	//std::cout << " (Total time = " << elapsed / 1000000 << " s)" << std::endl;

	////////////////////

	/*double ooc;
	std::vector<double> vois;
	std::vector<double> ooces;
	std::vector<double> importantes;
	std::vector<std::vector<double>> resampledFeature_0;
	std::vector<std::vector<double>> resampledFeature_1;*/

	std::vector<double> importances(features.size() + 1, -1);
	std::vector<double> vois(features.size() + 1, -1);
	std::vector<double> oocs(features.size() + 1, -1);

	auto total_t1 = std::chrono::steady_clock::now();

	unsigned concurentThreadsSupported = std::thread::hardware_concurrency();
	std::cout << "Num cores: " << concurentThreadsSupported << std::endl;
	ThreadPool pool(concurentThreadsSupported);
	const int count = features.size();
	Semaphore sem;

	for (int featureIndex = 0; featureIndex < count; ++featureIndex)
	{
		/*pool.execute([featureIndex, &sem, &importances, &dataset_0, &dataset_1]() {
			importances.at(featureIndex) = runImportance(featureIndex, dataset_0, dataset_1);
			sem.notify();
		});*/
		pool.execute([featureIndex, &sem, &vois, &oocs, &dataset_0_i, &dataset_1_i]() {
			double voi = -INFINITY;
			double ooc = -INFINITY;

			try {
				std::tie(voi, ooc) = runImportance(featureIndex, dataset_0_i, dataset_1_i);
			}
			catch (const std::runtime_error& e) {
				std::cout << "feature #" << featureIndex << ": runtime error: " << e.what() << std::endl;
			}
			catch (const std::exception& e) {
				std::cout << "feature #" << featureIndex << ": exception: " << e.what() << std::endl;
			}
			catch (...) {
				std::cout << "feature #" << featureIndex << ": unknown error" << std::endl;
			}

			oocs.at(featureIndex) = ooc;
			vois.at(featureIndex) = voi;

			mu.lock();
			std::cout << "feature #" << featureIndex << ": finished" << std::endl;
			mu.unlock();

			sem.notify();
		});

		//auto imp = runImportance(featureIndex, dataset_0_i, dataset_1_i);
		//std::wcout << "PMQ and VOI ends, imp = " << imp << std::endl;
	}
	for (int i = 0; i < count; ++i)
	{
		sem.wait();
	}
	pool.close();


	double min = vois[0];
	double max = vois[0];
	for (auto i = 0; i < vois.size(); ++i)
	{
		if (vois[i] > max)
		{
			max = vois[i];
		}
		if (vois[i] < min)
		{
			min = vois[i];
		}
	}
	double range = max - min;
	if (range == 0)
	{
		range = 1;
	}
	for (auto i = 0; i < vois.size(); ++i)
	{
		vois[i] = (vois[i] - min) / range;
	}

	auto total_t2 = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(total_t2 - total_t1).count();
	std::wcout << "PMQ and VOI ends";
	std::wcout << '\n';
	std::wcout << '\n';
	std::wcout << '\n';

	vois[vois.size() - 1] = 0;
	oocs[oocs.size() - 1] = 1;

	std::vector<Record> importancesAsRecord;
	Record voi_r(vois);
	importancesAsRecord.push_back(voi_r);
	Record ooc_r(oocs);
	importancesAsRecord.push_back(ooc_r);
	printRecords(importancesAsRecord, features, 10, 10, 15);
	std::wcout << " (Total time = " << elapsed / 1000000 << " s)" << std::endl;
	std::wcout << '\n';
	std::wcout << '\n';


	saveToCsv("PMQxVOI_result.csv", importancesAsRecord, features);

	for (size_t i = 0; i < vois.size() - 1; i++)
	{
		importances[i] = vois[i] * oocs[i];
	}

	auto importances_indexes = sort_indexes(importances);

	std::vector<Feature> top_200;

	for (size_t i = 0; i < 200; i++)
	{
		top_200.push_back(features[importances_indexes[i]]);
		std::cout << "Importance = " << importances[importances_indexes[i]] << std::endl;
	}

	/////////////////


	//unsigned int iterations = 1000;

	//using Vector = std::vector<double>;
	//using Metric = metric::distance::Euclidian<Vector::value_type>;
	//using Graph = metric::graph::Grid6;

	//int dimensionX = 5;

	//metric::mapping::SOM<Vector, Metric, Graph> DR(dimensionX, dimensionX);

	//if (!DR.isValid()) {
	//	std::cout << "SOM is not valid" << std::endl;
	//	return EXIT_FAILURE;
	//}


	//std::vector<Record> VOIxOOCdata;
	//std::tie(VOIxOOCdata, features) = readCsvData(VOIxOOCcsvFilename);

	//std::vector<std::vector<double>> somSpace;
	//std::vector<std::vector<int>> clusters(dimensionX * dimensionX, std::vector<int>());

	//for (auto i = 0; i < VOIxOOCdata[0].size(); ++i)
	//{
	//	if (VOIxOOCdata[0][i] != -INF && VOIxOOCdata[1][i] != -INF)
	//	{
	//		somSpace.push_back({ VOIxOOCdata[0][i], VOIxOOCdata[1][i] });
	//	}
	//}

	////somSpace.push_back({ 0, 0 });
	////somSpace.push_back({ 0, 1 });
	////somSpace.push_back({ 1, 0 });
	////somSpace.push_back({ 1, 1 });
	////somSpace.push_back({ 5, 5 });
	////somSpace.push_back({ 5, 6 });
	////somSpace.push_back({ 6, 5 });
	////somSpace.push_back({ 6, 6 });

	///* Train */
	//const auto t1 = std::chrono::steady_clock::now();
	//DR.train(somSpace, iterations);
	//const auto t2 = std::chrono::steady_clock::now();

	//std::cout << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "s)" << std::endl;

	//std::cout << "size = " << DR.size() << std::endl;

	//for (auto i = 0; i < somSpace.size(); ++i)
	//{
	//	int cluster = DR.BMU(somSpace[i]);
	//	clusters[cluster].push_back(i);
	//}

	//std::cout << "Nodes " << std::endl;
	//auto nodes = DR.nodes();
	//std::vector<double> topRightNode = nodes[0];
	//int topRightNodeIndex = 0;
	//for (auto i = 0; i < nodes.size(); ++i)
	//{
	//	if (nodes[i][0] > topRightNode[0] || nodes[i][1] > topRightNode[1])
	//	{
	//		topRightNode = nodes[i];
	//		topRightNodeIndex = i;
	//	}
	//	vector_print(nodes[i]);
	//	std::cout << "Cluster #" << i << " size: " << clusters[i].size() << std::endl;
	//}
	//std::wcout << '\n';
	//std::wcout << '\n';
	//std::cout << "Top right cluster, feature indexes: " << topRightNodeIndex << std::endl;
	//std::cout << "Cluster size: " << clusters[topRightNodeIndex].size() << std::endl;
	//std::cout << "Top right cluster indexes: " << std::endl;

	//vector_print(clusters[topRightNodeIndex]);

	std::vector<std::string> sensorNames;
	std::vector<std::vector<double>> sensorsCorrelations(top_200.size(), std::vector<double>(top_200.size()));
	std::wcout << '\n';
	std::wcout << '\n';
	std::cout << "Top right cluster features: " << std::endl;

	std::vector<std::vector<double>> correlationsMailfuncted(top_200.size(), std::vector<double>(top_200.size()));
	std::vector<std::vector<double>> correlationsAll(top_200.size(), std::vector<double>(top_200.size()));
	std::vector<std::vector<double>> correlationsDifference(top_200.size(), std::vector<double>(top_200.size()));

	for (auto i = 0; i < top_200.size(); ++i)
	{
		try {
			std::cout << i + 1 << "/" << top_200.size() << " -> feature name: " << top_200[i].bezeichnung << " " << top_200[i].id << std::endl;
			sensorNames.push_back(top_200[i].bezeichnung);
			
			int feature1 = top_200[i].index;
			int feature2;

			for (auto k = i; k < top_200.size(); ++k)
			{
				feature2 = top_200[k].index;
				correlationsMailfuncted[i][k] = runCorrelation(feature1, feature2, dataset_0_i);
				correlationsAll[i][k] = runCorrelation(feature1, feature2, dataset_1_i);
				correlationsDifference[i][k] = correlationsAll[i][k] - correlationsMailfuncted[i][k];

				correlationsMailfuncted[k][i] = correlationsMailfuncted[i][k];
				correlationsAll[k][i] = correlationsAll[i][k];
				correlationsDifference[k][i] = correlationsDifference[i][k];
			}
		}
		catch (const std::runtime_error& e) {
			std::cout << "feature #" << i << ": runtime error: " << e.what() << std::endl;
		}
		catch (const std::exception& e) {
			std::cout << "feature #" << i << ": exception: " << e.what() << std::endl;
		}
		catch (...) {
			std::cout << "feature #" << i << ": unknown error" << std::endl;
		}

		//for (auto k = i + 1; k < clusters[topRightNodeIndex].size(); ++k)
		//{
		//	sensorsCorrelations[i][k] = runCorrelation(clusters[topRightNodeIndex][i], clusters[topRightNodeIndex][k], dataset_0_i);
		//}
	}

	saveToCsv("correlationsMailfuncted.csv", correlationsMailfuncted, top_200);
	saveToCsv("correlationsAll.csv", correlationsAll, top_200);
	saveToCsv("correlationsDifference.csv", correlationsDifference, top_200);

	//std::wcout << '\n';
	//std::wcout << '\n';
	//std::cout << "Sensors correlation matrix: " << std::endl;
	//matrix_print(sensorsCorrelations);
	std::wcout << '\n';
	std::wcout << '\n';
	std::cout << "sensor Names Distance Matrix:" << std::endl;

	metric::distance::Edit<std::string> distance;
	std::vector<std::vector<double>> sensorNamesDistanceMatrix(sensorNames.size(), std::vector<double>(sensorNames.size()));

	for (auto i = 0; i < sensorNames.size(); ++i)
	{
		for (auto j = 0; j < sensorNames.size(); ++j)
		{
			sensorNamesDistanceMatrix[i][j] = distance(sensorNames[i], sensorNames[j]);
		}
	}
	std::wcout << '\n';
	std::wcout << '\n';
	std::cout << "sensor Clusters:" << std::endl;


	std::vector<std::vector<std::string>> sensorsClusters;
	std::vector<std::string> existSensorNames = sensorNames;
	for (auto i = 0; i < sensorNames.size(); ++i)
	{
		auto it = std::find(existSensorNames.begin(), existSensorNames.end(), sensorNames[i]);
		if (it != existSensorNames.end()) {

			std::vector<std::string> cluster;
			cluster.push_back(sensorNames[i]);
			existSensorNames.erase(it);

			for (auto k = i + 1; k < sensorNames.size(); ++k)
			{
				auto it2 = std::find(existSensorNames.begin(), existSensorNames.end(), sensorNames[k]);
				if (it2 != existSensorNames.end()) {
					if (sensorNamesDistanceMatrix[i][k] < 10)
					{
						cluster.push_back(sensorNames[k]);
						existSensorNames.erase(it2);
					}
				}
			}
			sensorsClusters.push_back(cluster); 
		}
	}

	for (auto i = 0; i < sensorsClusters.size(); ++i)
	{
		std::cout << "cluster #" << i << std::endl;
		std::vector<std::vector<double>> correlationsMailfuncted(sensorsClusters[i].size(), std::vector<double>(sensorsClusters[i].size()));
		std::vector<std::vector<double>> correlationsAll(sensorsClusters[i].size(), std::vector<double>(sensorsClusters[i].size()));
		std::vector<std::vector<double>> correlationsDifference(sensorsClusters[i].size(), std::vector<double>(sensorsClusters[i].size()));
		for (size_t j = 0; j < sensorsClusters[i].size(); j++)
		{
			std::cout << sensorsClusters[i][j] << std::endl;
			int feature1 = lookupFeatureIndex(sensorsClusters[i][j], features);
			int feature2;

			for (auto k = j + 1; k < sensorsClusters[i].size(); ++k)
			{
				feature2 = lookupFeatureIndex(sensorsClusters[i][k], features);
				correlationsMailfuncted[j][k] = runCorrelation(feature1, feature2, dataset_0_i);
				correlationsAll[j][k] = runCorrelation(feature1, feature2, dataset_1_i);
				correlationsDifference[j][k] = correlationsAll[j][k] - correlationsMailfuncted[j][k];
			}
		}
		std::cout << "Sensors correlation for mailfuncted data matrix: " << std::endl;
		matrix_print(correlationsMailfuncted);
		std::cout << "Sensors correlation for all data matrix: " << std::endl;
		matrix_print(correlationsAll);
		std::cout << "Difference netween correlations matrix: " << std::endl;
		matrix_print(correlationsDifference);
		std::cout << std::endl;
	}

	//calculateVariances(features, events, records, "all_features_variances.csv");
	calculateVariances(top_200, events, records, "top_200_features_variances.csv");


	//auto hc = clustering::HierarchicalClustering<std::string, metric::distance::Edit<std::string>>(sensorNames, 10);
	//hc.hierarchical_clustering();

	//for (size_t i = 0; i < hc.clusters.size(); i++)
	//{
	//	std::cout << "cluster #" << i << std::endl;
	//	for (size_t j = 0; j < hc.clusters[i].data.size(); j++)
	//	{
	//		std::cout << hc.clusters[i].data[j] << std::endl;
	//	}
	//	std::cout << std::endl;
	//}

	return 0;

}
