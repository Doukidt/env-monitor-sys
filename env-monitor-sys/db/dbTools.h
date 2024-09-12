/**
 * @file dbTools.h
 * @author Yilin Wang (yilin233@foxmail.com)
 * @brief A tool class used to handle database operations, supporting database 
 *  connection, insertion, read, query, and other operations.
 * @version 1.0
 * @date 2024-09-11
 *
 * @copyright Copyright (c) 2024 Yilin Wang
 *
 * MIT License
 */


#pragma once

// 包含MySQL连接和操作的相关头文件
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
#include "../esys/esysControl.h"  // 其他自定义依赖项

namespace ems {  // namespace ems start

	/**
	 * @class dbTools
	 * @brief 用于处理数据库操作的工具类，支持数据库连接、插入、读取、查询等操作。
	 */
	class dbTools {
	private:
		// MySQL连接对象
		std::unique_ptr<sql::Connection> con;
		// 用于缓存表结构信息的映射
		std::unordered_map<std::string, std::unordered_map<std::string, std::string>> table_structure_cache;

		std::string url;				// 数据库URL
		std::string user;				// 数据库用户名
		std::string password;			// 数据库密码
		std::string schema;				// 使用的数据库schema
		std::string build_file_location;// 构建数据库文件位置
		bool log_operations;			// 是否记录操作日志
		std::shared_mutex mtx;			// 共享互斥锁，用于同步访问

		/**
		 * @brief 私有构造函数，初始化数据库连接。
		 */
		dbTools();

		/**
		 * @brief 私有析构函数。
		 */
		~dbTools() = default;

		/**
		 * @brief 删除拷贝构造函数。
		 */
		dbTools(const dbTools&) = delete;

		/**
		 * @brief 删除赋值操作符。
		 */
		dbTools& operator=(const dbTools&) = delete;

		/**
		 * @brief 执行SQL语句。
		 * @param sql 要执行的SQL语句字符串。
		 */
		void executeSQL(const std::string& sql);

		/**
		 * @brief 初始化数据库连接。
		 * @param url 数据库URL。
		 * @param user 数据库用户名。
		 * @param password 数据库密码。
		 * @param schema 使用的数据库schema。
		 */
		void initConnection(const std::string& url, const std::string& user, const std::string& password, const std::string& schema);

		/**
		 * @brief 去除字符串两端的空格。
		 * @param str 要去除空格的字符串。
		 * @return std::string 去除空格后的字符串。
		 */
		std::string trim(const std::string& str);

	public:
		/**
		 * @brief 获取dbTools类的单例实例。
		 *
		 * @return dbTools& 单例实例的引用。
		 */
		static dbTools& getInstance() {
			static dbTools instance;
			return instance;
		}

		/**
		 * @brief 获取指定表的结构信息。
		 *
		 * @param table_name 表名。
		 * @return std::unordered_map<std::string, std::string> 表的列名和该列类型的哈希表。
		 * @note 如果表结构已缓存，则直接从缓存中获取。
		 */
		std::unordered_map<std::string, std::string> getTableStructure(const std::string& table_name);

		/**
		 * @brief 向指定表中插入多行数据。
		 *
		 * @param table_name 表名。
		 * @param data 要插入的数据列表，vector<unordered_map<string 列名, string 值>>。
		 * @return int 操作结果，成功返回 EXIT_SUCCESS，失败返回 EXIT_FAILURE。
		 */
		int dbInsert(const std::string& table_name, const std::vector<std::unordered_map<std::string, std::string>>& data);

		/**
		 * @brief 向指定表中插入一行数据。
		 *
		 * @param table_name 表名。
		 * @param data 要插入的数据，unordered_map<string 列名, string 值>。
		 * @return int 操作结果，成功返回 EXIT_SUCCESS，失败返回 EXIT_FAILURE。
		 */
		int dbInsert(const std::string& table_name, const std::unordered_map<std::string, std::string>& data);

		/**
		 * @brief 从指定表中读取多行数据。
		 *
		 * @param table_name 表名。
		 * @param data 用于存储读取数据的引用，vector<map<string 列名, string 值>>。
		 * @param count_row 要读取的行数。
		 * @return int 操作结果，成功返回 EXIT_SUCCESS，失败返回 EXIT_FAILURE。
		 */
		int dbRead(const std::string& table_name, std::vector<std::unordered_map<std::string, std::string>>& data, unsigned int count_row);

		/**
		 * @brief 从指定表中读取一行数据。
		 *
		 * @param table_name 表名。
		 * @param data 用于存储读取数据的引用，unordered_map<string 列名, string 值>。
		 * @return int 操作结果，成功返回 EXIT_SUCCESS，失败返回 EXIT_FAILURE。
		 */
		int dbRead(const std::string& table_name, std::unordered_map<std::string, std::string>& data);

		/**
		 * @brief 根据客户端IP从指定表中读取多行数据。
		 *
		 * @param table_name 表名。
		 * @param client_ip 客户端IP。
		 * @param data 用于存储读取数据的引用，vector<map<string 列名, string 值>>。
		 * @param count_row 要读取的行数。
		 * @return int 操作结果，成功返回 EXIT_SUCCESS，失败返回 EXIT_FAILURE。
		 */
		int dbReadByClientIP(const std::string& table_name, const std::string& client_ip, std::vector<std::unordered_map<std::string, std::string>>& data, unsigned int count_row);

		/**
		 * @brief 根据客户端IP从指定表中读取一行数据。
		 *
		 * @param table_name 表名。
		 * @param client_ip 客户端IP。
		 * @param data 用于存储读取数据的引用，unordered_map<string 列名, string 值>。
		 * @return int 操作结果，成功返回 EXIT_SUCCESS，失败返回 EXIT_FAILURE。
		 */
		int dbReadByClientIP(const std::string& table_name, const std::string& client_ip, std::unordered_map<std::string, std::string>& data);

		/**
		 * @brief 从指定表中查询属性的唯一值。
		 *
		 * @param table_name 表名。
		 * @param attribute 要查询的属性。
		 * @param data 用于存储唯一值的vector。
		 * @return int 操作结果，成功返回 EXIT_SUCCESS，失败返回 EXIT_FAILURE。
		 */
		int dbDistinctSelect(const std::string& table_name, const std::string& attribute, std::vector<std::string>& data);
	};

}  // namespace ems end
