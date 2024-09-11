#pragma once

#include "httplib.h"
#include "../esys/esysControl.h"

namespace ems {
	class httpServer {
	private:

		std::string host;
		std::string port;
		std::string mount_dir;
		std::shared_mutex& mtx;
		bool log_operations;

		httplib::Server hvr;
		
		httpServer(const httpServer&) = delete;
		httpServer& operator=(const httpServer&) = delete;

		friend class esysControl;

		void bindApi();

		int run();

		int stop();

	public:
		
		httpServer(std::shared_mutex& mtx);

	};
} // namespace ems