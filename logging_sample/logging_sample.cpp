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

#include "logging.h"

#include "converting.h"
#include "argument_parser.h"

#include "fmt/format.h"
#include "fmt/xchar.h"

#include <iostream>

constexpr auto PROGRAM_NAME = L"logging_sample";

using namespace logging;
using namespace converting;
using namespace argument_parser;

#ifdef _DEBUG
logging_level log_level = logging_level::parameter;
logging_styles logging_style = logging_styles::console_only;
#else
logging_level log_level = logging_level::information;
logging_styles logging_style = logging_styles::file_only;
#endif

void parse_bool(const wstring& key, argument_manager& arguments, bool& value);
bool parse_arguments(argument_manager& arguments);
void display_help(void);

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

	vector<thread> threads;
	for (unsigned short thread_index = 0; thread_index < 10; ++thread_index)
	{
		threads.push_back(
			thread([](const unsigned short& thread_index)
				{
					for (unsigned int log_index = 0; log_index < 1000; ++log_index)
					{
						logger::handle().write(logging_level::information, fmt::format(L"테스트_in_thread_{}: {}", thread_index, log_index));
					}
				}, thread_index)
		);
	}

	for (auto& thread : threads)
	{
		thread.join();
	}

	logger::handle().stop();

    return 0;
}

void parse_bool(const wstring& key, argument_manager& arguments, bool& value)
{
	auto target = arguments.get(key);
	if (target.empty())
	{
		return;
	}

	auto temp = target;
	transform(temp.begin(), temp.end(), temp.begin(), ::tolower);

	value = temp.compare(L"true") == 0;
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

	bool temp_condition = false;
	parse_bool(L"--write_console_only", arguments, temp_condition);
	if (temp_condition)
	{
		logging_style = logging_styles::console_only;
	}
	else
	{
		temp_condition = true;
		parse_bool(L"--write_console", arguments, temp_condition);
		if (temp_condition)
		{
			logging_style = logging_styles::file_and_console;
		}
		else
		{
			logging_style = logging_styles::file_only;
		}
	}

	target = arguments.get(L"--logging_level");
	if (!target.empty())
	{
		log_level = (logging_level)atoi(converter::to_string(target).c_str());
	}

	return true;
}

void display_help(void)
{
	wcout << L"logging sample options:" << endl << endl;
	wcout << L"--write_console [value] " << endl;
	wcout << L"\tThe write_console_mode on/off. If you want to display log on console must be appended '--write_console true'.\n\tInitialize value is --write_console off." << endl << endl;
	wcout << L"--logging_level [value]" << endl;
	wcout << L"\tIf you want to change log level must be appended '--logging_level [level]'." << endl;
}
