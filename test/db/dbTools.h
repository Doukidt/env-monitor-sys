#pragma once

#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/exception.h>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "../esys/esysControl.h"

namespace ems {

	class dbTools {
	private:
		// MySQL连接对象
		std::unique_ptr<sql::Connection> con;
		// 用于缓存表结构信息的映射
		std::unordered_map<std::string, std::unordered_map<std::string, std::string>> table_structure_cache;

		std::string url;
		std::string user;
		std::string password;
		std::string schema;
		std::string build_dir;
		bool log_operations;
		std::shared_mutex mtx;

		// 私有构造函数，初始化数据库连接。
		dbTools();

		// 私有的析构函数
		~dbTools() = default;

		// 删除拷贝构造函数和赋值操作符
		dbTools(const dbTools&) = delete;
		dbTools& operator=(const dbTools&) = delete;

		void executeSQL(const std::string& sql);

		// 初始化数据库连接
		void initConnection(const std::string& url, const std::string& user, const std::string& password, const std::string& schema);

		std::string trim(const std::string& str);

	public:
		/**
		 * @brief 获取单例实例
		 *
		 * @return dbTools& 单例实例
		 */
		static dbTools& getInstance() {
			static dbTools instance;
			return instance;
		}

		/**
		 * @brief 获取指定表的结构信息。
		 *
		 * @param table_name 表名。
		 * @return std::unordered_map<std::string, std::string> 表的列名、该列值类型的哈希表。
		 * @note 如果表结构已缓存，则直接从缓存中获取。
		 */
		std::unordered_map<std::string, std::string> getTableStructure(const std::string& table_name);

		/**
		 * @brief 向指定表中插入数据。
		 *
		 * @param table_name 表名。
		 * @param data 要插入的数据列表，vector<unordered_map<string 列名, string 值>>。
		 * @return int 操作结果，成功返回 EXIT_SUCCESS，失败返回 EXIT_FAILURE。
		 */
		int dbInsert(const std::string& table_name, const std::vector<std::unordered_map<std::string, std::string>>& data);

		int dbInsert(const std::string& table_name, const std::unordered_map<std::string, std::string>& data);

		/**
		 * @brief 从指定表中读取数据。
		 *
		 * @param table_name 表名。
		 * @param data 用于存储读取数据的引用，vector<map<string 列名, string 值>>。
		 * @param count_row 要读取的行数。
		 * @return int 操作结果，成功返回 EXIT_SUCCESS，失败返回 EXIT_FAILURE。
		 */
		int dbRead(const std::string& table_name, std::vector<std::unordered_map<std::string, std::string>>& data, unsigned int count_row);

		int dbRead(const std::string& table_name, std::unordered_map<std::string, std::string>& data);

		int dbReadByClientIP(const std::string& table_name, const std::string& client_ip, std::vector<std::unordered_map<std::string, std::string>>& data, unsigned int count_row);

		int dbReadByClientIP(const std::string& table_name, const std::string& client_ip, std::unordered_map<std::string, std::string>& data);

		int dbDistinctSelect(const std::string& table_name, const std::string& attribute, std::vector<std::string>& data);
	};

}  // namespace ems
