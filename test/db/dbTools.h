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
		// MySQL���Ӷ���
		std::unique_ptr<sql::Connection> con;
		// ���ڻ����ṹ��Ϣ��ӳ��
		std::unordered_map<std::string, std::unordered_map<std::string, std::string>> table_structure_cache;

		std::string url;
		std::string user;
		std::string password;
		std::string schema;
		std::string build_dir;
		bool log_operations;
		std::shared_mutex mtx;

		// ˽�й��캯������ʼ�����ݿ����ӡ�
		dbTools();

		// ˽�е���������
		~dbTools() = default;

		// ɾ���������캯���͸�ֵ������
		dbTools(const dbTools&) = delete;
		dbTools& operator=(const dbTools&) = delete;

		void executeSQL(const std::string& sql);

		// ��ʼ�����ݿ�����
		void initConnection(const std::string& url, const std::string& user, const std::string& password, const std::string& schema);

		std::string trim(const std::string& str);

	public:
		/**
		 * @brief ��ȡ����ʵ��
		 *
		 * @return dbTools& ����ʵ��
		 */
		static dbTools& getInstance() {
			static dbTools instance;
			return instance;
		}

		/**
		 * @brief ��ȡָ����Ľṹ��Ϣ��
		 *
		 * @param table_name ������
		 * @return std::unordered_map<std::string, std::string> �������������ֵ���͵Ĺ�ϣ��
		 * @note �����ṹ�ѻ��棬��ֱ�Ӵӻ����л�ȡ��
		 */
		std::unordered_map<std::string, std::string> getTableStructure(const std::string& table_name);

		/**
		 * @brief ��ָ�����в������ݡ�
		 *
		 * @param table_name ������
		 * @param data Ҫ����������б�vector<unordered_map<string ����, string ֵ>>��
		 * @return int ����������ɹ����� EXIT_SUCCESS��ʧ�ܷ��� EXIT_FAILURE��
		 */
		int dbInsert(const std::string& table_name, const std::vector<std::unordered_map<std::string, std::string>>& data);

		int dbInsert(const std::string& table_name, const std::unordered_map<std::string, std::string>& data);

		/**
		 * @brief ��ָ�����ж�ȡ���ݡ�
		 *
		 * @param table_name ������
		 * @param data ���ڴ洢��ȡ���ݵ����ã�vector<map<string ����, string ֵ>>��
		 * @param count_row Ҫ��ȡ��������
		 * @return int ����������ɹ����� EXIT_SUCCESS��ʧ�ܷ��� EXIT_FAILURE��
		 */
		int dbRead(const std::string& table_name, std::vector<std::unordered_map<std::string, std::string>>& data, unsigned int count_row);

		int dbRead(const std::string& table_name, std::unordered_map<std::string, std::string>& data);

		int dbReadByClientIP(const std::string& table_name, const std::string& client_ip, std::vector<std::unordered_map<std::string, std::string>>& data, unsigned int count_row);

		int dbReadByClientIP(const std::string& table_name, const std::string& client_ip, std::unordered_map<std::string, std::string>& data);

		int dbDistinctSelect(const std::string& table_name, const std::string& attribute, std::vector<std::string>& data);
	};

}  // namespace ems
