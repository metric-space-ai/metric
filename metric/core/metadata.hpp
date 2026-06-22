// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef METRIC_CORE_METADATA_HPP
#define METRIC_CORE_METADATA_HPP

#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace mtrc::core {

class Metadata {
  public:
	enum class Kind { null_value, object_value, array_value, string_value, number_value, bool_value };

	Metadata() = default;
	Metadata(std::nullptr_t) {}
	Metadata(const char *value) : kind_(Kind::string_value), string_(value == nullptr ? "" : value) {}
	Metadata(std::string value) : kind_(Kind::string_value), string_(std::move(value)) {}
	Metadata(bool value) : kind_(Kind::bool_value), bool_(value) {}

	template <typename Number, typename std::enable_if<std::is_arithmetic<Number>::value &&
														  !std::is_same<typename std::decay<Number>::type, bool>::value,
													  int>::type = 0>
	Metadata(Number value) : kind_(Kind::number_value), number_(static_cast<long double>(value))
	{
	}

	Metadata(const std::map<std::string, std::string> &values) : kind_(Kind::object_value)
	{
		for (const auto &entry : values) {
			object_.emplace(entry.first, Metadata(entry.second));
		}
	}

	template <typename Value> Metadata(const std::vector<Value> &values) : kind_(Kind::array_value)
	{
		array_.reserve(values.size());
		for (const auto &value : values) {
			array_.emplace_back(value);
		}
	}

	Metadata(std::initializer_list<std::pair<std::string, Metadata>> fields) : kind_(Kind::object_value)
	{
		for (const auto &field : fields) {
			object_[field.first] = field.second;
		}
	}

	static auto object() -> Metadata
	{
		Metadata value;
		value.kind_ = Kind::object_value;
		return value;
	}

	static auto array() -> Metadata
	{
		Metadata value;
		value.kind_ = Kind::array_value;
		return value;
	}

	auto kind() const -> Kind { return kind_; }
	auto is_null() const -> bool { return kind_ == Kind::null_value; }
	auto is_object() const -> bool { return kind_ == Kind::object_value; }
	auto is_array() const -> bool { return kind_ == Kind::array_value; }
	auto is_string() const -> bool { return kind_ == Kind::string_value; }
	auto is_number() const -> bool { return kind_ == Kind::number_value; }
	auto is_bool() const -> bool { return kind_ == Kind::bool_value; }

	auto empty() const -> bool
	{
		switch (kind_) {
		case Kind::null_value:
			return true;
		case Kind::object_value:
			return object_.empty();
		case Kind::array_value:
			return array_.empty();
		case Kind::string_value:
			return string_.empty();
		case Kind::number_value:
		case Kind::bool_value:
			return false;
		}
		return true;
	}

	auto size() const -> std::size_t
	{
		if (kind_ == Kind::object_value) {
			return object_.size();
		}
		if (kind_ == Kind::array_value) {
			return array_.size();
		}
		if (kind_ == Kind::string_value) {
			return string_.size();
		}
		return 0;
	}

	auto contains(const std::string &key) const -> bool
	{
		return kind_ == Kind::object_value && object_.find(key) != object_.end();
	}

		auto operator[](const std::string &key) -> Metadata &
		{
			ensure_object();
			return object_[key];
		}

		auto operator[](const std::string &key) const -> const Metadata &
		{
			return at(key);
		}

	auto at(const std::string &key) -> Metadata &
	{
		require_object();
		return object_.at(key);
	}

	auto at(const std::string &key) const -> const Metadata &
	{
		require_object();
		return object_.at(key);
	}

	auto at(std::size_t index) -> Metadata &
	{
		require_array();
		return array_.at(index);
	}

		auto at(std::size_t index) const -> const Metadata &
		{
			require_array();
			return array_.at(index);
		}

		auto front() -> Metadata &
		{
			require_array();
			return array_.front();
		}

		auto front() const -> const Metadata &
		{
			require_array();
			return array_.front();
		}

		auto back() -> Metadata &
		{
			require_array();
			return array_.back();
		}

