#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <iterator>
#include <iostream>
#include <fstream>
#include <vector>

class CSVReader {
private:
    std::string filename_;
    std::string delimiter_;

public:
    CSVReader(const std::string& filename, const std::string& delimiter) :
            filename_(filename), delimiter_(delimiter) {}

    std::vector<std::vector<std::string>> getData() const {

        std::ifstream file(filename_);

        if(!file) {
            std::cerr << "File is failed to open" << std::endl;
        }

        std::string line;
        std::vector<std::vector<std::string>> data_list;

        while(std::getline(file, line)) {
            std::vector<std::string> fields;
            boost::algorithm::split(fields, line, boost::is_any_of(delimiter_));
            data_list.push_back(fields);
        }

        file.close();

        return data_list;
    }
};

struct Object {

    bool        status;
    uint64_t    id;
    uint32_t    number;
    std::time_t date;
    std::string partner;

    Object() = default;

    explicit Object(const std::vector<std::string>& fields) {
        this -> id         = boost::lexical_cast<uint32_t>(fields[0]);
        this -> partner    = fields[1];
        this -> number     = boost::lexical_cast<uint64_t>(fields[2]);
        this -> date       = boost::lexical_cast<std::time_t>(fields[3]);
        this -> status     = boost::lexical_cast<bool>(fields[4]);
    }

    friend std::ostream& operator << (std::ostream& os, Object& object) {

        os << "[\n"
           << " Id: "       << object.id       << ",\n"
           << " Partner: "  << object.partner  << ",\n"
           << " Number: "   << object.number   << ",\n"
           << " Date: "     << object.date     << ",\n"
           << " Status: "   << object.status   << " \n"
           << "]";

        return os;
    }

};

namespace Tags {
    struct IdTag {};
    struct DateTag {};
};

using namespace Tags;

typedef boost::multi_index_container<
    Object,
    boost::multi_index::indexed_by<
        boost::multi_index::ordered_non_unique<
            boost::multi_index::tag<IdTag>,
            boost::multi_index::member<Object, uint64_t, &Object::id>
        >,
        boost::multi_index::ordered_non_unique<
            boost::multi_index::tag<DateTag>,
            boost::multi_index::member<Object, std::time_t, &Object::date>
        >
    >
> ObjectsContainer;

// start of 2019 (01 01 2019 00:00) in Unix time(GMT 3)
const std::time_t START_OF_2019_GMT_3 = static_cast<std::time_t>(1546290000);

signed main() {

    CSVReader reader("../data.csv", ";");

    const auto data_list = reader.getData();

    ObjectsContainer v{};
    for(const auto& list : data_list) {
        v.insert(Object(list));
    }

    const auto max_by_id_iterator = std::prev(v.get<IdTag>().end());
    auto object = (*max_by_id_iterator);

    std::cout << "Max by Id:\n" << object << std::endl;

    const auto id_100_iterator = v.get<IdTag>().find(100);
    object = (*id_100_iterator);

    std::cout << std::endl << "Id == 100\n" << object << std::endl;

    auto in_2019_iterator = v.get<DateTag>().lower_bound(START_OF_2019_GMT_3);

    std::cout << "Number of lines in 2019 : "
              << std::distance(in_2019_iterator, v.get<DateTag>().end())
              << std::endl;
    for(auto iterator = in_2019_iterator; iterator != v.get<DateTag>().end(); iterator = std::next(iterator)) {
        object = (*iterator);
        std::cout << object << std::endl;
    }

    return 0;
}