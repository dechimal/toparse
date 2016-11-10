// Author: dechimal (http://libdechimal.so/)
// distribute under the WTFPL. See file LICENSE.

#include <cstring>
#include <algorithm>
#include <boost/optional/optional.hpp>

#include "toparse.hpp"

namespace toparse { namespace detail {

template<typename Callback> static void parse_impl(int argc, char const * const argv[], options const & opts, Callback callback);

option::option(std::string name, char short_name, arg arg_req)
    : name_(name.find('=') != name.npos || (name.empty() && short_name == '\0') ? (throw invalid_option_name(std::move(name))), "" : std::move(name)),
      short_name_(short_name),
      arg_req(arg_req)
{}
option::option(char short_name, arg arg_req)
    : option("", short_name, arg_req)
{}
option::option(std::string name, arg arg_req)
    : option(name, '\0', arg_req)
{}

bool operator==(option const & a, option const & b) noexcept {
    return a.name() == b.name() && a.short_name() == b.short_name() && a.arg_requirement() == b.arg_requirement();
}

result parse(int argc, char const * const argv[], options const & opts) {
    std::vector<std::string> pos_params;
    std::vector<std::tuple<option, b::optional<std::string>>> opt_params;

    here::parse_impl(argc, argv, opts, [&] (b::optional<option> opt, b::optional<std::string> param) {
        if (!opt) pos_params.push_back(std::move(*param));
        else opt_params.emplace_back(*opt, std::move(param));
    });

    return std::make_tuple(std::move(pos_params), std::move(opt_params));
}

unparted_result parse_unparted(int argc, char const * const argv[], options const & opts) {
    unparted_result params;
    here::parse_impl(argc, argv, opts, [&] (b::optional<option> opt, b::optional<std::string> param) {
        params.emplace_back(std::move(opt), std::move(param));
    });
    return params;
}

options options_from_optstring(char const * optstr, char const * long_optstr) {
    options opts;
    if (optstr) {
        while (char short_name = *optstr++) {
            if (!short_name) break;
            auto n = std::strspn(optstr, ":");
            optstr += n;
            arg req = static_cast<arg>(std::min((std::size_t)2, n));
            opts.emplace_back(short_name, req);
        }
    }
    if (long_optstr) {
        while (char const * long_name = long_optstr) {
            auto m = std::strcspn(long_optstr, ",");
            if (!m) break;
            auto p = long_optstr += m;
            auto n = 0;
            while (n < 2) {
                if (*--p != ':') break;
                if (p == long_name) throw here::invalid_optstring(long_optstr);
                ++n;
            }
            arg req = static_cast<arg>(n);
            opts.emplace_back(std::string(long_name, m - n), req);
            if (!*long_optstr++) break;
        }
    }
    return opts;
}

result getopt(int argc, char const * const argv[], char const * optstr, char const * long_optstr) {
    return here::parse(argc, argv, here::options_from_optstring(optstr, long_optstr));
}

unparted_result getopt_unparted(int argc, char const * const argv[], char const * optstr, char const * long_optstr) {
    return here::parse_unparted(argc, argv, here::options_from_optstring(optstr, long_optstr));
}

template<typename Callback>
void parse_impl(int argc, char const * const argv[], options const & opts, Callback callback) {

    auto next = [&] { ++argv; return --argc > 0; };

    while (next()) {
        char const * str = *argv;
        auto hyphens = std::strspn(str, "-");
        if (hyphens >= 2) {
            if (!*(str += 2)) goto rest;
            auto n = std::strcspn(str, "=");
            if (!n) throw here::unrecognized_option("--=");
            auto eq_pos = str + n;
            auto i = std::find_if(opts.begin(), opts.end(), [&] (option const & opt) {
                return !std::strncmp(opt.name().c_str(), str, n);
            });
            if (i == opts.end()) throw here::unrecognized_option("--" + std::string(str, eq_pos));
            auto arg_req = i->arg_requirement();
            b::optional<std::string> a;
            bool cont = *eq_pos;
            if (arg_req == arg::none) {
                if (cont) throw here::unnecessary_argument("--" + std::string(str, eq_pos));
            } else if (arg_req == arg::mandatory) {
                if (!cont) {
                    if (!next()) throw here::argument_not_found("--" + std::string(str));
                    a = *argv;
                } else {
                    a = eq_pos+1;
                }
            } else {
                if (cont) a = eq_pos+1;
            }
            callback(*i, std::move(a));
        } else if (hyphens && str[1]) {
            while (char short_name = *++str) {
                auto i = std::find_if(opts.begin(), opts.end(), [&] (option const & opt) {
                    return opt.short_name() == short_name;
                });
                if (i == opts.end()) throw here::unrecognized_option({ '-', short_name });
                auto arg_req = i->arg_requirement();
                b::optional<std::string> a;
                if (arg_req == arg::mandatory) {
                    if (*++str == '\0') {
                        if (!next()) throw here::argument_not_found({ '-', short_name });
                        str = *argv;
                    }
                    a = str;
                } else if (arg_req == arg::optional) {
                    if (*++str != '\0') a = str;
                }
                callback(*i, std::move(a));
                if (arg_req != arg::none) break;
            }
        } else {
            callback(b::none, std::string(str));
        }
    }
    return;
rest:
    while (next()) callback(b::none, std::string(*argv));
}

error_base::error_base(std::string msg) : msg(msg) {}
char const * error_base::what() const noexcept { return msg.c_str(); }
invalid_option_name::invalid_option_name(std::string name) : error_base("invalid option name: " + std::move(name)) {}
unrecognized_option::unrecognized_option(std::string name) : error_base("unrecognized option: " + std::move(name)) {}
unnecessary_argument::unnecessary_argument(std::string name) : error_base("unnecessary argument found at option \"" + std::move(name) + '"') {}
argument_not_found::argument_not_found(std::string name) : error_base("no argument option: " + std::move(name)) {}
invalid_optstring::invalid_optstring(std::string name) : error_base("invalid optstring: " + std::move(name)) {}

}}
