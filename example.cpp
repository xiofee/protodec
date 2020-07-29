#include <stdio.h>
#include "proto.hpp"

using namespace proto;

message msg {
    varint<0>{1},
    varint<0>{9999999},
    varint<0>{0x7fffffff},
    varint<0>{0x80000000},
    int32<1>{0xffffffff},
    int64<3>{0xffffffff},
    binary<4>{"i am binary, my id is 4"},
    group<5> {},
    group<5> {
        varint<0>{1},
        varint<0>{2},
        varint<0>{3},
        varint<0>{4},
    }
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

int main() {
    auto bin = msg.serialize();
    for (auto c : bin) {
        printf("%02X ", int(c) & 0xff);
    }
    printf("\n");

    message dec;
    if (dec.deserialize(bin)) {
        printf("deserialize success\n");
    } else {
        printf("deserialize fail\n");
    }
    auto text = to_string(dec);
    printf(text.c_str());
    return 0;
}
