#include <variant>
#include <chrono>
#include <deque> // for Record test
#include <vector>
#include <thread>
#include <iostream>
#include <string>
#include <libpq-fe.h>


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
void matrix_print(const std::vector<std::vector<T>> &mat)
{

	std::cout << "[";
	for (int i = 0; i < mat.size(); i++)
	{
		for (int j = 0; j < mat[i].size() - 1; j++)
		{
			std::cout << mat[i][j] << ", ";
		}
		std::cout << mat[i][mat[i].size() - 1] << " ]" << std::endl;
		;
	}
}

/* Close connection to database */
void CloseConn(PGconn *conn)
{
	PQfinish(conn);
	printf("Connection closed\n");
	getchar();
	exit(1);
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

void ExecQuery(PGconn *conn, const char *queryString)
{
	// Execute with sql statement
	PGresult *res = NULL;

	res = PQexec(conn, queryString);
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
		CloseConn(conn);

	int ncols = PQnfields(res);
	printf("There are %d columns:", ncols);
	for (int i = 0; i < ncols; i++)
	{
		char *name = PQfname(res, i);
		printf(" %s", name);
	}
	printf("\n");

	int nrows = PQntuples(res);
	for (int i = 0; i < nrows; i++)
	{
		char* count = PQgetvalue(res, i, 0);
		printf("Count: %s\n",
			count);
	}

	printf("Total: %d rows\n", nrows);

	// Clear result
	PQclear(res);
}

int main()
{
	std::cout << "we have started" << std::endl;
	std::cout << '\n';

	//PMQ set1(data1);
	//PMQ set0(data0);
	//float SignificantDifferent = (set1 != set0) // values between 0...1

	PGconn     *conn = NULL;

	conn = ConnectDB();
	ExecQuery(conn, "SELECT COUNT(*) FROM public.sensordata");

	CloseConn(conn);

	//using Record = std::vector<int>;  // may be of arbitrary type, with appropriate accessors

	//std::vector<Record> payments = {
	//	{0,3,5,0},
	//	{1,4,5,0},
	//	{2,5,2,1},
	//	{3,6,2,1}
	//};

	//std::vector<std::function<double(Record)>> features;

	//for (int i = 0; i < (int)payments[0].size() - 1; ++i) {
	//	features.push_back(
	//		[=](auto r) { return r[i]; }  // we need closure: [=] instead of [&]   !! THIS DIFFERS FROM API !!
	//	);
	//}

	//std::function<bool(Record)> response = [](Record r) {
	//	if (r[r.size() - 1] >= 0.5)
	//		return true;
	//	else
	//		return false;
	//};

	//std::vector<Record> test_sample = {
	//	{0,3,5,0},
	//	{3,6,2,1}
	//};


	//std::vector<bool> prediction;
	//auto startTime = std::chrono::steady_clock::now();
	//auto endTime = std::chrono::steady_clock::now();

	//// test on int vector 

	//std::cout << "SVM on int vector: " << std::endl;
	//startTime = std::chrono::steady_clock::now();
	//metric::classification::edmClassifier<Record, CSVM> svmModel_1 = metric::classification::edmClassifier<Record, CSVM>();
	//std::cout << "training... " << std::endl;
	//svmModel_1.train(payments, features, response);
	//endTime = std::chrono::steady_clock::now();
	//std::cout << "trained (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000000 << " s)" << std::endl;

	//svmModel_1.predict(test_sample, features, prediction);
	//std::cout << "prediction: " << std::endl;
	//vector_print(prediction);

	std::cout << "\n";

	return 0;

}
