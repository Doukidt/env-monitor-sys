#include "dbTools.h"

namespace ems {
	void dbTools::executeSQL(const std::string& sql) {
		std::unique_ptr<sql::Statement> stmt(con->createStatement());
		stmt->execute(sql);
	}

	// 初始化连接的方法
	void dbTools::initConnection(const std::string& url, const std::string& user, const std::string& password, const std::string& schema) {
		try {
			std::unique_lock lock(mtx);
			sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
			con.reset(driver->connect(url, user, password));
			// 检查 schema 是否存在
			std::unique_ptr<sql::Statement> stmt(con->createStatement());
			std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SHOW DATABASES LIKE '" + schema + "'"));

			if (!res->next()) {
				std::cout << "[dbTools]: Schema '" << schema << "' does not exist. Creating schema..." << std::endl;
				// 读取并执行 SQL 文件
				std::ifstream file(build_file_location);
				if (!file.is_open()) {
					throw std::runtime_error("Failed to open SQL file: " + build_file_location + " Please make sure the create table file exist.");
				}

				std::stringstream sqlStream;
				sqlStream << file.rdbuf();  // 读取整个文件内容
				std::string sql = sqlStream.str();
				file.close();

				// 将SQL按";"分割为多条语句
				std::istringstream sqlCommands(sql);
				std::string singleSQL;
				while (std::getline(sqlCommands, singleSQL, ';')) {
					singleSQL = trim(singleSQL); // 去除前后的空白字符
					if (!singleSQL.empty()) {
						executeSQL(singleSQL + ";");  // 执行单条SQL语句
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
			// throw;  // 抛出异常以便上层处理
		}
		catch (const std::exception& e) {
			std::cerr << "[dbTools]: Error: " << e.what() << std::endl;
			// 处理其他异常
		}
	}

	// 去除字符串两端空白字符的辅助函数
	std::string dbTools::trim(const std::string& str) {
		size_t first = str.find_first_not_of(" \t\n\r");
		if (first == std::string::npos) return ""; // 全是空白字符
		size_t last = str.find_last_not_of(" \t\n\r");
		return str.substr(first, (last - first + 1));
	}

	// 构造函数实现
	dbTools::dbTools()
	{
		// 获取 esysControl 实例
		esysControl& esys = esysControl::getInstance();

		// 使用 esysControl 获取配置
		url = esys.getConfig("db_url");
		user = esys.getConfig("db_user");
		password = esys.getConfig("db_password");
		schema = esys.getConfig("db_schema");
		build_file_location = esys.getConfig("db_build_file_location");
		log_operations = esys.getConfig("log_operations") == "false" ? false : true;

		// 初始化连接
		initConnection(url, user, password, schema);
	}

	// 获取表结构的函数
	std::unordered_map<std::string, std::string> dbTools::getTableStructure(const std::string& table_name) {
		// 检查缓存中是否已有表结构信息
		if (table_structure_cache.find(table_name) != table_structure_cache.end()) {
			return table_structure_cache[table_name];
		}
		// 表结构信息未缓存，需要查询数据库
		std::unordered_map<std::string, std::string> columns;
		try {
			std::shared_lock lock(mtx);
			std::string query = "SHOW COLUMNS FROM " + table_name;
			std::unique_ptr<sql::Statement> stmt(con->createStatement());
			std::unique_ptr<sql::ResultSet> res(stmt->executeQuery(query));

			while (res->next()) {
				// 获取每一列的名称和类型
				columns.insert(std::make_pair(res->getString("Field"), res->getString("Type")));
			}

			// 将表结构信息缓存起来
			table_structure_cache.insert(std::make_pair(table_name, columns));
		}
		catch (const sql::SQLException& e) {
			std::cerr << "[dbTools]: Error getting table structure: " << e.what() << std::endl;
		}


		return columns;
	}

	// 插入数据的函数
	int dbTools::dbInsert(const std::string& table_name, const std::vector<std::unordered_map<std::string, std::string>>& data) {
		// 获取表的列结构
		std::unordered_map<std::string, std::string> columns = getTableStructure(table_name);

		if (columns.empty()) {
			std::cerr << "[dbTools]: Error: Unable to get table structure for " << table_name << std::endl;
			return EXIT_FAILURE;
		}

		for (auto& row : data) {
			// 构建SQL插入语句
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
				// 准备语句
				std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(query));

				// 绑定参数
				for (size_t i = 0; i < stream_datas.size(); ++i) {
					pstmt->setString(static_cast<int>(i + 1), stream_datas[i]);
				}

				// 执行插入操作
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

	// 读取数据的函数
	int dbTools::dbRead(const std::string& table_name, std::vector<std::unordered_map<std::string, std::string>>& data, unsigned int count_row) {
		// 获取表的列结构
		std::unordered_map<std::string, std::string> columns = getTableStructure(table_name);

		if (columns.empty()) {
			std::cerr << "[dbTools]: Error: Unable to get table structure for " << table_name << std::endl;
			return EXIT_FAILURE;
		}

		try {
			// 确保表中存在名为 "eid" 的列，或者替换为实际需要排序的列名
			if (columns.find("eid") == columns.end()) {
				std::cerr << "[dbTools]: Error: Table does not have 'eid' column for ordering." << std::endl;
				return EXIT_FAILURE;
			}

			// 构建查询语句，按 'eid' 降序排序，并限制结果行数
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
			data = _data[0]; // 将读取的第一条记录赋值给 data
		}
		else {
			// 处理没有数据的情况，如果需要的话
			data.clear(); // 清空 data 以确保它处于有效状态
		}
		return result;
	}

	int dbTools::dbReadByClientIP(const std::string& table_name, const std::string& client_ip, std::vector<std::unordered_map<std::string, std::string>>& data, unsigned int count_row) {
		// 获取表的列结构
		std::unordered_map<std::string, std::string> columns = getTableStructure(table_name);

		if (columns.empty()) {
			std::cerr << "[dbTools]: Error: Unable to get table structure for " << table_name << std::endl;
			return EXIT_FAILURE;
		}

		try {
			// 确保表中存在名为 "ClientIP" 的列，或者替换为实际需要筛选的列名
			if (columns.find("clientIP") == columns.end()) {
				std::cerr << "[dbTools]: Error: Table does not have 'clientIP' column for filtering." << std::endl;
				return EXIT_FAILURE;
			}

			// 构建查询语句，按 'eid' 降序排序，并筛选 'ClientIP'
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
		unsigned int count_row = 1; // 读取一条记录
		std::vector<std::unordered_map<std::string, std::string>> _data;
		int result = dbReadByClientIP(table_name, client_ip, _data, count_row);
		if (!_data.empty()) {
			data = _data[0]; // 将读取的第一条记录赋值给 data
		}
		else {
			// 处理没有数据的情况，如果需要的话
			data.clear(); // 清空 data 以确保它处于有效状态
		}
		return result;
	}



	int dbTools::dbDistinctSelect(const std::string& table_name, const std::string& attribute, std::vector<std::string>& data)
	{
		// 获取表的列结构
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
