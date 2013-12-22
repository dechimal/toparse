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

using result = std::tuple<std::vector<std::string>, std::vector<std::tuple<std::string, b::optional<std::string>>>>;
using description = std::vector<option>;

result parse(int argc, char * const argv[], description const &);

struct option {
    option(std::string name, b::optional<char> short_name, arg arg_req);
    option(std::string name, char short_name, arg arg_req);
    option(std::string name, arg arg_req);
private:
    friend result parse(int, char * const *, description const &);
    std::string name;
    b::optional<char> short_name;
    arg arg_req;
};

enum struct arg {
    none,
    mandatory,
    optional,
};

struct invalid_option_name : std::exception {
    invalid_option_name(std::string name) : msg("invalid option name: " + std::move(name)) {}
    char const * what() const noexcept override { return msg.c_str(); }
private:
    std::string msg;
};

struct unrecognized_option : std::exception {
    unrecognized_option(std::string name) : msg("unrecognized option: " + std::move(name)) {}
    char const * what() const noexcept override { return msg.c_str(); }
private:
    std::string msg;
};
struct unnecessary_argument : std::exception {
    unnecessary_argument(std::string name) : msg("unnecessary argument found at option: " + std::move(name)) {}
    char const * what() const noexcept override { return msg.c_str(); }
private:
    std::string msg;
};
struct argument_not_found : std::exception {
    argument_not_found(std::string name) : msg("argument is not found for option: " + std::move(name)) {}
    char const * what() const noexcept override { return msg.c_str(); }
private:
    std::string msg;
};

}

using detail::arg;
using detail::description;
using detail::result;
using detail::parse;
using detail::option;
using detail::invalid_option_name;
using detail::unrecognized_option;
using detail::invalid_option_name;
using detail::unnecessary_argument;
using detail::argument_not_found;

}

#endif
