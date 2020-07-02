#include <iostream>
#include <fstream>
#include <string>
#include "proto.hpp"

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

enum out_style {
	human = 0,
	cpp = 1
};


char byte_to_hex(unsigned char b, bool lower = true)
{
  static const char* hex_map[2] = {"0123456789ABCDEF", "0123456789abcdef"};

  return hex_map[int(lower)][int(b)];
}

/**
 * \brief convert un-printable character to escape sequences
 */
std::string to_readable_string(const void* data, size_t len)
{
  using byte_p_t = unsigned char*;

  std::string _result;
  _result.reserve(len * 3);

  for (auto i = 0; i < int(len); i++)
  {
    auto c = (byte_p_t(data))[i];
    if (c > 31 && c < 127 && c != '\"' && c != '\'')
    {
      _result += c;
    }
    else
    {
      // write "\xhh"
      _result += "\\x";
      _result += byte_to_hex((c >> 4) & 0x0f);
      _result += byte_to_hex(c & 0x0f);
    }
  }
  return _result;
}


/**
 * \brief get human readble string of data struct view
 */
void to_string(std::string& result, const proto::message& msg, int indent, int leftspace, int depth, int show_type,
  bool show_size)
{
  auto _cur_leftspace = leftspace + indent;

  if (0 == depth) return;

  if (-1 != depth) --depth;

  switch (msg.type_)
  {
  case proto::type_varint:
  case proto::type_int32:
  case proto::type_int64:
  {
    static const char* _value_type_desc[] = {"varint", "int64", "", "", "", "int32"};
    for (const auto& value : msg.values_)
    {
      // 1 : /* varint */ 12345
      result += std::string(_cur_leftspace, ' ');
      result += std::to_string(msg.id_);
      result += " : ";
      result += std::to_string(value);
      result += ',';
      if (show_type > 0)
      {
        result += " /* ";
        result += _value_type_desc[int(msg.type_)];
        result += " */ ";
      }
      result += '\n';
    }
    break;
  }
  case proto::type_binary:
  {
    for (const auto& value : msg.binary_values_)
    {
      // "1" : "saddf"
      result += std::string(_cur_leftspace, ' ');
      result += std::to_string(msg.id_);
      result += " : \"";
      result += to_readable_string(value.data(), value.size());
      result += "\",\n";
    }
    break;
  }
  case proto::type_group:
  {
    // 1 : { /* group */ /* child: 4 */
    //     1 : xxx
    // }
    result += std::string(_cur_leftspace, ' ');
    result += "";
    result += std::to_string(msg.id_);
    result += " : {";
    if (2 == show_type) result += " /* group */";

    if (show_size)
    {
      result += " /* childs: ";
      result += std::to_string(msg.childs_.size());
      result += " */\n";
    }
    else
      result += '\n';

    for (const auto& f : msg.childs_)
    {
      to_string(result, f, indent, _cur_leftspace, depth, show_type, show_size);
    }
    result += std::string(_cur_leftspace, ' ');
    result += "},\n";
    break;
  }
  case proto::type_packed:
  {
    // 1 : { /* packed binary */ /* len: 4 */ /* child: 4 */
    //     1 : xxx
    // }
    result += std::string(_cur_leftspace, ' ');
    result += std::to_string(msg.id_);
    result += " : {";
    if (show_type > 0) result += " /* packed binary */";

    if (show_size)
    {
      if (!msg.binary_values_.empty())
      {
        result += " /* len: ";
        result += std::to_string(msg.binary_values_[0].size());
        result += " */ /* child: ";
        result += std::to_string(msg.childs_.size());
        result += " */\n";
      }
      else
      {
        result += " /* len:  */ /* child: ";
        result += std::to_string(msg.childs_.size());
        result += " */\n";
      }
    }
    else
      result += '\n';

    for (const auto& f : msg.childs_)
    {
      to_string(result, f, indent, _cur_leftspace, depth, show_type, show_size);
    }
    result += std::string(_cur_leftspace, ' ');
    result += "},\n";
    break;
  }
  case proto::type_repeat:
  {
    result += std::string(_cur_leftspace, ' ');
    result += "/* repeat count: ";
    result += std::to_string(msg.childs_.size());
    result += "*/\n";
  }
  default:
  {
    for (const auto& f : msg.childs_)
    {
      to_string(result, f, indent, _cur_leftspace - 2, depth, show_type, show_size);
    }
    break;
  }
  }
}

