#include "dbTools.h"

namespace ems {
	void dbTools::executeSQL(const std::string& sql) {
		std::unique_ptr<sql::Statement> stmt(con->createStatement());
		stmt->execute(sql);
	}

	// ��ʼ�����ӵķ���
	void dbTools::initConnection(const std::string& url, const std::string& user, const std::string& password, const std::string& schema) {
		try {
			std::unique_lock lock(mtx);
			sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
			con.reset(driver->connect(url, user, password));
			// ��� schema �Ƿ����
			std::unique_ptr<sql::Statement> stmt(con->createStatement());
			std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SHOW DATABASES LIKE '" + schema + "'"));

			if (!res->next()) {
				std::cout << "[dbTools]: Schema '" << schema << "' does not exist. Creating schema..." << std::endl;
				// ��ȡ��ִ�� SQL �ļ�
				std::ifstream file(build_file_location);
				if (!file.is_open()) {
					throw std::runtime_error("Failed to open SQL file: " + build_file_location + " Please make sure the create table file exist.");
				}

				std::stringstream sqlStream;
				sqlStream << file.rdbuf();  // ��ȡ�����ļ�����
				std::string sql = sqlStream.str();
				file.close();

				// ��SQL��";"�ָ�Ϊ�������
				std::istringstream sqlCommands(sql);
				std::string singleSQL;
				while (std::getline(sqlCommands, singleSQL, ';')) {
					singleSQL = trim(singleSQL); // ȥ��ǰ��Ŀհ��ַ�
					if (!singleSQL.empty()) {
						executeSQL(singleSQL + ";");  // ִ�е���SQL���
					}
				}

				std::cout << "[dbTools]: Schema created successfully." << std::endl;
			}

			con->setSchema(schema);
			std::cout << "[dbTools]: Connected to database successfully." << std::endl;
		}
		catch (const sql::SQLException& e) {
			std::cerr << "[dbTools]: SQLException during connection: " << e.what()
				<< ", MySQL Error Code: " << e.getErrorCode()
				<< ", SQLState: " << e.getSQLState() << std::endl;
			// throw;  // �׳��쳣�Ա��ϲ㴦��
		}
		catch (const std::exception& e) {
			std::cerr << "[dbTools]: Error: " << e.what() << std::endl;
			// ���������쳣
		}
	}

	// ȥ���ַ������˿հ��ַ��ĸ�������
	std::string dbTools::trim(const std::string& str) {
		size_t first = str.find_first_not_of(" \t\n\r");
		if (first == std::string::npos) return ""; // ȫ�ǿհ��ַ�
		size_t last = str.find_last_not_of(" \t\n\r");
		return str.substr(first, (last - first + 1));
	}

	// ���캯��ʵ��
	dbTools::dbTools()
	{
		// ��ȡ esysControl ʵ��
		esysControl& esys = esysControl::getInstance();

		// ʹ�� esysControl ��ȡ����
		url = esys.getConfig("db_url");
		user = esys.getConfig("db_user");
		password = esys.getConfig("db_password");
		schema = esys.getConfig("db_schema");
		build_file_location = esys.getConfig("db_build_file_location");
		log_operations = esys.getConfig("log_operations") == "false" ? false : true;

		// ��ʼ������
		initConnection(url, user, password, schema);
	}

	// ��ȡ��ṹ�ĺ���
	std::unordered_map<std::string, std::string> dbTools::getTableStructure(const std::string& table_name) {
		// ��黺�����Ƿ����б�ṹ��Ϣ
		if (table_structure_cache.find(table_name) != table_structure_cache.end()) {
			return table_structure_cache[table_name];
		}
		// ��ṹ��Ϣδ���棬��Ҫ��ѯ���ݿ�
		std::unordered_map<std::string, std::string> columns;
		try {
			std::shared_lock lock(mtx);
			std::string query = "SHOW COLUMNS FROM " + table_name;
			std::unique_ptr<sql::Statement> stmt(con->createStatement());
			std::unique_ptr<sql::ResultSet> res(stmt->executeQuery(query));

			while (res->next()) {
				// ��ȡÿһ�е����ƺ�����
				columns.insert(std::make_pair(res->getString("Field"), res->getString("Type")));
			}

			// ����ṹ��Ϣ��������
			table_structure_cache.insert(std::make_pair(table_name, columns));
		}
		catch (const sql::SQLException& e) {
			std::cerr << "[dbTools]: Error getting table structure: " << e.what() << std::endl;
		}


		return columns;
	}

