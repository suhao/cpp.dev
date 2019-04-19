/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 

#ifndef TEST_TEST_BOOST_INCLUDE_H_ 
#define TEST_TEST_BOOST_INCLUDE_H_ 

#include "boost/timer.hpp"
#include "boost/progress.hpp"
#include "boost/lambda/lambda.hpp"
#include "boost/static_assert.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"

#define BOOST_DATE_TIME_POSIX_TIME_STD_CONFIG
#include "boost/date_time/posix_time/posix_time.hpp"


#include <iostream>
#include <algorithm>

namespace boost {

int test_lambda() {
    typedef std::istream_iterator<int> input;
    std::for_each(input(std::cin), input(), std::cout << (boost::lambda::_1 * 3) << "\n");
    return 0;
}

void test_timer() {
    timer ti;
    std::cout << ti.elapsed_max() << std::endl;
    std::cout << ti.elapsed_min() << std::endl;
    std::cout << ti.elapsed() << std::endl;
}

template<int N = 2>
class ProgressTimer : boost::timer {
public:
    ProgressTimer(std::ostream& os = std::cout) : _stream(os) {
        BOOST_STATIC_ASSERT(N >= 0 && N <= 10);
    }

    virtual ~ProgressTimer() {
        try {
            auto flags = _stream.setf(std::istream::fixed, std::istream::floatfield);
            auto size = _stream.precision(N);
            _stream << elapsed() << " s\n" << std::endl;
            _stream.precision(size);
            _stream.flags(flags);
        } catch (...) {

        }
    }

private:
    std::ostream& _stream;
};

template<>
class ProgressTimer<2>: public boost::progress_timer {};


void test_progress_timer() {
    progress_timer t;
    std::cout << t.elapsed() << std::endl;
    ProgressTimer<10> t10;
    ProgressTimer<> t2;
    // ProgressTimer<111> t111;
}

void test_progress_display() {
    std::vector<std::string> v(100);
    progress_display displayer(v.size());
    for (auto& x : v) {
        ++displayer;
        Sleep(10);
    }
}

void test_date_time() {
    using namespace gregorian;
    date d1;
    date d2(2018, 3, 25);
    assert(d1 == date(not_a_date_time));
    std::cout << day_clock::local_day() << std::endl;
    std::cout << day_clock::universal_day() << std::endl;
    date d3 = from_string("2018-03-25");
    date d4 = from_undelimited_string("20180325");
    assert(d3 == d4);
    try {
        date d1(1399, 12, 1);
        date d2(10000, 1, 1);
        date d3(2010, 2, 29);
    } catch (...) {

    }

    date_period dp1(d3, days(20));
    std::cout << dp1 << std::endl;


    std::cout << gregorian_calendar::is_leap_year(2010) << std::endl;

    date d(2018, 3, 26);
    auto start = date(d.year(), d.month(), 1);
    auto end = d.end_of_month();
    for (day_iterator it(start); it != end; ++it) {
        std::cout << *it << " " << it->day_of_week() << std::endl;
    }
 
}

void test_time() {
    using namespace posix_time;
    time_duration td1(1, 10, 30, 1000);
    time_duration td2 = hours(2) + seconds(20);

}



void test() {
    test_time();
}

}


#endif  // !#define (TEST_TEST_BOOST_INCLUDE_H_ )

