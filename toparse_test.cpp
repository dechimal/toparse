// Author: dechimal (http://libdechimal.so/)
// distribute under the WTFPL. See file LICENSE.

#include <tuple>
#include <iostream>
#include <vector>
#include <iterator>
#include "boost/optional/optional.hpp"
#include "toparse.hpp"

using toparse::arg;
using toparse::options;
using toparse::option;
using toparse::pos_args;
using toparse::opt_args;
using toparse::unparted_result;
namespace b = boost;

void test(std::vector<char const *> const & cmdline, options const & opts,
          pos_args const & expected_pos_result,
          opt_args const & expected_opt_result,
          unparted_result const & expected_unparted_result) {
    auto r = toparse::parse(cmdline.size(), cmdline.data(), opts);
    auto const & pos_result = std::get<0>(r);
    auto const & opt_result = std::get<1>(r);
    assert(pos_result == expected_pos_result);
    assert(opt_result == expected_opt_result);

    auto unparted_result = toparse::parse_unparted(cmdline.size(), cmdline.data(), opts);
    assert(unparted_result == expected_unparted_result);
}

int main() {
    std::vector<char const *> cmdline {
        "argv0", "--flag", "-f", "value1", "--mandatory=1", "-m1", "value2", "-m", "1", "--mandatory", "1",
        "-m", "", "-s1", "value3", "--mandatory", "", "--mandatory=", "--optional=1", "-o1", "value4", "--optional",
        "-o", "value5", "--optional=", "-ffm1", "--", "-f1", "--mandatory", "-s1",
    };
    options opts {
        { "flag", 'f', arg::none },
        { "mandatory", 'm', arg::mandatory },
        { "optional", 'o', arg::optional },
        { 's', arg::mandatory },
    };

    pos_args expected_pos_result {
        "value1", "value2", "value3", "value4", "value5", "-f1", "--mandatory", "-s1",
    };
    opt_args expected_opt_result {
        std::make_tuple(opts[0], b::none),
        std::make_tuple(opts[0], b::none),
        std::make_tuple(opts[1], std::string("1")),
        std::make_tuple(opts[1], std::string("1")),
        std::make_tuple(opts[1], std::string("1")),
        std::make_tuple(opts[1], std::string("1")),
        std::make_tuple(opts[1], std::string("")),
        std::make_tuple(opts[3], std::string("1")),
        std::make_tuple(opts[1], std::string("")),
        std::make_tuple(opts[1], std::string("")),
        std::make_tuple(opts[2], std::string("1")),
        std::make_tuple(opts[2], std::string("1")),
        std::make_tuple(opts[2], b::none),
        std::make_tuple(opts[2], b::none),
        std::make_tuple(opts[2], std::string("")),
        std::make_tuple(opts[0], b::none),
        std::make_tuple(opts[0], b::none),
        std::make_tuple(opts[1], std::string("1"))
    };
    unparted_result expected_unparted_result {
        std::make_tuple(opts[0], b::none),
        std::make_tuple(opts[0], b::none),
        std::make_tuple(b::none, std::string("value1")),
        std::make_tuple(opts[1], std::string("1")),
        std::make_tuple(opts[1], std::string("1")),
        std::make_tuple(b::none, std::string("value2")),
        std::make_tuple(opts[1], std::string("1")),
        std::make_tuple(opts[1], std::string("1")),
        std::make_tuple(opts[1], std::string("")),
        std::make_tuple(opts[3], std::string("1")),
        std::make_tuple(b::none, std::string("value3")),
        std::make_tuple(opts[1], std::string("")),
        std::make_tuple(opts[1], std::string("")),
        std::make_tuple(opts[2], std::string("1")),
        std::make_tuple(opts[2], std::string("1")),
        std::make_tuple(b::none, std::string("value4")),
        std::make_tuple(opts[2], b::none),
        std::make_tuple(opts[2], b::none),
        std::make_tuple(b::none, std::string("value5")),
        std::make_tuple(opts[2], std::string("")),
        std::make_tuple(opts[0], b::none),
        std::make_tuple(opts[0], b::none),
        std::make_tuple(opts[1], std::string("1")),
        std::make_tuple(b::none, std::string("-f1")),
        std::make_tuple(b::none, std::string("--mandatory")),
        std::make_tuple(b::none, std::string("-s1")),
    };
    ::test(cmdline, opts, expected_pos_result, expected_opt_result, expected_unparted_result);
    try {
        ::test({ "argv0", "--foo" }, { { "flag", arg::none } },
               {}, {}, {});
        assert(false);
    } catch (toparse::unrecognized_option const &) {}
    try {
        ::test({ "argv0", "--foo=1" }, { { "foo", arg::none } },
               {}, {}, {});
        assert(false);
    } catch (toparse::unnecessary_argument const &) {}
    try {
        ::test({ "argv0", "--foo" }, { { "foo", arg::mandatory } },
               {}, {}, {});
        assert(false);
    } catch (toparse::argument_not_found const &) {}
    try {
        ::test({ "argv0", "--foo=" }, { { "foo=", arg::none } },
        {}, {}, {});
        assert(false);
    } catch (toparse::invalid_option_name const &) {}
    {
        auto opts = toparse::options_from_optstring("fm:o::a:::b", "flags,mandatory:,optional::,args:::");
        auto expected_opts = options{
            { 'f', arg::none },
            { 'm', arg::mandatory },
            { 'o', arg::optional },
            { 'a', arg::optional },
            { 'b', arg::none },
            { std::string("flags"), arg::none },
            { std::string("mandatory"), arg::mandatory },
            { std::string("optional"), arg::optional },
            { std::string("args:"), arg::optional },
        };
        assert(opts == expected_opts);
    }
}
