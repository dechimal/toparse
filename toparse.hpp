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
struct invalid_option_name;
struct unnecessary_argument;
struct argument_not_found;
struct invalid_optstring;

using result =
    std::tuple<
        std::vector<std::string>,
        std::vector<std::tuple<option, b::optional<std::string>>>
    >;
using merged_result =
    std::vector<std::tuple<b::optional<option>, b::optional<std::string>>>;
using description = std::vector<option>;

result parse(int argc, char const * const argv[], description const &);
result getopt(int argc, char const * const argv[], char const * opts, char const * long_opts);
merged_result parse_merged(int argc, char const * const argv[], description const &);
merged_result getopt_merged(int argc, char const * const argv[], char const * opts, char const * long_opts);
description description_from_string(char const * opts, char const * long_opts);
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
    friend struct option_access;
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

struct invalid_option_name : std::exception {
    invalid_option_name(std::string name);
    char const * what() const noexcept override;
private:
    std::string msg;
};

struct unrecognized_option : std::exception {
    unrecognized_option(std::string name);
    char const * what() const noexcept override;
private:
    std::string msg;
};
struct unnecessary_argument : std::exception {
    unnecessary_argument(std::string name);
    char const * what() const noexcept override;
private:
    std::string msg;
};
struct argument_not_found : std::exception {
    argument_not_found(std::string name);
    char const * what() const noexcept override;
private:
    std::string msg;
};

struct invalid_optstring : std::exception {
    invalid_optstring(std::string name);
    char const * what() const noexcept override;
private:
    std::string msg;
};

}

using detail::arg;
using detail::description;
using detail::result;
using detail::merged_result;
using detail::parse;
using detail::option;
using detail::getopt;
using detail::parse_merged;
using detail::getopt_merged;
using detail::description_from_string;
using detail::invalid_option_name;
using detail::unrecognized_option;
using detail::invalid_option_name;
using detail::unnecessary_argument;
using detail::argument_not_found;
using detail::invalid_optstring;

}

#endif
