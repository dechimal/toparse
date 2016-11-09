// Author: dechimal (http://libdechimal.so/)
// distribute under the WTFPL. See file LICENSE.

#include <cstring>
#include <algorithm>
#include <boost/optional/optional.hpp>

#include "toparse.hpp"

namespace toparse { namespace detail {

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

template<typename F>
static void parse_impl(int argc, char const * const argv[], description const & desc, F f);

result parse(int argc, char const * const argv[], description const & desc) {
    std::vector<std::string> pos_params;
    std::vector<std::tuple<option, b::optional<std::string>>> opt_params;

    here::parse_impl(argc, argv, desc, [&] (b::optional<option> opt, b::optional<std::string> param) {
        if (!opt) pos_params.push_back(std::move(*param));
        else opt_params.emplace_back(*opt, std::move(param));
    });

    return std::make_tuple(std::move(pos_params), std::move(opt_params));
}

merged_result parse_merged(int argc, char const * const argv[], description const & desc) {
    merged_result params;
    parse_impl(argc, argv, desc, [&] (b::optional<option> opt, b::optional<std::string> param) {
        params.emplace_back(std::move(opt), std::move(param));
    });
    return params;
}

description description_from_string(char const * opts, char const * long_opts) {
    description desc;
    if (opts) {
        while (char short_name = *opts++) {
            if (!short_name) break;
            auto n = strspn(opts, ":");
            opts += n;
            arg req = static_cast<arg>(std::min((std::size_t)2, n));
            desc.emplace_back(short_name, req);
        }
    }
    if (long_opts) {
        while (char const * long_name = long_opts) {
            auto m = strcspn(long_opts, ",");
            if (!m) break;
            auto p = long_opts += m;
            auto n = 0;
            while (n < 2) {
                if (*--p != ':') break;
                if (p == long_name) throw here::invalid_optstring(long_opts);
                ++n;
            }
            arg req = static_cast<arg>(n);
            desc.emplace_back(std::string(long_name, m - n), req);
            if (!*long_opts++) break;
        }
    }
    return desc;
}

result getopt(int argc, char const * const argv[], char const * opts, char const * long_opts) {
    return parse(argc, argv, description_from_string(opts, long_opts));
}

merged_result getopt_merged(int argc, char const * const argv[], char const * opts, char const * long_opts) {
    return parse_merged(argc, argv, description_from_string(opts, long_opts));
}

template<typename F>
static void parse_impl(int argc, char const * const argv[], description const & desc, F f) {

    auto cont = [&] { return argc > 0; };
    auto next = [&] { --argc; ++argv; };

    while (next(), cont()) {
        char const * str = *argv;
        auto size = strlen(str);
        auto end = str+size;
        auto hyphens = "--";
        if (std::search(str, end, hyphens, hyphens + 2) == str) {
            if (size == 2) {
                while (next(), cont()) f(b::none, std::string(*argv));
                break;
            }
            str += 2;
            auto eq_pos = std::find(str, end, '=');
            auto i = std::find_if(desc.begin(), desc.end(), [&] (option const & opt) {
                return !opt.name().empty() && std::search(str, end, opt.name().begin(), opt.name().end()) == str;
            });
            if (i == desc.end()) throw here::unrecognized_option("--" + std::string(str, eq_pos));
            auto arg_req = i->arg_requirement();
            auto const & long_name = i->name();
            switch (arg_req) {
            case arg::none:
                if (eq_pos != end) throw unnecessary_argument("--" + std::string(str, eq_pos));
                f(*i, b::none);
                break;
            case arg::mandatory:
                if (eq_pos == end) {
                    next();
                    if (!cont()) throw here::argument_not_found("--" + std::string(str));
                    str = *argv;
                } else {
                    str = eq_pos+1;
                }
                f(*i, std::string(str));
                break;
            default:
                auto a = eq_pos == end ? b::none : b::make_optional(std::string(eq_pos+1));
                f(*i, std::move(a));
            }
        } else if (size > 1 && str[0] == '-' && str[1] != '\0') {
            while (++str, *str != '\0') {
                char short_name = *str;
                auto i = std::find_if(desc.begin(), desc.end(), [&] (option const & opt) {
                    return opt.short_name() != '\0' && opt.short_name() == short_name;
                });
                if (i == desc.end()) throw here::unrecognized_option(std::string("-")+*str);
                switch (i->arg_requirement()) {
                case arg::none:
                    f(*i, b::none);
                    break;
                case arg::mandatory:
                    ++str;
                    if (*str == '\0') {
                        next();
                        if (!cont()) throw here::argument_not_found(std::string("-")+*str);
                        str = *argv;
                    }
                    f(*i, std::string(str));
                    goto finish;
                default:
                    ++str;
                    auto a = *str == '\0' ? b::none : b::make_optional(std::string(str));
                    f(*i, std::move(a));
                    goto finish;
                }
            }
        finish:;
        } else {
            f(b::none, std::string(str));
        }
    }
}

invalid_option_name::invalid_option_name(std::string name) : msg("invalid option name: " + std::move(name)) {}
char const * invalid_option_name::what() const noexcept { return msg.c_str(); }

unrecognized_option::unrecognized_option(std::string name) : msg("unrecognized option: " + std::move(name)) {}
char const * unrecognized_option::what() const noexcept { return msg.c_str(); }

unnecessary_argument::unnecessary_argument(std::string name) : msg("unnecessary argument found at option \"" + std::move(name) + '"') {}
char const * unnecessary_argument::what() const noexcept { return msg.c_str(); }

argument_not_found::argument_not_found(std::string name) : msg("no argument option: " + std::move(name)) {}
char const * argument_not_found::what() const noexcept { return msg.c_str(); }

invalid_optstring::invalid_optstring(std::string name) : msg("invalid optstring: " + std::move(name)) {}
char const * invalid_optstring::what() const noexcept { return msg.c_str(); }

}}