		auto back() const -> const Metadata &
		{
			require_array();
			return array_.back();
		}

	auto push_back(Metadata value) -> void
	{
		ensure_array();
		array_.push_back(std::move(value));
	}

	auto begin() -> std::vector<Metadata>::iterator
	{
		ensure_array();
		return array_.begin();
	}

	auto end() -> std::vector<Metadata>::iterator
	{
		ensure_array();
		return array_.end();
	}

	auto begin() const -> std::vector<Metadata>::const_iterator
	{
		require_array();
		return array_.begin();
	}

	auto end() const -> std::vector<Metadata>::const_iterator
	{
		require_array();
		return array_.end();
	}

	auto update(Metadata other) -> void
	{
		ensure_object();
		other.require_object();
		for (auto &entry : other.object_) {
			object_[entry.first] = std::move(entry.second);
		}
	}

	template <typename T> auto value(const std::string &key, T fallback) const -> T
	{
		if (!contains(key)) {
			return fallback;
		}
		return at(key).template get<T>();
	}

	template <typename T> auto get() const -> T
	{
		using Decayed = typename std::decay<T>::type;
		if constexpr (std::is_same<Decayed, std::string>::value) {
			if (kind_ == Kind::string_value) {
				return string_;
			}
			throw std::runtime_error("metadata value is not a string");
		} else if constexpr (std::is_same<Decayed, bool>::value) {
			if (kind_ == Kind::bool_value) {
				return bool_;
			}
			throw std::runtime_error("metadata value is not a bool");
		} else if constexpr (std::is_integral<Decayed>::value) {
			if (kind_ != Kind::number_value) {
				throw std::runtime_error("metadata value is not a number");
			}
			return static_cast<Decayed>(number_);
		} else if constexpr (std::is_floating_point<Decayed>::value) {
			if (kind_ != Kind::number_value) {
				throw std::runtime_error("metadata value is not a number");
			}
			return static_cast<Decayed>(number_);
		} else if constexpr (is_vector<Decayed>::value) {
			require_array();
			Decayed result;
			result.reserve(array_.size());
			for (const auto &entry : array_) {
				result.push_back(entry.template get<typename Decayed::value_type>());
			}
			return result;
		} else if constexpr (is_string_map<Decayed>::value) {
			require_object();
			Decayed result;
			for (const auto &entry : object_) {
				result.emplace(entry.first, entry.second.template get<std::string>());
			}
			return result;
		} else {
			static_assert(always_false<Decayed>::value, "unsupported metadata conversion");
		}
	}

	auto dump() const -> std::string
	{
		std::ostringstream out;
		write(out);
		return out.str();
	}

	static auto parse(const std::string &text) -> Metadata
	{
		Parser parser(text);
		auto value = parser.parse_value();
		parser.skip_ws();
		if (!parser.done()) {
			throw std::runtime_error("trailing characters in metadata text");
		}
		return value;
	}

	friend auto operator==(const Metadata &lhs, const Metadata &rhs) -> bool
	{
		if (lhs.kind_ != rhs.kind_) {
			return false;
		}
		switch (lhs.kind_) {
		case Kind::null_value:
			return true;
		case Kind::object_value:
			return lhs.object_ == rhs.object_;
		case Kind::array_value:
			return lhs.array_ == rhs.array_;
		case Kind::string_value:
			return lhs.string_ == rhs.string_;
		case Kind::number_value:
			return lhs.number_ == rhs.number_;
		case Kind::bool_value:
			return lhs.bool_ == rhs.bool_;
		}
		return false;
	}

	friend auto operator!=(const Metadata &lhs, const Metadata &rhs) -> bool { return !(lhs == rhs); }
	friend auto operator==(const Metadata &lhs, const char *rhs) -> bool { return lhs == Metadata(rhs); }
	friend auto operator==(const char *lhs, const Metadata &rhs) -> bool { return Metadata(lhs) == rhs; }
	friend auto operator!=(const Metadata &lhs, const char *rhs) -> bool { return !(lhs == rhs); }

