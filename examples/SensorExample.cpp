#include <variant>
#include <chrono>
#include <deque> // for Record test
#include <vector>
#include <thread>
#include <iostream>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <codecvt>
#include <random>

#include <string.h>
#include <libpq-fe.h>

//#include "utils/poor_mans_quantum.hpp"

#include "modules/mapping/details/classification/metric_classification.hpp"
//#include "../details/classification/details/correlation_weighted_accuracy.hpp"

template <typename T0, typename ... Ts>
std::ostream & operator<< (std::ostream & s,
	std::variant<T0, Ts...> const & v)
{
	std::visit([&](auto && arg) { s << arg; }, v); return s;
}

template <typename T>
void vector_print(const std::vector<T> &vec)
{

	std::cout << "[";
	for (size_t i = 0; i < vec.size(); i++)
	{
		if (i < vec.size() - 1)
		{
			std::cout << vec[i] << ", ";
		}
		else
		{
			std::cout << vec[i] << "]" << std::endl;
		}
	}
}


template <typename T>
void matrix_print(const std::vector<std::vector<T>> &mat, int maxRows = -1)
{

	std::cout << "[" << std::endl;
	for (int i = 0; i < mat.size(); i++)
	{
		std::cout << "  [ ";
		for (int j = 0; j < mat[i].size() - 1; j++)
		{
			std::cout << mat[i][j] << ", ";
		}
		std::cout << mat[i][mat[i].size() - 1] << " ]" << std::endl;

		if (maxRows >= 0 && i > maxRows)
		{
			std::cout << "  ..." << std::endl;
			break;
		}
	}
	std::cout << "]" << std::endl;
}

void query_print(PGresult *res)
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
	for (int i = 0; i < nrows; i++)
	{
		for (int j = 0; j < ncols; j++)
		{
			char* v = PQgetvalue(res, i, j);
			printf("%s | ", v);
		}
		printf("\n");
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
	std::string m_dbhost = "95.216.78.62";
	int         m_dbport = 5432;
	std::string m_dbname = "DAA";
	std::string m_dbuser = "stepan";
	std::string m_dbpass = "Ghokle73";

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
typedef std::vector<long int> Record;
std::vector<Record> getSensorData(PGconn *conn, const char *queryString)
{
	// Execute with sql statement
	PGresult *res = NULL;

	res = PQexec(conn, queryString);
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
		CloseConn(conn);

	int date_fnum = PQfnumber(res, "date");
	int value_fnum = PQfnumber(res, "value");

	int nrows = PQntuples(res);

	long int t;
	long int b;

	std::vector<Record> dataSet;
	for (int i = 0; i < nrows; i++)
	{
		t = static_cast<long int> (getEpochTime(PQgetvalue(res, i, date_fnum)));
		b = static_cast<long int> (!strcmp(PQgetvalue(res, i, value_fnum), "1"));

		dataSet.push_back({ t, b });
	}

	// Clear result
	PQclear(res);

	return dataSet;
}

int main()
{
	std::cout << "we have started" << std::endl;
	std::cout << '\n';

	PGconn     *conn = NULL;

	conn = ConnectDB();
	auto dataset_0 = getSensorData(conn, "SELECT * FROM public.sensordata WHERE metaid @> '{1,7,8}'::int[]");
	//auto dataset_1 = getSensorData(conn, "SELECT * FROM public.sensordata WHERE metaid @> '{1,7,8}'::int[] AND value = '1' LIMIT 100");
	CloseConn(conn);


	matrix_print(dataset_0, 10);
	//matrix_print(dataset_1, 10);

	//PMQ set0(dataset_0);
	//PMQ set1(dataset_1);
	//float SignificantDifferent = (set1 != set0); // values between 0...1

	//using Record1 = std::vector<int>;  // may be of arbitrary type, with appropriate accessors

	/*std::vector<Record1> payments = {
		{0,3,5,0},
		{1,4,5,0},
		{2,5,2,1},
		{3,6,2,1}
	};*/

	std::vector<std::function<double(Record)>> features;

	for (int i = 0; i < (int)dataset_0[0].size() - 1; ++i) {
		features.push_back(
			[=](auto r) { return r[i]; }  // we need closure: [=] instead of [&]   !! THIS DIFFERS FROM API !!
		);
	}

	std::function<bool(Record)> response = [](Record r) {
		if (r[r.size() - 1] > 0.5)
			return true;
		else
			return false;
	};


	std::random_device rd;     // only used once to initialise (seed) engine
	std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
	std::uniform_int_distribution<int> uni(0, dataset_0.size()); // guaranteed unbiased

	auto random_integer = uni(rng);
	std::vector<Record> test_sample = {
		dataset_0[uni(rng)],
		dataset_0[uni(rng)],
		dataset_0[uni(rng)],
		dataset_0[uni(rng)],
		dataset_0[uni(rng)]
	};


	std::vector<bool> prediction;
	auto startTime = std::chrono::steady_clock::now();
	auto endTime = std::chrono::steady_clock::now();

	// test on int vector 

	std::cout << "SVM on int vector: " << std::endl;
	startTime = std::chrono::steady_clock::now();
	metric::classification::edmClassifier<Record, CSVM> svmModel_1 = metric::classification::edmClassifier<Record, CSVM>();
	std::cout << "training... " << std::endl;
	svmModel_1.train(dataset_0, features, response);
	endTime = std::chrono::steady_clock::now();
	std::cout << "trained (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000000 << " s)" << std::endl;

	svmModel_1.predict(test_sample, features, prediction);
	std::cout << "test sample: " << std::endl;
	matrix_print(test_sample);
	std::cout << "prediction: " << std::endl;
	vector_print(prediction);

	std::cout << "\n";

	return 0;

}
