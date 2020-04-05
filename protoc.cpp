#include <iostream>
#include <fstream>
#include <string>
#include "proto.hpp"

enum out_style {
	human = 0,
	cpp = 1
};

// (Text and binary are the same on non-Windows platforms.)
#if defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
#  include <io.h>
#  if defined(_MSC_VER) || defined(__MINGW32__)
#    include <fcntl.h>
#    ifndef _O_BINARY
#      define _O_BINARY O_BINARY
#    endif
#    ifndef _O_TEXT
#      define _O_TEXT O_TEXT
#    endif
#  else
#    include <sys/fcntl.h>
#  endif
#  define SET_STDIN_BINARY_MODE()                                                                                      \
    {                                                                                                                  \
      _setmode(0, _O_BINARY);                                                                                          \
    }
#  define SET_STDIN_TEXT_MODE()                                                                                        \
    {                                                                                                                  \
      _setmode(0, _O_TEXT);                                                                                            \
    }
#else
#  define SET_STDIN_BINARY_MODE()                                                                                      \
    {}
#  define SET_STDIN_TEXT_MODE()                                                                                        \
    {}
#endif

bool load_from_stdin(proto::message& msg, int dec_pack_depth = -1)
{
  SET_STDIN_BINARY_MODE();

  std::string data;
  {
    char            buf[8192] = {0};
    std::streamsize r;

    while (r = std::cin.rdbuf()->sgetn(buf, 8190), r > 0 && r <= 8190)
    {
      buf[r] = 0;
      if (data.capacity() < 10)
      {
        data.reserve(8192);
      }
      data.append(buf, std::size_t(r));
    }
  }

  SET_STDIN_TEXT_MODE();

  if (msg.deserialize(data, dec_pack_depth)) return true;

  std::cin.rdbuf()->sputn(data.data(), data.size());
  return false;
}

/**
 * \brief decode protobuf read from file
 * \param file
 */
bool load_from_file(proto::message& msg, std::istream& file, int dec_pack_depth = -1)
{
  if (file.fail()) return false;

  std::string data;

  {
    char            buf[8192] = {0};
    std::streamsize r;

    while (r = file.rdbuf()->sgetn(buf, 8190), r > 0 && r <= 8190)
    {
      buf[r] = 0;
      if (data.capacity() < 10)
      {
        data.reserve(8192);
      }
      data.append(buf, std::size_t(r));
    }
  }

  if (msg.deserialize(data, dec_pack_depth)) return true;

  file.rdbuf()->sputn(data.data(), data.size());
  return false;
}

void print_help()
{
	std::cout << 
				"protobuf decode\n"
				"protoc [option] <file|stdin>\n"
				"-h, --help    show this help\n"
				"-v, --version show version\n"
				"-d, --depth   set decode depth\n"
				"-f, --force   force output until error\n"
				"-s, --style   set output style(human, cpp)\n"
				"--decode_raw  use stdin input\n\n";
}

int main(int argc, char *argv[])
{
	bool opt_from_file = true;
	bool opt_force = false;
	int opt_depth = 2;
	out_style opt_style = human;
	std::string file;

	if (argc == 1)
	{
		print_help();
		return 0;
	}

	for (auto i = 1; i < argc; ++i)
	{
		std::string arg(argv[i]);
		if ("-h" == arg || "--help" == arg)
		{
			print_help();
			return 0;
		}
		if ("-v" == arg || "--version" == arg)
		{
			std::cout << "libprotoc 9.9.9 diy version" << std::endl;
			return 0;
		}
		else if ("-d" == arg || "-depth" == arg)
		{
			++i;
			opt_depth = std::atoi(argv[i]);
		}
		else if ("-f" == arg || "-force" == arg)
		{
			opt_force = true;
		}
		else if ("-s" == arg || "-style" == arg)
		{
			++i;
			std::string style(argv[i]);
			if ("cpp" == style || "1" == style)
				opt_style = cpp;
			else
				opt_style = human;
		}
		else if ("--decode_raw" == arg)
		{
			opt_from_file = false;
		}
		else
		{
			file = arg;
		}
	}

	proto::message msg;
	bool success = false;
	if (opt_from_file && !file.empty())
	{
		std::ifstream infile(file, std::ios::binary | std::ios::in);
		if (infile.is_open())
		{
			success = load_from_file(msg, infile, opt_depth);
		}
	}
	else
	{
		load_from_stdin(msg, opt_depth);
	}

	if (opt_force)
	{
		switch (opt_style)
		{
		case cpp:
			std::cout << msg.to_cpp_code();
			break;
		default:
			std::cout << msg.to_string();
			break;
		}
	}

	if (!success)
	{
		std::cout << "// decode fail" << std::endl;
		return -1;
	}
	return 0;
}
