#ifndef __PROTO_HPP__
#define __PROTO_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#  pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <vector>
#include <list>
#include <tuple>
#include <string>

namespace proto {

namespace {
const int MAX_VARINT32_BYTES = 5;
const int MAX_VARINT64_BYTES = 10;
const int INT32_BYTES        = 4;
const int INT64_BYTES        = 8;
} // namespace

enum types {
  type_varint    = 0,
  type_int64     = 1,
  type_binary    = 2,
  type_group     = 3,
  type_end       = 4,
  type_int32     = 5,
  type_reserve1  = 6,
  type_reserve2  = 7,
  type_undefined = 8,
  type_packed    = 9,
  type_repeat    = 10
};

/**
 * \brief encode a number to varint encoding
 * \param num number
 * \return encoded varint string
 */
std::string        encode_varint(uint64_t num);
inline std::string encode_varint(uint64_t num)
{
  std::string _result;

  while (num > 0x7f)
  {
    _result += uint8_t((num & 0x7F) | 0x80);
    num >>= 7;
  }
  _result += uint8_t(num);

  return _result;
}

size_t        decode_varint(const void* data, size_t length, uint64_t& result);
inline size_t decode_varint(const void* data, size_t length, uint64_t& result)
{
  auto _data = static_cast<const unsigned char*>(data);

  if (nullptr == _data || 0 == length) return 0;

  if (length > 10) length = 10;

  int      _max = length;
  int      i    = 0;
  uint64_t r    = 0;
  while (i < _max)
  {
    auto c = _data[i];
    r += (c & 0x7F) << (i * 7);
    ++i;
    if (0 == (c & 0x80))
    {
      result = r;
      return i;
    }
  }

  return 0;
}

/**
 * \brief protobuf root field class
 */
class message
{
public:
  types                      type_;
  int                        id_;
  std::list<message>         childs_;        // sub fields or repeat field
  std::vector<std::uint64_t> values_;        // varint, int32, int64 data store here
  std::vector<std::string>   binary_values_; // binary data store here

  ////////////////////////////////////////////////////////////
  message()
    : type_(type_undefined)
    , id_(0)
    , values_{0}
    , binary_values_{""}
  {}

  message(types type, int id)
    : type_(type)
    , id_(id)
    , values_{0}
    , binary_values_{""}
  {}

  message(types type, int id, std::uint64_t value)
    : type_(type)
    , id_(id)
    , values_{value}
    , binary_values_{""}
  {}

  message(types type, int id, const std::string& value)
    : type_(type)
    , id_(id)
    , values_{0}
    , binary_values_{value}
  {}

  message(types type, int id, const std::vector<std::uint32_t>& values)
    : type_(type)
    , id_(id)
    , binary_values_{""}
  {
    append_value(values);
  }

  message(types type, int id, const std::vector<std::uint64_t>& values)
    : type_(type)
    , id_(id)
    , binary_values_{""}
  {
    append_value(values);
  }

  message(types type, int id, const std::vector<std::string>& values)
    : type_(type)
    , id_(id)
    , values_{0}
  {
    append_value(values);
  }

  message(types type, int id, const std::initializer_list<std::uint32_t>& values)
    : type_(type)
    , id_(id)
    , binary_values_{""}
  {
    append_value(values);
  }

  message(types type, int id, const std::initializer_list<std::uint64_t>& values)
    : type_(type)
    , id_(id)
    , binary_values_{""}
  {
    append_value(values);
  }

  message(types type, int id, const std::initializer_list<std::string>& values)
    : type_(type)
    , id_(id)
  {
    append_value(values);
  }

  explicit message(const std::vector<message>& fields)
    : type_(type_undefined)
    , id_(0)
  {
    append_child(fields);
  }

  explicit message(const std::list<message>& fields)
    : type_(type_undefined)
    , id_(0)
  {
    append_child(fields);
  }

  message(const std::initializer_list<message>& fields)
    : type_(type_undefined)
    , id_(0)
  {
    append_child(fields);
  }

  message(const message& obj)
  {
    if (&obj != this)
    {
      type_          = obj.type_;
      id_            = obj.id_;
      childs_        = obj.childs_;
      values_        = obj.values_;
      binary_values_ = obj.binary_values_;
    }
  }

