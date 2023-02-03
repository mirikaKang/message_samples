/*****************************************************************************
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
#include <future>
#include <memory>

#include "job.h"
#include "logging.h"
#include "job_pool.h"
#include "converting.h"
#include "file_handler.h"
#include "argument_parser.h"
#include "messaging_client.h"

#include "container.h"
#include "values/string_value.h"
#include "values/container_value.h"

#include "fmt/xchar.h"
#include "fmt/format.h"

constexpr auto PROGRAM_NAME = L"echo_client";

using namespace std;
using namespace logging;
using namespace threads;
using namespace network;
using namespace container;
using namespace converting;
using namespace file_handler;
using namespace argument_parser;

bool encrypt_mode = false;
bool compress_mode = false;
bool binary_mode = false;
unsigned short compress_block_size = 1024;
#ifdef _DEBUG
logging_level log_level = logging_level::parameter;
logging_styles logging_style = logging_styles::console_only;
#else
logging_level log_level = logging_level::information;
logging_styles logging_style = logging_styles::file_only;
#endif
wstring connection_key = L"echo_network";
wstring server_ip = L"127.0.0.1";
unsigned short server_port = 9876;
unsigned short high_priority_count = 1;
unsigned short normal_priority_count = 2;
unsigned short low_priority_count = 3;

shared_ptr<thread_pool> _thread_pool = nullptr;

map<wstring, function<void(const vector<uint8_t>&)>> _registered_messages;

optional<promise<bool>> _promise_status;
future<bool> _future_status;
shared_ptr<messaging_client> _client = nullptr;

bool parse_arguments(argument_manager& arguments);
void display_help(void);

void create_client(void);
void create_thread_pool(void);
void send_echo_test_message(const wstring& target_id, const wstring& target_sub_id);
void connection(const wstring& target_id, const wstring& target_sub_id, const bool& condition);
void received_message(shared_ptr<container::value_container> container);
void received_binary_message(const wstring& source_id, const wstring& source_sub_id, 
	const wstring& target_id, const wstring& target_sub_id, const vector<uint8_t>& data);
void received_echo_test(const vector<uint8_t>& data);

int main(int argc, char* argv[])
{
	argument_manager arguments(argc, argv);
	if (!parse_arguments(arguments))
	{
		return 0;
	}

	logger::handle().set_write_console(logging_style);
	logger::handle().set_target_level(log_level);
#ifdef _WIN32
	logger::handle().start(PROGRAM_NAME, locale("ko_KR.UTF-8"));
#else
	logger::handle().start(PROGRAM_NAME);
#endif

	_registered_messages.insert({ L"echo_test", received_echo_test });

	create_thread_pool();

	create_client();

	_promise_status = { promise<bool>() };
	_future_status = _promise_status.value().get_future();

	_future_status.wait();
	_promise_status.reset();

	_thread_pool->stop();
	_thread_pool.reset();

	_client->stop();
	_client.reset();

	logger::handle().stop();

	return 0;
}

bool parse_arguments(argument_manager& arguments)
{
	wstring temp;

	auto string_target = arguments.to_string(L"--help");
	if (string_target != nullopt)
	{
		display_help();

		return false;
	}

	auto bool_target = arguments.to_bool(L"--encrypt_mode");
	if (bool_target != nullopt)
	{
		encrypt_mode = *bool_target;
	}

	bool_target = arguments.to_bool(L"--compress_mode");
	if (bool_target != nullopt)
	{
		compress_mode = *bool_target;
	}

	bool_target = arguments.to_bool(L"--binary_mode");
	if (bool_target != nullopt)
	{
		binary_mode = *bool_target;
	}

	auto ushort_target = arguments.to_ushort(L"--compress_block_size");
	if (ushort_target != nullopt)
	{
		compress_block_size = *ushort_target;
	}

	string_target = arguments.to_string(L"--connection_key");
	if (string_target != nullopt)
	{
		temp = converter::to_wstring(file::load(*string_target));
		if (!temp.empty())
		{
			connection_key = temp;
		}
	}

	ushort_target = arguments.to_ushort(L"--server_port");
	if (ushort_target != nullopt)
	{
		server_port = *ushort_target;
	}

	ushort_target = arguments.to_ushort(L"--high_priority_count");
	if (ushort_target != nullopt)
	{
		high_priority_count = *ushort_target;
	}

	ushort_target = arguments.to_ushort(L"--normal_priority_count");
	if (ushort_target != nullopt)
	{
		normal_priority_count = *ushort_target;
	}

	ushort_target = arguments.to_ushort(L"--low_priority_count");
	if (ushort_target != nullopt)
	{
		low_priority_count = *ushort_target;
	}
	
	auto int_target = arguments.to_int(L"--logging_level");
	if (int_target != nullopt)
	{
		log_level = (logging_level)*int_target;
	}
	
	bool_target = arguments.to_bool(L"--write_console_only");
	if (bool_target != nullopt && *bool_target)
	{
		logging_style = logging_styles::console_only;

		return true;
	}

	bool_target = arguments.to_bool(L"--write_console");
	if (bool_target != nullopt && *bool_target)
	{
		logging_style = logging_styles::file_and_console;

		return true;
	}

	logging_style = logging_styles::file_only;

	return true;
}

void display_help(void)
{
	wcout << L"pathfinder connector options:" << endl << endl;
	wcout << L"--write_console [value] " << endl;
	wcout << L"\tThe write_console_mode on/off. If you want to display log on console must be appended '--write_console true'.\n\tInitialize value is --write_console off." << endl << endl;
	wcout << L"--logging_level [value]" << endl;
	wcout << L"\tIf you want to change log level must be appended '--logging_level [level]'." << endl;
}

void create_client(void)
{
	if (_client != nullptr)
	{
		_client.reset();
	}

	_client = make_shared<messaging_client>(PROGRAM_NAME);
	_client->set_encrypt_mode(encrypt_mode);
	_client->set_compress_mode(compress_mode);
	_client->set_compress_block_size(compress_block_size);
	_client->set_connection_key(connection_key);
	_client->set_connection_notification(&connection);
	if (binary_mode)
	{
		_client->set_binary_notification(&received_binary_message);
		_client->set_session_types({ session_types::binary_line });
	}
	else
	{
		_client->set_message_notification(&received_message);
		_client->set_session_types({ session_types::message_line });
	}
	_client->start(server_ip, server_port, high_priority_count, normal_priority_count, low_priority_count);
}

void create_thread_pool(void)
{
	if (_thread_pool != nullptr)
	{
		_thread_pool.reset();
	}

	_thread_pool = make_shared<thread_pool>();
	for (unsigned short high = 0; high < high_priority_count; ++high)
	{
		_thread_pool->append(make_shared<thread_worker>(priorities::high));
	}
	for (unsigned short normal = 0; normal < normal_priority_count; ++normal)
	{
		_thread_pool->append(make_shared<thread_worker>(priorities::normal, vector<priorities> { priorities::high }));
	}
	for (unsigned short low = 0; low < low_priority_count; ++low)
	{
		_thread_pool->append(make_shared<thread_worker>(priorities::low, vector<priorities> { priorities::high, priorities::normal }));
	}
	_thread_pool->start();
}

void send_echo_test_message(const wstring& target_id, const wstring& target_sub_id)
{
	if (binary_mode)
	{
		_client->send_binary(target_id, target_sub_id, converter::to_array(L"echo_test"));

		return;
	}

	shared_ptr<container::value_container> container =
		make_shared<container::value_container>(target_id, target_sub_id, L"echo_test", vector<shared_ptr<value>>{});

	_client->send(container);
}

void connection(const wstring& target_id, const wstring& target_sub_id, const bool& condition)
{
	logger::handle().write(logging_level::information,
		fmt::format(L"an echo_client({}[{}]) is {} an echo_server", target_id, target_sub_id,
			condition ? L"connected to" : L"disconnected from"));

	if (condition)
	{
		send_echo_test_message(target_id, target_sub_id);

		return;
	}

	if (_promise_status.has_value())
	{
		_promise_status.value().set_value(false);
	}
}

void received_message(shared_ptr<container::value_container> container)
{
	if (container == nullptr)
	{
		return;
	}

	auto message_type = _registered_messages.find(container->message_type());
	if (message_type != _registered_messages.end())
	{
		if (_thread_pool)
		{
			_thread_pool->push(make_shared<job>(priorities::high, 
				converter::to_array(container->serialize()), message_type->second));
		}

		return;
	}

	logger::handle().write(logging_level::sequence,
		fmt::format(L"unknown message: {}", container->serialize()));

	if (_promise_status.has_value())
	{
		_promise_status.value().set_value(false);
	}
}

void received_binary_message(const wstring& source_id, const wstring& source_sub_id, 
	const wstring& target_id, const wstring& target_sub_id, const vector<uint8_t>& data)
{
	if (data.empty())
	{
		if (_promise_status.has_value())
		{
			_promise_status.value().set_value(false);
		}

		return;
	}

	logger::handle().write(logging_level::sequence,
		fmt::format(L"received message: {}", converter::to_wstring(data)));

	if (_promise_status.has_value())
	{
		_promise_status.value().set_value(true);
	}
}

void received_echo_test(const vector<uint8_t>& data)
{
	if (data.empty())
	{
		if (_promise_status.has_value())
		{
			_promise_status.value().set_value(false);
		}

		return;
	}

	shared_ptr<container::value_container> container = make_shared<container::value_container>(data, false);
	if (container == nullptr)
	{
		if (_promise_status.has_value())
		{
			_promise_status.value().set_value(false);
		}

		return;
	}

	logger::handle().write(logging_level::sequence,
		fmt::format(L"received message: {}", container->message_type()));

	if (_promise_status.has_value())
	{
		_promise_status.value().set_value(true);
	}
}