// Author: dechimal (http://libdechimal.so/)
// distribute under the WTFPL. See file LICENSE.

#if !defined TOPARSE_HPP_INCLUDED_
#define      TOPARSE_HPP_INCLUDED_

#include <tuple>
#include <vector>
#include <exception>
#include <boost/optional/optional.hpp>

// a. -f --flag
// b. -f1 --flag=1
// c. -f 1 --flag 1

// none: a
// mandatory: b, c
// optional: a, b

//           none     mandatory optional
// -f        nothing  N/A       nothing
// -f1       N/A      '1'       '1'
// -f ''     N/A      ''        N/A
// -f 1      N/A      '1'       N/A
// --flag    nothing  N/A       nothing
// --flag=   N/A      ''        ''
// --flag=1  N/A      '1'       '1'
// --flag '' N/A      ''        N/A
// --flag 1  N/A      '1'       N/A

namespace toparse {
namespace here = toparse;
namespace detail {
namespace here = detail;

namespace b = boost;

enum struct arg;
struct option;
struct error_base;
struct invalid_option_name;
struct unnecessary_argument;
struct argument_not_found;
struct invalid_optstring;

using options = std::vector<option>;
using pos_args = std::vector<std::string>;
using opt_args = std::vector<std::tuple<option, b::optional<std::string>>>;
using result = std::tuple<pos_args, opt_args>;
using unparted_result =
    std::vector<std::tuple<b::optional<option>, b::optional<std::string>>>;

result parse(int argc, char const * const argv[], options const & opts);
unparted_result parse_unparted(int argc, char const * const argv[], options const & opts);
result getopt(int argc, char const * const argv[], char const * optstr, char const * long_optstr);
unparted_result getopt_unparted(int argc, char const * const argv[], char const * optstr, char const * long_optstr);
options options_from_optstring(char const * optstr, char const * long_optstr);
bool operator==(option const & a, option const & b) noexcept;
inline bool operator!=(option const & a, option const & b) noexcept;

struct option {
    option(std::string name, char short_name, arg arg_req);
    option(char short_name, arg arg_req);
    option(std::string name, arg arg_req);
    std::string const & name() const noexcept { return name_; }
    char short_name() const noexcept { return short_name_; }
    arg arg_requirement() const noexcept { return arg_req; }
private:
    std::string name_;
    char short_name_;
    arg arg_req;
};

inline bool operator!=(option const & a, option const & b) noexcept {
    return !(a == b);
}

enum struct arg {
    none,
    mandatory,
    optional,
};

struct error_base : std::exception {
    error_base(std::string msg);
    char const * what() const noexcept override;
private:
    std::string msg;
};

struct invalid_option_name : error_base { invalid_option_name(std::string name); };
struct unrecognized_option : error_base { unrecognized_option(std::string name); };
struct unnecessary_argument : error_base { unnecessary_argument(std::string name); };
struct argument_not_found : error_base { argument_not_found(std::string name); };
struct invalid_optstring : error_base { invalid_optstring(std::string name); };

}

using detail::arg;
using detail::options;
using detail::result;
using detail::unparted_result;
using detail::parse;
using detail::option;
using detail::getopt;
using detail::pos_args;
using detail::opt_args;
using detail::parse_unparted;
using detail::getopt_unparted;
using detail::options_from_optstring;
using detail::error_base;
using detail::invalid_option_name;
using detail::unrecognized_option;
using detail::unnecessary_argument;
using detail::argument_not_found;
using detail::invalid_optstring;

}

#endif