  message(message&& obj) noexcept
  {
    if (&obj != this)
    {
      type_          = obj.type_;
      id_            = obj.id_;
      childs_        = std::move(obj.childs_);
      values_        = std::move(obj.values_);
      binary_values_ = std::move(obj.binary_values_);
    }
  }

  message& operator=(const message& obj)
  {
    if (&obj != this)
    {
      type_          = obj.type_;
      id_            = obj.id_;
      childs_        = obj.childs_;
      values_        = obj.values_;
      binary_values_ = obj.binary_values_;
    }
    return *this;
  }

  ////////////////////////////////////////////////////////////
  void set_value(std::uint64_t value)
  {
    binary_values_.clear();
    binary_values_.resize(1);
    values_.clear();
    values_.push_back(value);
  }

  void set_value(const std::string& value)
  {
    values_.clear();
    values_.resize(1);
    binary_values_.clear();
    binary_values_.emplace_back(value);
  }

  ////////////////////////////////////////////////////////////
  void set_value(const std::vector<std::uint32_t>& values)
  {
    binary_values_.clear();
    binary_values_.resize(1);
    values_.clear();
    values_.reserve(values.size());
    for (auto v : values)
    {
      values_.push_back(v);
    }
  }

  void set_value(const std::vector<std::uint64_t>& values)
  {
    binary_values_.clear();
    binary_values_.resize(1);
    values_.clear();
    values_.reserve(values.size());
    for (auto v : values)
    {
      values_.push_back(v);
    }
  }

  void set_value(const std::vector<std::string>& values)
  {
    values_.clear();
    values_.resize(1);
    binary_values_.clear();
    binary_values_.reserve(values.size());
    for (const auto& f : values)
    {
      binary_values_.emplace_back(f);
    }
  }

  ////////////////////////////////////////////////////////////
  void set_value(const std::initializer_list<std::uint32_t>& values)
  {
    binary_values_.clear();
    binary_values_.resize(1);
    values_.clear();
    values_.reserve(values.size());
    for (auto v : values)
    {
      values_.push_back(v);
    }
  }

  void set_value(const std::initializer_list<std::uint64_t>& values)
  {
    binary_values_.clear();
    binary_values_.resize(1);
    values_.clear();
    values_.reserve(values.size());
    for (auto v : values)
    {
      values_.push_back(v);
    }
  }

  void set_value(const std::initializer_list<std::string>& values)
  {
    values_.clear();
    values_.resize(1);
    binary_values_.clear();
    binary_values_.reserve(values.size());
    for (const auto& f : values)
    {
      binary_values_.emplace_back(f);
    }
  }

  ////////////////////////////////////////////////////////////
  void append_value(std::uint32_t value) { values_.push_back(value); }

  void append_value(std::uint64_t value) { values_.push_back(value); }

  void append_value(const std::string& value) { binary_values_.emplace_back(value); }

  ////////////////////////////////////////////////////////////
  void append_value(const std::vector<std::uint32_t>& values)
  {
    values_.reserve(values.size());
    for (auto v : values)
    {
      values_.push_back(v);
    }
  }

  void append_value(const std::vector<std::uint64_t>& values)
  {
    values_.reserve(values.size());
    for (auto v : values)
    {
      values_.push_back(v);
    }
  }

  void append_value(const std::vector<std::string>& values)
  {
    binary_values_.reserve(values.size());
    for (const auto& v : values)
    {
      binary_values_.emplace_back(v);
    }
  }

  ////////////////////////////////////////////////////////////
  void append_value(const std::initializer_list<std::uint32_t>& values)
  {
    values_.reserve(values.size());
    for (auto v : values)
    {
      values_.push_back(v);
    }
  }

  void append_value(const std::initializer_list<std::uint64_t>& values)
  {
    values_.reserve(values.size());
    for (auto v : values)
    {
      values_.push_back(v);
    }
  }

  void append_value(const std::initializer_list<std::string>& values)
  {
    binary_values_.reserve(values.size());
    for (const auto& v : values)
    {
      binary_values_.emplace_back(v);
    }
  }