	template <typename T,
			  typename std::enable_if<std::is_arithmetic<T>::value || std::is_same<T, std::string>::value, int>::type = 0>
	friend auto operator==(const Metadata &lhs, const T &rhs) -> bool
	{
		return lhs == Metadata(rhs);
	}

		template <typename T,
			  typename std::enable_if<std::is_arithmetic<T>::value || std::is_same<T, std::string>::value, int>::type = 0>
	friend auto operator==(const T &lhs, const Metadata &rhs) -> bool
	{
		return Metadata(lhs) == rhs;
	}

	template <typename T,
			  typename std::enable_if<std::is_arithmetic<T>::value || std::is_same<T, std::string>::value, int>::type = 0>
	friend auto operator!=(const Metadata &lhs, const T &rhs) -> bool
	{
		return !(lhs == rhs);
	}

	template <typename T,
			  typename std::enable_if<std::is_arithmetic<T>::value || std::is_same<T, std::string>::value, int>::type = 0>
	friend auto operator!=(const T &lhs, const Metadata &rhs) -> bool
	{
		return !(lhs == rhs);
	}

  private:
	template <typename> struct always_false : std::false_type {};
	template <typename> struct is_vector : std::false_type {};
	template <typename Value, typename Allocator>
	struct is_vector<std::vector<Value, Allocator>> : std::true_type {};
	template <typename> struct is_string_map : std::false_type {};
	template <typename Compare, typename Allocator>
	struct is_string_map<std::map<std::string, std::string, Compare, Allocator>> : std::true_type {};

	class Parser {
	  public:
		explicit Parser(const std::string &text) : text_(text) {}

		auto done() const -> bool { return position_ >= text_.size(); }

		auto skip_ws() -> void
		{
			while (!done() && std::isspace(static_cast<unsigned char>(text_[position_]))) {
				++position_;
			}
		}

		auto parse_value() -> Metadata
		{
			skip_ws();
			if (done()) {
				throw std::runtime_error("unexpected end of metadata text");
			}
			const auto current = text_[position_];
			if (current == '{') {
				return parse_object();
			}
			if (current == '[') {
				return parse_array();
			}
			if (current == '"') {
				return Metadata(parse_string());
			}
			if (current == 't') {
				consume_literal("true");
				return Metadata(true);
			}
			if (current == 'f') {
				consume_literal("false");
				return Metadata(false);
			}
			if (current == 'n') {
				consume_literal("null");
				return Metadata();
			}
			return parse_number();
		}

	  private:
		auto parse_object() -> Metadata
		{
			consume('{');
			auto result = Metadata::object();
			skip_ws();
			if (peek('}')) {
				consume('}');
				return result;
			}
			while (true) {
				const auto key = parse_string();
				consume(':');
				result[key] = parse_value();
				skip_ws();
				if (peek('}')) {
					consume('}');
					return result;
				}
				consume(',');
			}
		}

		auto parse_array() -> Metadata
		{
			consume('[');
			auto result = Metadata::array();
			skip_ws();
			if (peek(']')) {
				consume(']');
				return result;
			}
			while (true) {
				result.push_back(parse_value());
				skip_ws();
				if (peek(']')) {
					consume(']');
					return result;
				}
				consume(',');
			}
		}

		auto parse_string() -> std::string
		{
			consume('"');
			std::string result;
			while (!done()) {
				const auto current = text_[position_++];
				if (current == '"') {
					return result;
				}
				if (current == '\\') {
					if (done()) {
						throw std::runtime_error("incomplete metadata escape sequence");
					}
					const auto escaped = text_[position_++];
					switch (escaped) {
					case '"':
					case '\\':
					case '/':
						result.push_back(escaped);
						break;
					case 'b':
						result.push_back('\b');
						break;
					case 'f':
						result.push_back('\f');
						break;
					case 'n':
						result.push_back('\n');
						break;
					case 'r':
						result.push_back('\r');
						break;
					case 't':
						result.push_back('\t');
						break;
					default:
						throw std::runtime_error("unsupported metadata escape sequence");
					}
				} else {
					result.push_back(current);
				}
			}
			throw std::runtime_error("unterminated metadata string");
		}

