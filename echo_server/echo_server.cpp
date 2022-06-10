﻿/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2021, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include <iostream>
#include <string>
#include <stdlib.h>

#include "logging.h"
#include "converting.h"
#include "file_handler.h"
#include "argument_parser.h"
#include "messaging_server.h"

#ifndef __USE_TYPE_CONTAINER__
#include "cpprest/json.h"
#else
#include "container.h"
#include "values/string_value.h"
#include "values/container_value.h"
#endif

#include "fmt/xchar.h"
#include "fmt/format.h"

#include <signal.h>

constexpr auto PROGRAM_NAME = L"echo_server";

using namespace std;
using namespace logging;
using namespace network;
using namespace converting;
using namespace file_handler;
using namespace argument_parser;

#ifdef _DEBUG
bool write_console = true;
#else
bool write_console = false;
#endif
bool encrypt_mode = false;
bool compress_mode = false;
unsigned short compress_block_size = 1024;
#ifdef _DEBUG
logging_level log_level = logging_level::parameter;
#else
logging_level log_level = logging_level::information;
#endif
wstring connection_key = L"echo_network";
unsigned short server_port = 9876;
unsigned short high_priority_count = 4;
unsigned short normal_priority_count = 4;
unsigned short low_priority_count = 4;
size_t session_limit_count = 0;

#ifndef __USE_TYPE_CONTAINER__
map<wstring, function<void(shared_ptr<json::value>)>> _registered_messages;
#else
map<wstring, function<void(shared_ptr<container::value_container>)>> _registered_messages;
#endif

shared_ptr<messaging_server> _server = nullptr;

void parse_bool(const wstring& key, argument_manager& arguments, bool& value);
void parse_ushort(const wstring& key, argument_manager& arguments, unsigned short& value);
void parse_ulong(const wstring& key, argument_manager& arguments, unsigned long& value);
bool parse_arguments(argument_manager& arguments);
void display_help(void);

void create_server(void);
void connection(const wstring& target_id, const wstring& target_sub_id, const bool& condition);
#ifndef __USE_TYPE_CONTAINER__
void received_message(shared_ptr<json::value> container);
void received_echo_test(shared_ptr<json::value> container);
#else
void received_message(shared_ptr<container::value_container> container);
void received_echo_test(shared_ptr<container::value_container> container);
#endif
void signal_callback(int signum);

int main(int argc, char* argv[])
{
	argument_manager arguments(argc, argv);
	if (!parse_arguments(arguments))
	{
		return 0;
	}

	signal(SIGINT, signal_callback);

	logger::handle().set_write_console(write_console);
	logger::handle().set_target_level(log_level);
#ifdef _WIN32
	logger::handle().start(PROGRAM_NAME, locale("ko_KR.UTF-8"));
#else
	logger::handle().start(PROGRAM_NAME);
#endif

	_registered_messages.insert({ L"echo_test", received_echo_test });

	create_server();

	_server->wait_stop();

	logger::handle().stop();

	return 0;
}

void parse_bool(const wstring& key, argument_manager& arguments, bool& value)
{
	auto target = arguments.get(key);
	if (!target.empty())
	{
		auto temp = target;
		transform(temp.begin(), temp.end(), temp.begin(), ::tolower);

		if (temp.compare(L"true") == 0)
		{
			value = true;
		}
		else
		{
			value = false;
		}
	}
}

void parse_ushort(const wstring& key, argument_manager& arguments, unsigned short& value)
{
	auto target = arguments.get(key);
	if (!target.empty())
	{
		value = (unsigned short)atoi(converter::to_string(target).c_str());
	}
}

void parse_ulong(const wstring& key, argument_manager& arguments, unsigned long& value)
{
	auto target = arguments.get(key);
	if (!target.empty())
	{
		value = (unsigned long)atol(converter::to_string(target).c_str());
	}
}

bool parse_arguments(argument_manager& arguments)
{
	wstring temp;

	auto target = arguments.get(L"--help");
	if (!target.empty())
	{
		display_help();

		return false;
	}

	parse_bool(L"--encrypt_mode", arguments, encrypt_mode);
	parse_bool(L"--compress_mode", arguments, compress_mode);
	parse_ushort(L"--compress_block_size", arguments, compress_block_size);

	target = arguments.get(L"--connection_key");
	if (!target.empty())
	{
		temp = converter::to_wstring(file::load(target));
		if (!temp.empty())
		{
			connection_key = temp;
		}
	}

	parse_ushort(L"--server_port", arguments, server_port);
	parse_ushort(L"--high_priority_count", arguments, high_priority_count);
	parse_ushort(L"--normal_priority_count", arguments, normal_priority_count);
	parse_ushort(L"--low_priority_count", arguments, low_priority_count);
	parse_ulong(L"--session_limit_count", arguments, session_limit_count);
	parse_bool(L"--write_console_mode", arguments, write_console);

	target = arguments.get(L"--logging_level");
	if (!target.empty())
	{
		log_level = (logging_level)atoi(converter::to_string(target).c_str());
	}

	return true;
}

