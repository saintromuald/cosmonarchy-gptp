#pragma once
#include "BW\game_runner.h"
#include "BW\bwdata.h"
#include "snp.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <atomic>

#include <json.hpp>
using Json = nlohmann::json;

class FraudLauncherClient {
public:
	~FraudLauncherClient() { disconnect(); };

	bool connect();
	void disconnect();
	bool send(std::string& message);
	bool handle_messages();
	bool connected() const { return m_connected; };

private:
	HANDLE m_pipe_to_server = INVALID_HANDLE_VALUE;
	HANDLE m_pipe_from_server = INVALID_HANDLE_VALUE;
	std::atomic<bool> m_connected = false;
	std::atomic<bool> m_running = false;
	const char* m_pipe_to_server_name = "\\\\.\\pipe\\fraud_launcher_c2s";
	const char* m_pipe_from_server_name = "\\\\.\\pipe\\fraud_launcher_s2c";
};
// These are temporary
void example_create_singleplayer();
void example_create_lobby();
void example_join_lobby();


inline FraudLauncherClient fraud_launcher{};