  ////////////////////////////////////////////////////////////
  void set_child(const message& f)
  {
    childs_.clear();
    this->id(f.id_) = f;
  }

  void set_child(const std::vector<message>& fields)
  {
    childs_.clear();
    for (const auto& f : fields)
    {
      this->id(f.id_) = f;
    }
  }

  void set_child(const std::list<message>& fields)
  {
    childs_.clear();
    for (const auto& f : fields)
    {
      this->id(f.id_) = f;
    }
  }

  void set_child(const std::initializer_list<message>& fields)
  {
    childs_.clear();
    for (const auto& f : fields)
    {
      this->id(f.id_) = f;
    }
  }

  ////////////////////////////////////////////////////////////
  void append_child(const message& f)
  {
    // get from childs
    auto& _field = this->id(f.id_);

    // id not exits, append
    if (!_field)
    {
      childs_.push_back(f);
      return;
    }

    // else id exits, repeat field

    switch (_field.type_)
    {
    case type_varint:
    case type_int32:
    case type_int64:
    {
      _field.append_value(f.values_);
      break;
    }
    case type_binary:
    {
      _field.append_value(f.binary_values_);
      break;
    }
    case type_group:
    case type_packed:
    {
      auto _new_field = message{type_repeat, f.id_};
      _new_field.childs_.push_back(_field);
      _new_field.childs_.push_back(f);
      _field = _new_field;
      break;
    }
    case type_repeat:
    {
      _field.childs_.push_back(f);
      break;
    }
    default:;
    }
  }

  void append_child(message&& f)
  {
    // get from childs
    auto& _field = this->id(f.id_);

    // id not exits, append
    if (!_field)
    {
      childs_.emplace_back(std::move(f));
      return;
    }

    // else id exits, repeat field

    switch (_field.type_)
    {
    case type_varint:
    case type_int32:
    case type_int64:
    {
      _field.append_value(f.values_);
      break;
    }
    case type_binary:
    {
      _field.append_value(f.binary_values_);
      break;
    }
    case type_group:
    case type_packed:
    {
      auto _new_field = message{type_repeat, f.id_};
      _new_field.childs_.push_back(_field);
      _new_field.childs_.emplace_back(std::move(f));
      _field = std::move(_new_field);
      break;
    }
    case type_repeat:
    {
      _field.childs_.emplace_back(std::move(f));
      break;
    }
    default:;
    }
  }

  void append_child(const std::vector<message>& fields)
  {
    for (const auto& f : fields)
    {
      append_child(f);
    }
  }

  void append_child(const std::list<message>& fields)
  {
    for (const auto& f : fields)
    {
      append_child(f);
    }
  }

  void append_child(const std::initializer_list<message>& fields)
  {
    for (const auto& f : fields)
    {
      append_child(f);
    }
  }

  ////////////////////////////////////////////////////////////

  std::uint64_t value()
  {
    if (values_.empty())
    {
      values_.push_back(0);
    }
    return values_[0];
  }

  const std::string& binary_value()
  {
    if (binary_values_.empty())
    {
      binary_values_.emplace_back("");
    }
    return binary_values_[0];
  }

  bool is_repeat() const { return type_repeat == type_ || values_.size() > 1 || binary_values_.size() > 1; }

  bool has(int id) const
  {
    for (auto& f : childs_)
    {
      if (id == f.id_) return true;
    }
    return false;
  }

  bool has_child() const { return !childs_.empty(); }

  message& at(size_t index)
  {
    static message _undefined;
    for (auto& f : childs_)
    {
      if (0 == index) return f;
      --index;
    }
    return _undefined;
  }

  message& id(int id)
  {
    static message _undefined;
    for (auto& f : childs_)
    {
      if (id == f.id_) return f;
    }
    return _undefined;
  }

  message& operator[](int id) { return this->id(id); }

  typedef void (*unspecified_bool_type)();
  static void unspecified_bool_true() {}

  operator unspecified_bool_type() const noexcept { return (type_undefined == type_) ? 0 : unspecified_bool_true; }

  /**
   * \brief serialize to binary
   * \return serialized binary protobuf data
   */
  std::string serialize() { return serialize(*this); }

