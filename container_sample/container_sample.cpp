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

#ifdef __USE_TYPE_CONTAINER__
#include "logging.h"
#include "argument_parser.h"

#include "container.h"
#include "converting.h"
#include "values/bool_value.h"
#include "values/float_value.h"
#include "values/double_value.h"
#include "values/long_value.h"
#include "values/ulong_value.h"
#include "values/llong_value.h"
#include "values/ullong_value.h"
#include "values/container_value.h"

#include "fmt/xchar.h"
#include "fmt/format.h"

#include <limits.h>
#include <memory>
#include <iostream>

constexpr auto PROGRAM_NAME = L"container_sample";

using namespace logging;
using namespace container;
using namespace converting;
using namespace argument_parser;

bool write_console = false;
#ifdef _DEBUG
logging_level log_level = logging_level::parameter;
#else
logging_level log_level = logging_level::information;
#endif

bool parse_arguments(argument_manager& arguments);
void display_help(void);

int main(int argc, char* argv[])
{
	argument_manager arguments(argc, argv);
	if (!parse_arguments(arguments))
	{
		return 0;
	}

	logger::handle().set_write_console(write_console);
	logger::handle().set_target_level(log_level);
	logger::handle().start(PROGRAM_NAME);

	auto start = logger::handle().chrono_start();
	value_container data;
	data.add(bool_value(L"false_value", false));
	data.add(bool_value(L"true_value", true));
	data.add(float_value(L"float_value", (float)1.234567890123456789));
	data.add(double_value(L"double_value", (double)1.234567890123456789));
	logger::handle().write(logging_level::information, fmt::format(L"data serialize:\n{}", data.serialize()), start);
	logger::handle().write(logging_level::information, fmt::format(L"data xml:\n{}", data.to_xml()), start);
	logger::handle().write(logging_level::information, fmt::format(L"data json:\n{}", data.to_json()), start);

	start = logger::handle().chrono_start();
	value_container data2(data);
	data2.add(make_shared<long_value>(L"long_value", LONG_MAX));
	data2.add(make_shared<ulong_value>(L"ulong_value", ULONG_MAX));
	data2.add(make_shared<llong_value>(L"llong_value", LLONG_MAX));
	data2.add(make_shared<ullong_value>(L"ullong_value", ULLONG_MAX));
	data2.add(make_shared<container_value>(L"container_value", vector<shared_ptr<value>>
		{
			make_shared<long_value>(L"long_value", LONG_MAX),
			make_shared<ulong_value>(L"ulong_value", ULONG_MAX),
			make_shared<llong_value>(L"llong_value", LLONG_MAX),
			make_shared<ullong_value>(L"ullong_value", ULLONG_MAX)
		}));
	logger::handle().write(logging_level::information, fmt::format(L"data serialize:\n{}", data2.serialize()), start);
	logger::handle().write(logging_level::information, fmt::format(L"data xml:\n{}", data2.to_xml()), start);
	logger::handle().write(logging_level::information, fmt::format(L"data json:\n{}", data2.to_json()), start);

	start = logger::handle().chrono_start();
	value_container data3(data2);
	data3.remove(L"false_value");
	data3.remove(L"true_value");
	data3.remove(L"float_value");
	data3.remove(L"double_value");
	data3.remove(L"container_value");
	logger::handle().write(logging_level::information, fmt::format(L"data serialize:\n{}", data3.serialize()), start);
	logger::handle().write(logging_level::information, fmt::format(L"data xml:\n{}", data3.to_xml()), start);
	logger::handle().write(logging_level::information, fmt::format(L"data json:\n{}", data3.to_json()), start);

	logger::handle().stop();

    return 0;
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

	target = arguments.get(L"--write_console_mode");
	if (!target.empty())
	{
		temp = target;
		transform(temp.begin(), temp.end(), temp.begin(), ::tolower);

		if (temp.compare(L"true") == 0)
		{
			write_console = true;
		}
		else
		{
			write_console = false;
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
	wcout << L"container sample options:" << endl << endl;
	wcout << L"--write_console_mode [value] " << endl;
	wcout << L"\tThe write_console_mode on/off. If you want to display log on console must be appended '--write_console_mode true'.\n\tInitialize value is --write_console_mode off." << endl << endl;
	wcout << L"--logging_level [value]" << endl;
	wcout << L"\tIf you want to change log level must be appended '--logging_level [level]'." << endl;
}
#else
int main(int argc, char* argv[])
{
	return 0;
}
#endif