/**
 * \brief get human readble string view struct
 * \param show_type
 *    control how to show type info
 *    0: do not show
 *    1: less (varint, int64, int32, packed)
 *    2: full (varint, int64, int32, packed, binary, group)
 * \param show_size
 *    control how to show size info
 *    false: do not show
 *    true: show (packed, binary, group)
 * \return readble string
 */
std::string to_string(const proto::message& msg, int indent = 2, int leftspace = 0, int depth = -1,
  int show_type = 2, bool show_size = true)
{
  std::string result;
  result += "{\n";
  to_string(result, msg, indent, leftspace, depth, show_type, show_size);
  result += "}\n";
  return result;
}

/**
 * \brief get cpp code
 */
void to_cpp_code(
  std::string& result, const proto::message& msg, int indent, int leftspace, int depth, bool use_namespace)
{
  auto _cur_leftspace = leftspace + indent;

  if (0 == depth) return;

  if (-1 != depth) --depth;

  switch (msg.type_)
  {
  case proto::type_varint:
  case proto::type_int32:
  case proto::type_int64:
  {
    static const char* _value_type_desc[] = {"varint", "int64", "", "", "", "int32"};

    for (const auto& value : msg.values_)
    {
      // proto::varint<1>{ 3 },
      result += std::string(_cur_leftspace, ' ');
      if (use_namespace) result += "proto::";
      result += _value_type_desc[int(msg.type_)];
      result += "<";
      result += std::to_string(msg.id_);
      result += ">{ ";
      result += std::to_string(value);
      result += " },\n";
    }
  }
  break;
  case proto::type_binary:
  {
    for (const auto& value : msg.binary_values_)
    {
      // proto::binary<2>{ "saddf" },
      result += std::string(_cur_leftspace, ' ');
      if (use_namespace) result += "proto::";
      result += "binary<";
      result += std::to_string(msg.id_);
      result += ">{ \"";
      result += to_readable_string(value.data(), value.size());
      result += "\" },\n";
    }
  }
  break;
  case proto::type_group:
  {
    // proto::group<3>{
    result += std::string(_cur_leftspace, ' ');
    if (use_namespace) result += "proto::";
    result += "group<";
    result += std::to_string(msg.id_);
    result += ">{\n";

    for (const auto& f : msg.childs_) to_cpp_code(result, f, indent, _cur_leftspace, depth, use_namespace);

    result += std::string(_cur_leftspace, ' ');
    result += "},\n";
  }
  break;
  case proto::type_packed:
  {
    // proto::packed<4>{
    result += std::string(_cur_leftspace, ' ');
    if (use_namespace) result += "proto::";
    result += "packed<";
    result += std::to_string(msg.id_);
    result += ">{\n";

    for (const auto& f : msg.childs_)
    {
      to_cpp_code(result, f, indent, _cur_leftspace, depth, use_namespace);
    }
    result += std::string(_cur_leftspace, ' ');
    result += "},\n";
    break;
  }
  default:
  {
    for (const auto& f : msg.childs_) to_cpp_code(result, f, indent, _cur_leftspace - 2, depth, use_namespace);
  }
  break;
  }
}

/**
 * \brief get cpp code
 */
std::string to_cpp_code(const proto::message& msg, int indent = 2, int leftspace = 0, int depth = -1,
  bool use_namespace = false)
{
  std::string result;
  result += "{\n";
  to_cpp_code(result, msg, indent, leftspace, depth, use_namespace);
  result += "}\n";
  return result;
}


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
		success = load_from_stdin(msg, opt_depth);
	}

	if (success || opt_force)
	{
		switch (opt_style)
		{
		case cpp:
			std::cout << to_cpp_code(msg);
			break;
		default:
			std::cout << to_string(msg);
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
