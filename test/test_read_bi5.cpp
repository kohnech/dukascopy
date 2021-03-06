/*
Copyright 2013 Michael O'Keeffe (a.k.a. ninety47).

This file is part of ninety47 Dukascopy toolbox.

The "ninety47 Dukascopy toolbox" is free software: you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation, either version 3 of the License,
or any later version.

"ninety47 Dukascopy toolbox" is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along with
"ninety47 Dukascopy toolbox".  If not, see <http://www.gnu.org/licenses/>.
*/

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem.hpp>
#include <ninety47/dukascopy.h>
#include <ninety47/dukascopy/defs.h>
#include <cstring>
#include <iostream>
#include <fstream>

#define STRINGIZE(x) #x
#define STRINGIZE_VALUE_OF(x) STRINGIZE(x)

namespace fs = boost::filesystem;
namespace pt = boost::posix_time;
namespace gr = boost::gregorian;

void print_help_message() {
    std::cout << "./test_read_bi5 [ASSET] [DATE: YYYY/MM/DD]\n"
            << "\t Example: ./test_read_bi5 GBPJPY 2012/11/03\n";
}

struct Date {
    int year{0}, month{0}, day{0}, hour{0};
};

Date parse_date(std::string datestr) {
    Date date;
    size_t pos = 0;
    std::string delimiter = "/";
    pos = datestr.find(delimiter);
    std::string year_str = datestr.substr(0, pos);
    datestr.erase(0, pos + delimiter.length());

    pos = datestr.find(delimiter);
    std::string month_str = datestr.substr(0, pos);
    datestr.erase(0, pos + delimiter.length());

    std::string day_str = datestr;

    date.year = std::stoi(year_str);
    date.month = std::stoi(month_str);
    date.day = std::stoi(day_str);

    return date;
}

int read_file(std::string filename, std::string out_file, Date date) {
    unsigned char *buffer;
    fs::path p(filename);
    size_t buffer_size;
    if (fs::exists(p) && fs::is_regular(p)) {
        buffer_size = fs::file_size(p);
        buffer = new unsigned char[ buffer_size ];
    } else {
        std::cerr << "Error: couldn't access the data file. |"
                  << filename << "|" <<  '\n';
        return EXIT_FAILURE;
    }

    if (fs::is_empty(p)) {
        std::cerr << "File " << filename << " is empty!" << '\n';
        return EXIT_FAILURE;
    }

    std::ofstream fout;
    fout.open(out_file);

    std::ifstream fin;
    fin.open(filename, std::ifstream::binary);
    fin.read(reinterpret_cast<char*>(buffer), buffer_size);
    fin.close();

    pt::ptime epoch(gr::date(date.year, date.month, date.day), pt::hours(date.hour));
    size_t raw_size = 0;
    n47::tick_data *data = n47::read_bi5(
            buffer, buffer_size, epoch, PV_YEN_PAIR, &raw_size);
    n47::tick_data_iterator iter;

    if (data == 0) {
        std::cerr << "Failure: Failed to load the data!" << '\n';
        return 0;
    }

    if (data->size() != (raw_size / n47::ROW_SIZE)) {
        std::cerr << "Failure: Loaded " << data->size()
                  << " ticks but file size indicates we should have loaded "
                  << (raw_size / n47::ROW_SIZE) << '\n';
        return 0;
    }

    // std::cout << "time, bid, bid_vol, ask, ask_vol" << '\n';
    fout << "time, bid, bid_vol, ask, ask_vol" << '\n';
    int counter = 0;
    for (iter = data->begin(); iter != data->end(); iter++) {
        // std::cout << ((*iter)->epoch + (*iter)->td) << ", "
        //           << (*iter)->bid << ", " << (*iter)->bidv << ", "
        //           << (*iter)->ask << ", " << (*iter)->askv << '\n';
        fout << ((*iter)->epoch + (*iter)->td) << ", "
                  << (*iter)->bid << ", " << (*iter)->bidv << ", "
                  << (*iter)->ask << ", " << (*iter)->askv << '\n';
        counter++;
    }

    std::cout << ".end." << "\n\n"
          << "From " << buffer_size << " bytes we read " << counter
          << " records." << '\n'
          << raw_size << " / " << n47::ROW_SIZE << " = "
          << (raw_size / n47::ROW_SIZE) << '\n';

    delete data;
    delete[] buffer;
    fout.close();

    return EXIT_SUCCESS;
}

int main(int argc, char* argv[]) {
    // Check input
    if (argc != 3) {
        print_help_message();
        return EXIT_FAILURE;
    }
      

    std::string test_data_prefix = "../test/data/compressed/";
    std::string test_data_asset = argv[1];
    std::string test_data_date = argv[2];
    std::string test_data_file = "01h_ticks.bi5";

    std::array<std::string, 24> all_files = {
        "00h_ticks.bi5",
        "01h_ticks.bi5",
        "02h_ticks.bi5",
        "03h_ticks.bi5",
        "04h_ticks.bi5",
        "05h_ticks.bi5",
        "06h_ticks.bi5",
        "07h_ticks.bi5",
        "08h_ticks.bi5",
        "09h_ticks.bi5",
        "10h_ticks.bi5",
        "11h_ticks.bi5",
        "12h_ticks.bi5",
        "13h_ticks.bi5",
        "14h_ticks.bi5",
        "15h_ticks.bi5",
        "16h_ticks.bi5",
        "17h_ticks.bi5",
        "18h_ticks.bi5",
        "19h_ticks.bi5",
        "20h_ticks.bi5",
        "21h_ticks.bi5",
        "22h_ticks.bi5",
        "23h_ticks.bi5"
    };

    int hour = 1;
    for (auto file : all_files) {
        std::string filename = test_data_prefix + test_data_asset + "/" + test_data_date + "/" + file;
        std::cout << "Reading data from file " << filename << '\n';
        Date date = parse_date(test_data_date);      
        date.hour = hour;        
        const std::string out_file = "output" + file + ".csv";
        read_file(filename, out_file, date);
        std::cout << "Data saved to file " << out_file << '\n';
        ++hour;
    }

    return EXIT_SUCCESS;
}
