#include "httpServer.h"

namespace ems {
	httpServer::httpServer(std::shared_mutex& mtx) : mtx(std::ref(mtx))
	{
		esysControl& esys = esysControl::getInstance();
		host = esys.getConfig("hs_host");
		port = esys.getConfig("hs_port");
		mount_dir = esys.getConfig("hs_mount_dir");
		log_operations = esys.getConfig("log_operations") == "false" ? false : true;
	}
	void httpServer::bindApi()
	{
		using namespace httplib;
		hvr.Get(R"(/api/(.*))", [&](const Request& req, Response& res) {
			std::string url = req.path;
			std::string api = "";
			std::string prefix = "/api/";
			size_t pos = url.find(prefix);
			if (pos != std::string::npos) {
				api = url.substr(pos + prefix.length());
			}
			dbTools& db = dbTools::getInstance();
			std::string http_status_code = "200";
			std::stringstream ss;
			ss << "{ \"code\" : " + http_status_code + ", " << "\"data\" : ";
			if (api == "clientip") {
				std::vector<std::string> all_client_ip;
				db.dbDistinctSelect("envtable", "clientIP", all_client_ip);
				ss << "[";
				for (size_t i = 0; i < all_client_ip.size(); ++i) {
					ss << "\"" + all_client_ip[i] + "\"";
					if (i != all_client_ip.size() - 1) {
						ss << ", ";
					}
				}
				ss << "]";
			}
			else if (api == "record") {
				std::unordered_map<std::string, std::string> data;
				std::unordered_map<std::string, std::string> table_structure;
				db.dbReadByClientIP("envtable", req.get_param_value("ip"), data);
				table_structure = db.getTableStructure("envtable");
				ss << "{";
				auto it = data.begin();
				while (it != data.end()) {
					ss << "\"" + it->first + "\": " +"\"" + it->second + "\"";
					++it;
					if (it != data.end()) {
						ss << ", ";
					}
				}
				ss << "}";
			}
			else if (api == "alarm") {
				std::map<std::string, std::string> message = alarmModule::getInstance().getAlarmMessage();
				ss << "{ ";
				auto it = message.begin();
				while (it != message.end()) {
					ss << "\"ip\": \"" << it->first << "\", \"message\": \"" << it->second << "\"";
					++it;
					if (it != message.end()) {
						ss << ", ";
					}
				}
				ss << " }";
			}
			else {
				ss << "'Invaild api'";
			}
			ss << "}";
			{
				std::unique_lock lock(mtx);
				if (log_operations) {
					std::cout << "[httpServer]: HTTP GET Request from \"" + req.get_header_value("Host") + url + "\"." << std::endl;
					std::cout << "[httpServer]: HTTP GET Response : \"" + ss.str() + "\"." << std::endl;
				}
			}
			res.set_content(ss.str(), "application/json");
			});
	}

	int httpServer::run()
	{
		if (port == "") {
			std::stringstream ss;
			ss << hvr.bind_to_any_port(host);
			port = ss.str();
		}
		bindApi();
		hvr.set_mount_point("/", mount_dir);
		bool rt = hvr.bind_to_port(host, stoi(port));
		if (rt) {
			std::unique_lock lock(mtx);
			std::cout << "[httpServer]: httpServer is bind to " << host + ":" + port + ", start successfully." << std::endl;
		}
		else {
			std::unique_lock lock(mtx);
			std::cout << "[httpServer]: Error: bind to port " + port + " failed, start failed." << std::endl;
		}	
		hvr.listen_after_bind();
		//{
		//	std::unique_lock lock(mtx);
		//	std::cout << "[httpServer]: test message." << std::endl;
		//}
		return 0;
	}

	int httpServer::stop()
	{
		hvr.stop();
		return 0;
	}
} // namespace ems