	// �������ݵĺ���
	int dbTools::dbInsert(const std::string& table_name, const std::vector<std::unordered_map<std::string, std::string>>& data) {
		// ��ȡ����нṹ
		std::unordered_map<std::string, std::string> columns = getTableStructure(table_name);

		if (columns.empty()) {
			std::cerr << "[dbTools]: Error: Unable to get table structure for " << table_name << std::endl;
			return EXIT_FAILURE;
		}

		for (auto& row : data) {
			// ����SQL�������
			std::string query = "INSERT INTO " + table_name + " (";
			std::string placeholders = " VALUES (";
			std::vector<std::string> stream_datas;
			int colIndex = 0;
			for (const auto& col : row) {
				query += col.first;
				if (col.second == "NOW()") {
					placeholders += "NOW()";
				}
				else {
					placeholders += "?";
					stream_datas.push_back(col.second);
				}
				if (colIndex < row.size() - 1) {
					query += ", ";
					placeholders += ", ";
				}
				colIndex++;
			}

			query += ")" + placeholders + ")";

			try {
				std::unique_lock lock(mtx);
				// ׼�����
				std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(query));

				// �󶨲���
				for (size_t i = 0; i < stream_datas.size(); ++i) {
					pstmt->setString(static_cast<int>(i + 1), stream_datas[i]);
				}

				// ִ�в������
				pstmt->executeUpdate();

				if (log_operations) std::cout << "[dbTools]: Inserted data successfully into table " << table_name << "." << std::endl;
			}
			catch (const sql::SQLException& e) {
				std::cerr << "[dbTools]: Error inserting data: " << e.what() << std::endl;
				return EXIT_FAILURE;
			}
		}
		return EXIT_SUCCESS;
	}

	int dbTools::dbInsert(const std::string& table_name, const std::unordered_map<std::string, std::string>& data) {
		std::vector<std::unordered_map<std::string, std::string>> _data = { data };
		return dbInsert(table_name, _data);
	}

	// ��ȡ���ݵĺ���
	int dbTools::dbRead(const std::string& table_name, std::vector<std::unordered_map<std::string, std::string>>& data, unsigned int count_row) {
		// ��ȡ����нṹ
		std::unordered_map<std::string, std::string> columns = getTableStructure(table_name);

		if (columns.empty()) {
			std::cerr << "[dbTools]: Error: Unable to get table structure for " << table_name << std::endl;
			return EXIT_FAILURE;
		}

		try {
			// ȷ�����д�����Ϊ "eid" ���У������滻Ϊʵ����Ҫ���������
			if (columns.find("eid") == columns.end()) {
				std::cerr << "[dbTools]: Error: Table does not have 'eid' column for ordering." << std::endl;
				return EXIT_FAILURE;
			}

			// ������ѯ��䣬�� 'eid' �������򣬲����ƽ������
			std::string query = "SELECT * FROM " + table_name + " ORDER BY eid DESC";
			if (count_row > 0) {
				query += " LIMIT " + std::to_string(count_row);
			}
			{
				std::unique_lock lock(mtx);
				std::unique_ptr<sql::Statement> stmt(con->createStatement());
				std::unique_ptr<sql::ResultSet> res(stmt->executeQuery(query));

				while (res->next()) {
					std::unordered_map<std::string, std::string> row;
					for (const auto& column : columns) {
						row[column.first] = res->getString(column.first);
					}
					data.push_back(row);
				}
			}
			

			if (log_operations) std::cout << "[dbTools]: Read " << data.size() << " rows from table " << table_name << "." << std::endl;
		}
		catch (const sql::SQLException& e) {
			std::cerr << "[dbTools]: Error reading data: " << e.what() << std::endl;
			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
	}


	int dbTools::dbRead(const std::string& table_name, std::unordered_map<std::string, std::string>& data) {
		unsigned int count_row = 1;
		std::vector<std::unordered_map<std::string, std::string>> _data;
		int result = dbRead(table_name, _data, count_row);
		if (!_data.empty()) {
			data = _data[0]; // ����ȡ�ĵ�һ����¼��ֵ�� data
		}
		else {
			// ����û�����ݵ�����������Ҫ�Ļ�
			data.clear(); // ��� data ��ȷ����������Ч״̬
		}
		return result;
	}

	int dbTools::dbReadByClientIP(const std::string& table_name, const std::string& client_ip, std::vector<std::unordered_map<std::string, std::string>>& data, unsigned int count_row) {
		// ��ȡ����нṹ
		std::unordered_map<std::string, std::string> columns = getTableStructure(table_name);

		if (columns.empty()) {
			std::cerr << "[dbTools]: Error: Unable to get table structure for " << table_name << std::endl;
			return EXIT_FAILURE;
		}

		try {
			// ȷ�����д�����Ϊ "ClientIP" ���У������滻Ϊʵ����Ҫɸѡ������
			if (columns.find("clientIP") == columns.end()) {
				std::cerr << "[dbTools]: Error: Table does not have 'clientIP' column for filtering." << std::endl;
				return EXIT_FAILURE;
			}

			// ������ѯ��䣬�� 'eid' �������򣬲�ɸѡ 'ClientIP'
			std::string query = "SELECT * FROM " + table_name + " WHERE clientIP = '" + client_ip + "' ORDER BY eid DESC";
			if (count_row > 0) {
				query += " LIMIT " + std::to_string(count_row);
			}
			{
				std::unique_lock lock(mtx);
				std::unique_ptr<sql::Statement> stmt(con->createStatement());
				std::unique_ptr<sql::ResultSet> res(stmt->executeQuery(query));
				while (res->next()) {
					std::unordered_map<std::string, std::string> row;
					for (const auto& column : columns) {
						row[column.first] = res->getString(column.first);
					}
					data.push_back(row);
				}
			}
			if (log_operations) std::cout << "[dbTools]: Read " << data.size() << " rows for clientIP '" << client_ip << "' from table " << table_name << "." << std::endl;
		}
		catch (const sql::SQLException& e) {
			std::cerr << "[dbTools]: Error reading data: " << e.what() << std::endl;
			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
	}

	int dbTools::dbReadByClientIP(const std::string& table_name, const std::string& client_ip, std::unordered_map<std::string, std::string>& data) {
		unsigned int count_row = 1; // ��ȡһ����¼
		std::vector<std::unordered_map<std::string, std::string>> _data;
		int result = dbReadByClientIP(table_name, client_ip, _data, count_row);
		if (!_data.empty()) {
			data = _data[0]; // ����ȡ�ĵ�һ����¼��ֵ�� data
		}
		else {
			// ����û�����ݵ�����������Ҫ�Ļ�
			data.clear(); // ��� data ��ȷ����������Ч״̬
		}
		return result;
	}



	int dbTools::dbDistinctSelect(const std::string& table_name, const std::string& attribute, std::vector<std::string>& data)
	{
		// ��ȡ����нṹ
		std::unordered_map<std::string, std::string> columns = getTableStructure(table_name);

		if (columns.empty()) {
			std::cerr << "[dbTools]: Error: Unable to get table structure for " << table_name + "." << std::endl;
			return EXIT_FAILURE;
		}

		bool get_attribute = false;
		for (auto& it : columns) {
			if (attribute == it.first) {
				get_attribute = true;
				break;
			}
		}
		if (get_attribute) {
			try
			{
				std::shared_lock lock(mtx);
				std::string query = "SELECT DISTINCT " + attribute + " FROM " + table_name;

				std::unique_ptr<sql::Statement> stmt(con->createStatement());
				std::unique_ptr<sql::ResultSet> res(stmt->executeQuery(query));

				while (res->next()) {
					data.push_back(res->getString(attribute));
				}
			}
			catch (const sql::SQLException& e) {
				std::cerr << "[dbTools]: Error reading data: " << e.what() << std::endl;
				return EXIT_FAILURE;
			}
		}
		else {
			std::cerr << "[dbTools]: Error: Unable to find attribute " << attribute + "." << std::endl;
		}

		return EXIT_SUCCESS;
	}
}  // namespace ems