void display_help(void)
{
	wcout << L"pathfinder options:" << endl << endl;
	wcout << L"--encrypt_mode [value] " << endl;
	wcout << L"\tThe encrypt_mode on/off. If you want to use encrypt mode must be appended '--encrypt_mode true'.\n\tInitialize value is --encrypt_mode off." << endl << endl;
	wcout << L"--compress_mode [value]" << endl;
	wcout << L"\tThe compress_mode on/off. If you want to use compress mode must be appended '--compress_mode true'.\n\tInitialize value is --compress_mode off." << endl << endl;
	wcout << L"--compress_block_size [value]" << endl;
	wcout << L"\tThe compress_mode on/off. If you want to change compress block size must be appended '--compress_block_size size'.\n\tInitialize value is --compress_mode 1024." << endl << endl;
	wcout << L"--connection_key [value]" << endl;
	wcout << L"\tIf you want to change a specific key string for the connection to the main server must be appended\n\t'--connection_key [specific key string]'." << endl << endl;
	wcout << L"--server_port [value]" << endl;
	wcout << L"\tIf you want to change a port number for the connection to the main server must be appended\n\t'--server_port [port number]'." << endl << endl;
	wcout << L"--high_priority_count [value]" << endl;
	wcout << L"\tIf you want to change high priority thread workers must be appended '--high_priority_count [count]'." << endl << endl;
	wcout << L"--normal_priority_count [value]" << endl;
	wcout << L"\tIf you want to change normal priority thread workers must be appended '--normal_priority_count [count]'." << endl << endl;
	wcout << L"--low_priority_count [value]" << endl;
	wcout << L"\tIf you want to change low priority thread workers must be appended '--low_priority_count [count]'." << endl << endl;
	wcout << L"--session_limit_count [value]" << endl;
	wcout << L"\tIf you want to change session limit count must be appended '--session_limit_count [count]'." << endl << endl;
	wcout << L"--write_console_mode [value] " << endl;
	wcout << L"\tThe write_console_mode on/off. If you want to display log on console must be appended '--write_console_mode true'.\n\tInitialize value is --write_console_mode off." << endl << endl;
	wcout << L"--logging_level [value]" << endl;
	wcout << L"\tIf you want to change log level must be appended '--logging_level [level]'." << endl;
}

void create_server(void)
{
	if (_server != nullptr)
	{
		_server.reset();
	}

	_server = make_shared<messaging_server>(PROGRAM_NAME);
	_server->set_encrypt_mode(encrypt_mode);
	_server->set_compress_mode(compress_mode);
	_server->set_connection_key(connection_key);
	_server->set_session_limit_count(session_limit_count);
	_server->set_possible_session_types({ session_types::message_line });
	_server->set_connection_notification(&connection);
	_server->set_message_notification(&received_message);
	_server->start(server_port, high_priority_count, normal_priority_count, low_priority_count);
}

void connection(const wstring& target_id, const wstring& target_sub_id, const bool& condition)
{
	logger::handle().write(logging_level::information,
		fmt::format(L"an echo_client({}[{}]) is {} to an echo_server", target_id, target_sub_id, 
			condition ? L"connected" : L"disconnected"));
}

#ifndef __USE_TYPE_CONTAINER__
void received_message(shared_ptr<json::value> container)
#else
void received_message(shared_ptr<container::value_container> container)
#endif
{
	if (container == nullptr)
	{
		return;
	}

#ifdef __USE_TYPE_CONTAINER__
	auto message_type = _registered_messages.find(container->message_type());
#else
#ifdef _WIN32
	auto message_type = _registered_messages.find((*container)[HEADER][MESSAGE_TYPE].as_string());
#else
	auto message_type = _registered_messages.find(converter::to_wstring((*container)[HEADER][MESSAGE_TYPE].as_string()));
#endif
#endif
	if (message_type != _registered_messages.end())
	{
		message_type->second(container);

		return;
	}

#ifdef __USE_TYPE_CONTAINER__
	logger::handle().write(logging_level::information,
		fmt::format(L"received message: {}", container->serialize()));
#else
#ifdef _WIN32
	logger::handle().write(logging_level::information,
		fmt::format(L"received message: {}", container->serialize()));
#else
	logger::handle().write(logging_level::information,
		converter::to_wstring(fmt::format("received message: {}", container->serialize())));
#endif
#endif
}

#ifndef __USE_TYPE_CONTAINER__
void received_echo_test(shared_ptr<json::value> container)
#else
void received_echo_test(shared_ptr<container::value_container> container)
#endif
{
	if (container == nullptr)
	{
		return;
	}

#ifdef __USE_TYPE_CONTAINER__
	logger::handle().write(logging_level::information, 
		fmt::format(L"received message: {}", container->serialize()));
#else
#ifdef _WIN32
	logger::handle().write(logging_level::information, 
		fmt::format(L"received message: {}", container->serialize()));
#else
	logger::handle().write(logging_level::information, 
		converter::to_wstring(fmt::format("received message: {}", container->serialize())));
#endif
#endif

#ifndef __USE_TYPE_CONTAINER__
	shared_ptr<json::value> message = make_shared<json::value>(json::value::object(true));
	(*message)[HEADER][SOURCE_ID] = (*container)[HEADER][TARGET_ID];
	(*message)[HEADER][SOURCE_SUB_ID] = (*container)[HEADER][TARGET_SUB_ID];
	(*message)[HEADER][TARGET_ID] = (*container)[HEADER][SOURCE_ID];
	(*message)[HEADER][TARGET_SUB_ID] = (*container)[HEADER][SOURCE_SUB_ID];
	(*message)[HEADER][MESSAGE_TYPE] = (*container)[HEADER][MESSAGE_TYPE];
#else
	shared_ptr<container::value_container> message = container->copy(false);
	message->swap_header();
#endif
	_server->send(message);
}

void signal_callback(int signum)
{
	_server->stop();
	
	logger::handle().stop();

	exit(signum);
}