		auto parse_number() -> Metadata
		{
			const auto begin = position_;
			if (peek('-')) {
				++position_;
			}
			while (!done() && std::isdigit(static_cast<unsigned char>(text_[position_]))) {
				++position_;
			}
			if (!done() && text_[position_] == '.') {
				++position_;
				while (!done() && std::isdigit(static_cast<unsigned char>(text_[position_]))) {
					++position_;
				}
			}
			if (!done() && (text_[position_] == 'e' || text_[position_] == 'E')) {
				++position_;
				if (!done() && (text_[position_] == '+' || text_[position_] == '-')) {
					++position_;
				}
				while (!done() && std::isdigit(static_cast<unsigned char>(text_[position_]))) {
					++position_;
				}
			}
			if (begin == position_) {
				throw std::runtime_error("expected metadata number");
			}
			return Metadata(std::stold(text_.substr(begin, position_ - begin)));
		}

		auto consume(char expected) -> void
		{
			skip_ws();
			if (done() || text_[position_] != expected) {
				throw std::runtime_error("unexpected metadata character");
			}
			++position_;
		}

		auto consume_literal(const char *literal) -> void
		{
			while (*literal != '\0') {
				if (done() || text_[position_] != *literal) {
					throw std::runtime_error("unexpected metadata literal");
				}
				++position_;
				++literal;
			}
		}

		auto peek(char value) const -> bool { return !done() && text_[position_] == value; }

		const std::string &text_;
		std::size_t position_{0};
	};

	auto ensure_object() -> void
	{
		if (kind_ == Kind::null_value) {
			kind_ = Kind::object_value;
			return;
		}
		require_object();
	}

	auto ensure_array() -> void
	{
		if (kind_ == Kind::null_value) {
			kind_ = Kind::array_value;
			return;
		}
		require_array();
	}

	auto require_object() const -> void
	{
		if (kind_ != Kind::object_value) {
			throw std::runtime_error("metadata value is not an object");
		}
	}

	auto require_array() const -> void
	{
		if (kind_ != Kind::array_value) {
			throw std::runtime_error("metadata value is not an array");
		}
	}

	auto write(std::ostream &out) const -> void
	{
		switch (kind_) {
		case Kind::null_value:
			out << "null";
			break;
		case Kind::object_value:
			write_object(out);
			break;
		case Kind::array_value:
			write_array(out);
			break;
		case Kind::string_value:
			write_string(out, string_);
			break;
		case Kind::number_value:
			out.precision(std::numeric_limits<long double>::max_digits10);
			out << number_;
			break;
		case Kind::bool_value:
			out << (bool_ ? "true" : "false");
			break;
		}
	}

	auto write_object(std::ostream &out) const -> void
	{
		out << '{';
		bool first = true;
		for (const auto &entry : object_) {
			if (!first) {
				out << ',';
			}
			first = false;
			write_string(out, entry.first);
			out << ':';
			entry.second.write(out);
		}
		out << '}';
	}

	auto write_array(std::ostream &out) const -> void
	{
		out << '[';
		for (std::size_t index = 0; index < array_.size(); ++index) {
			if (index != 0) {
				out << ',';
			}
			array_[index].write(out);
		}
		out << ']';
	}

	static auto write_string(std::ostream &out, const std::string &value) -> void
	{
		out << '"';
		for (const auto character : value) {
			switch (character) {
			case '"':
				out << "\\\"";
				break;
			case '\\':
				out << "\\\\";
				break;
			case '\b':
				out << "\\b";
				break;
			case '\f':
				out << "\\f";
				break;
			case '\n':
				out << "\\n";
				break;
			case '\r':
				out << "\\r";
				break;
			case '\t':
				out << "\\t";
				break;
			default:
				out << character;
				break;
			}
		}
		out << '"';
	}

	Kind kind_{Kind::null_value};
	std::map<std::string, Metadata> object_;
	std::vector<Metadata> array_;
	std::string string_;
	long double number_{0};
	bool bool_{false};
};

} // namespace mtrc::core

#endif