  /**
   * \brief deserialize protobuf from string
   * \param input serialized binary protobuf data
   * \param dec_pack_depth decode packed type depth
   * \return true if all data valid, else return false
   */
  bool deserialize(const std::string& input, int dec_pack_depth = -1)
  {
    auto result = deserialize(*this, input.data(), input.size(), 1, dec_pack_depth);
    return std::get<0>(result);
  }

  /**
   * \brief deserialize protobuf from string
   * \param input serialized binary protobuf data
   * \param length out processed length
   * \param dec_pack_depth decode packed type depth
   * \return true if all data valid, else return false
   */
  bool deserialize(const std::string& input, size_t* length, int dec_pack_depth = -1)
  {
    auto result = deserialize(*this, input.data(), *length, 1, dec_pack_depth);
    *length     = std::get<1>(result);
    return std::get<0>(result);
  }

  /**
   * \brief deserialize protobuf from string
   * \param input serialized binary protobuf data
   * \param length input length, out processed length
   * \param dec_pack_depth decode packed type depth
   * \return true if all data valid, else return false
   */
  bool deserialize(const void* input, size_t* length, int dec_pack_depth = -1)
  {
    auto result = deserialize(*this, input, *length, 1, dec_pack_depth);
    *length     = std::get<1>(result);
    return std::get<0>(result);
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
  std::string to_string(
    int indent = 2, int leftspace = 0, int depth = -1, int show_type = 2, bool show_size = true) const
  {
    std::string result;
    result += "{\n";
    to_string(result, *this, indent, leftspace, depth, show_type, show_size);
    result += "}\n";
    return result;
  }

  /**
   * \brief get cpp code
   */
  std::string to_cpp_code(int indent = 2, int leftspace = 0, int depth = -1, bool use_namespace = false) const
  {
    std::string result;
    result += "{\n";
    to_cpp_code(result, *this, indent, leftspace, depth, use_namespace);
    result += "}\n";
    return result;
  }

private:
  /**
   * \brief convert to protobuf field key
   */
  inline std::string encode_key(types type, int id)
  {
    auto _key = (std::uint64_t(id) << 3) | std::uint64_t(type);
    return encode_varint(_key);
  }

  static char byte_to_hex(unsigned char b, bool lower = true)
  {
    static const char* hex_map[2] = {"0123456789ABCDEF", "0123456789abcdef"};

    return hex_map[int(lower)][int(b)];
  }

  /**
   * \brief convert un-printable character to escape sequences
   */
  static std::string to_readable_string(const void* data, size_t len)
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

  int calc_varint_encoded_size(std::uint64_t num)
  {
    // 0x000000000000007F 0 0000000 0000000 0000000 0000000 0000000 0000000 0000000 0000000 1111111
    // 0x0000000000003F80 0 0000000 0000000 0000000 0000000 0000000 0000000 0000000 1111111 0000000
    // 0x00000000001FC000 0 0000000 0000000 0000000 0000000 0000000 0000000 1111111 0000000 0000000
    // 0x000000000FE00000 0 0000000 0000000 0000000 0000000 0000000 1111111 0000000 0000000 0000000
    // 0x00000007F0000000 0 0000000 0000000 0000000 0000000 1111111 0000000 0000000 0000000 0000000

    // 0x000003F800000000 0 0000000 0000000 0000000 1111111 0000000 0000000 0000000 0000000 0000000
    // 0x0001FC0000000000 0 0000000 0000000 1111111 0000000 0000000 0000000 0000000 0000000 0000000
    // 0x00FE000000000000 0 0000000 1111111 0000000 0000000 0000000 0000000 0000000 0000000 0000000
    // 0x7F00000000000000 0 1111111 0000000 0000000 0000000 0000000 0000000 0000000 0000000 0000000
    // 0x8000000000000000 1 0000000 0000000 0000000 0000000 0000000 0000000 0000000 0000000 0000000

    if (num <= (std::numeric_limits<uint32_t>::max)())
    {
      return 1                       //
             + !!(num & 0x000003F80) //
             + !!(num & 0x0001FC000) //
             + !!(num & 0x00FE00000) //
             + !!(num & 0x7F0000000);
    }

    return 1                              //
           + !!(num & 0x0000000000003F80) //
           + !!(num & 0x00000000001FC000) //
           + !!(num & 0x000000000FE00000) //
           + !!(num & 0x00000007F0000000) //
           + !!(num & 0x000003F800000000) //
           + !!(num & 0x0001FC0000000000) //
           + !!(num & 0x00FE000000000000) //
           + !!(num & 0x7F00000000000000) //
           + !!(num & 0x8000000000000000);
  }

  std::size_t calc_serialized_size(const message& msg, int indent, int leftspace)
  {
    std::size_t _totalsize     = 0;
    auto        _cur_leftspace = leftspace + indent;

    switch (msg.type_)
    {
    case type_varint:
    {
      for (const auto& value : msg.values_)
      {
        std::ignore = value;
        _totalsize += calc_varint_encoded_size(msg.id_);
        _totalsize += calc_varint_encoded_size(value);
      }
      break;
    }
    case type_int32:
    {
      for (const auto& value : msg.values_)
      {
        std::ignore = value;
        _totalsize += calc_varint_encoded_size(msg.id_);
        _totalsize += INT32_BYTES;
      }
      break;
    }
    case type_int64:
    {
      for (const auto& value : msg.values_)
      {
        std::ignore = value;
        _totalsize += calc_varint_encoded_size(msg.id_);
        _totalsize += INT64_BYTES;
      }
      break;
    }
    case type_binary:
    {
      for (const auto& value : msg.binary_values_)
      {
        _totalsize += calc_varint_encoded_size(msg.id_);
        _totalsize += calc_varint_encoded_size(value.size());
        _totalsize += value.size();
      }
      break;
    }
    case type_group:
    {
      _totalsize += calc_varint_encoded_size(msg.id_) * 2;

      for (const auto& f : msg.childs_)
      {
        _totalsize += calc_serialized_size(f, indent, _cur_leftspace);
      }

      break;
    }
    case type_packed:
    {
      _totalsize += calc_varint_encoded_size(msg.id_);

      std::size_t _subsize = 0;
      for (const auto& f : msg.childs_)
      {
        _subsize += calc_serialized_size(f, indent, _cur_leftspace);
      }
      _totalsize += calc_varint_encoded_size(_subsize);
      _totalsize += _subsize;
      break;
    }
    case type_repeat:
    case type_undefined:
    {
      for (const auto& f : msg.childs_)
      {
        _totalsize += calc_serialized_size(f, indent, _cur_leftspace - 2);
      }
      break;
    }
    default:;
    }

    return _totalsize;
  }

  /**
   * \brief encode to protobuf
   */
  std::string serialize(const message& msg)
  {
    std::string _result;
    _result.reserve(calc_serialized_size(msg, 2, 0));

    switch (msg.type_)
    {
    case type_varint:
    {
      //using std_uint64_p_t = std::uint64_t*;
      for (const auto& value : msg.values_)
      {
        _result += encode_key(msg.type_, msg.id_);
        _result += encode_varint(value);
      }
      break;
    }
    case type_int32:
    {
      for (const auto& value : msg.values_)
      {
        using const_char_ptr = const char*;
        _result += encode_key(msg.type_, msg.id_);
        _result.append(const_char_ptr(&value), INT32_BYTES);
      }
      break;
    }
    case type_int64:
    {
      for (const auto& value : msg.values_)
      {
        using char_ptr = const char*;
        _result += encode_key(msg.type_, msg.id_);
        _result.append(char_ptr(&value), INT64_BYTES);
      }
      break;
    }
    case type_binary:
    {
      for (const auto& value : msg.binary_values_)
      {
        std::uint64_t _value_size = value.size();
        _result += encode_key(msg.type_, msg.id_);
        _result += encode_varint(_value_size);
        _result += value;
      }
      break;
    }
    case type_group:
    {
      _result += encode_key(msg.type_, msg.id_);
      for (const auto& f : msg.childs_)
      {
        _result += serialize(f);
      }
      _result += encode_key(type_end, msg.id_);
      break;
    }
    case type_packed:
    {
      _result += encode_key(type_binary, msg.id_);
      std::string _message;
      for (const auto& f : msg.childs_)
      {
        _message += serialize(f);
      }
      std::uint64_t _message_size = _message.size();

      _result += encode_varint(_message_size);
      _result += _message;
      break;
    }
    case type_repeat:
    case type_undefined:
    {
      for (const auto& f : msg.childs_)
      {
        _result += serialize(f);
      }
      break;
    }
    default:;
    }

    // mutex_.unlock();

    return _result;
  }

  /**
   * \brief deserialize protobuf from string
   * \return { bool success, int used_size, int left_size }
   */
  std::tuple<bool, int, int> deserialize(
    message& msg, const void* input, const std::size_t length, int cur_depth, int dec_pack_depth)
  {
    if (0 == length) return std::make_tuple(false, 0, 0);

    auto _pdata = static_cast<const unsigned char*>(input);
    auto _left  = length;

    while (_left > 0)
    {
      // get key
      int _key;
      {
        std::uint64_t _key_u64;
        auto          _size = decode_varint(_pdata, _left, _key_u64);
        if (0 == _size) return std::make_tuple(false, int(length - _left), int(_left));

        _key = int(_key_u64);
        _pdata += _size;
        _left -= _size;
      }

      // extra id and type
      auto _id    = _key >> 3;
      auto _itype = _key & 7;

      if (!(_itype >= 0 && _itype < int(type_undefined)))
        return std::make_tuple(false, int(length - _left), int(_left));

      auto _type = types(_itype);

      if (0 == _left && !(type_group == msg.type_ && type_end == _type)) return std::make_tuple(false, int(length), 0);

      switch (_type)
      {
      case type_varint:
      {
        std::uint64_t _value;
        {
          auto _size = decode_varint(_pdata, _left, _value);
          if (0 == _size) return std::make_tuple(false, int(length - _left), int(_left));

          _pdata += _size;
          _left -= _size;
        }
        msg.append_child({type_varint, _id, _value});
        break;
      }
      case type_int64:
      {
        if (_left < sizeof(std::uint64_t)) return std::make_tuple(false, int(length - _left), int(_left));

        auto _value = *(reinterpret_cast<const std::uint64_t*>(_pdata));
        _pdata += sizeof(std::uint64_t);
        _left -= sizeof(std::uint64_t);

        msg.append_child({type_int64, _id, _value});
        break;
      }
      case type_binary:
      {
        std::size_t _binary_length;
        {
          std::uint64_t _binary_length_u64;
          auto          _size = decode_varint(_pdata, _left, _binary_length_u64);
          if (0 == _size) return std::make_tuple(false, int(length - _left), int(_left));

          _binary_length = std::size_t(_binary_length_u64);
          _pdata += _size;
          _left -= _size;
        }

        if (_left < _binary_length) return std::make_tuple(false, int(length - _left), int(_left));

        message _message{type_binary, _id};

        if (-1 == dec_pack_depth || (-1 != dec_pack_depth && cur_depth <= dec_pack_depth))
        // try dec packed message
        {
          message _packed{type_packed, _id};
          auto    _result = deserialize(_packed, _pdata, std::size_t(_binary_length), cur_depth + 1, dec_pack_depth);
          if (std::get<0>(_result))
          {
            _message.type_   = type_packed;
            _message.childs_ = std::move(_packed.childs_);
          }
        }

        // add raw binary message
        {
          using std_string_elem_t  = const std::string::value_type* const;
          using std_string_value_t = std::string::size_type;
          _message.binary_values_.clear();
          _message.binary_values_.emplace_back(std_string_elem_t(_pdata), std_string_value_t(_binary_length));
        }

        // append
        msg.append_child(std::move(_message));

        _pdata += _binary_length;
        _left -= _binary_length;

        break;
      }
      case type_group:
      {
        message _subgroup{type_group, _id};
        auto    _result = deserialize(_subgroup, _pdata, _left, cur_depth + 1, dec_pack_depth);
        if (!std::get<0>(_result))
        {
          return std::make_tuple(false, int(length - _left), int(_left));
        }

        _pdata += std::get<1>(_result);
        _left = std::get<2>(_result);
        msg.append_child(_subgroup);
        break;
      }
      case type_end:
      {
        if (msg.type_ == type_group)
        {
          return std::make_tuple(true, int(length - _left), int(_left));
        }
        break;
      }
      case type_int32:
      {
        if (_left < sizeof(std::uint32_t)) return std::make_tuple(false, int(length - _left), int(_left));

        auto _value = *(reinterpret_cast<const std::uint32_t*>(_pdata));
        _pdata += sizeof(std::uint32_t);
        _left -= sizeof(std::uint32_t);

        msg.append_child({type_int32, _id, _value});
        break;
      }
      default:
        return std::make_tuple(false, int(length - _left), int(_left));
      }
    }

    return std::make_tuple(0 == _left, int(length - _left), int(_left));
  }

  /**
   * \brief get human readble string of data struct view
   */
  void to_string(std::string& result, const proto::message& msg, int indent, int leftspace, int depth, int show_type,
    bool show_size) const
  {
    auto _cur_leftspace = leftspace + indent;

    if (0 == depth) return;

    if (-1 != depth) --depth;

    switch (msg.type_)
    {
    case type_varint:
    case type_int32:
    case type_int64:
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
    case type_binary:
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
    case type_group:
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
    case type_packed:
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
    case type_repeat:
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
   * \brief get cpp code
   */
  void to_cpp_code(
    std::string& result, const proto::message& msg, int indent, int leftspace, int depth, bool use_namespace) const
  {
    auto _cur_leftspace = leftspace + indent;

    if (0 == depth) return;

    if (-1 != depth) --depth;

    switch (msg.type_)
    {
    case type_varint:
    case type_int32:
    case type_int64:
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
    case type_binary:
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
    case type_group:
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
    case type_packed:
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
};

template<int ID>
class varint : public message
{
public:
  varint(std::uint64_t value)
    : message(type_varint, ID, value)
  {}

  varint(const std::vector<std::uint64_t>& values)
    : message(type_varint, ID, values)
  {}

  varint(const std::initializer_list<std::uint64_t>& values)
    : message(type_varint, ID, values)
  {}
};

template<int ID>
class int32 : public message
{
public:
  int32(std::uint32_t value)
    : message(type_int32, ID, value)
  {}

  int32(const std::vector<std::uint32_t>& values)
    : message(type_int32, ID, values)
  {}

  int32(const std::initializer_list<std::uint32_t>& values)
    : message(type_int32, ID, values)
  {}
};

template<int ID>
class int64 : public message
{
public:
  int64(const std::uint64_t value)
    : message(type_int64, ID, value)
  {}

  int64(const std::vector<std::uint64_t>& values)
    : message(type_int64, ID, values)
  {}

  int64(const std::initializer_list<std::uint64_t>& values)
    : message(type_int64, ID, values)
  {}
};

template<int ID>
class binary : public message
{
public:
  binary(const std::string& value)
    : message(type_binary, ID, value)
  {}

  binary(const std::vector<std::string>& values)
    : message(type_binary, ID, values)
  {}

  binary(const std::initializer_list<std::string>& values)
    : message(type_binary, ID, values)
  {}
};

template<int ID>
class group : public message
{
public:
  group(const std::vector<message>& fields)
    : message(fields)
  {
    type_ = type_group;
    id_   = ID;
  }

  group(const std::list<message>& fields)
    : message(fields)
  {
    type_ = type_group;
    id_   = ID;
  }

  group(const std::initializer_list<message>& fields)
    : message(fields)
  {
    type_ = type_group;
    id_   = ID;
  }
};

template<int ID>
class packed : public message
{
public:
  packed(const std::vector<message>& fields)
    : message(fields)
  {
    type_ = type_packed;
    id_   = ID;
  }

  packed(const std::list<message>& fields)
    : message(fields)
  {
    type_ = type_packed;
    id_   = ID;
  }

  packed(const std::initializer_list<message>& fields)
    : message(fields)
  {
    type_ = type_packed;
    id_   = ID;
  }
};

} // namespace proto

#endif // !__PROTO_HPP__
