#include <string>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <chrono>

#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include "thrift/gen-cpp/test_types.h"
#include "thrift/gen-cpp/test_constants.h"

#include "protobuf/test.pb.h"

const size_t      kItegersCount = 1000;
const size_t      kStringsCount = 100;
const int64_t     kIntegerValue = 26354;
const std::string kStringValue  = "shgfkghsdfjhgsfjhfgjhfgjsffghgsfdhgsfdfkdjhfioukjhkfdljgdfkgvjafdhasgdfwurtjkghfsdjkfg";

void
thrift_serialization_test(size_t iterations)
{
    using apache::thrift::transport::TMemoryBuffer;
    using apache::thrift::protocol::TBinaryProtocol;
    
    using namespace thrift_test;

    boost::shared_ptr<TMemoryBuffer>   buffer1(new TMemoryBuffer());
    boost::shared_ptr<TBinaryProtocol> protocol1(new TBinaryProtocol(buffer1));

    Record r1;

    for (size_t i = 0; i < kItegersCount; i++) {
        r1.ids.push_back(kIntegerValue);
    }

    for (size_t i = 0; i < kStringsCount; i++) {
        r1.strings.push_back(kStringValue);
    }

    std::string serialized;

    r1.write(protocol1.get());
    serialized = buffer1->getBufferAsString();

    // check if we can deserialize back
    boost::shared_ptr<TMemoryBuffer>   buffer2(new TMemoryBuffer());
    boost::shared_ptr<TBinaryProtocol> protocol2(new TBinaryProtocol(buffer2));

    Record r2;

    buffer2->resetBuffer((uint8_t*)serialized.data(), serialized.length());
    r2.read(protocol2.get());

    if (r1 != r2) {
        throw std::logic_error("thrift's case: invariant failed");
    }

    std::cout << "thrift: size = " << serialized.size() << " bytes" << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < iterations; i++) {
        buffer1->resetBuffer();
        r1.write(protocol1.get());
        serialized = buffer1->getBufferAsString();
        buffer2->resetBuffer((uint8_t*)serialized.data(), serialized.length());
        r2.read(protocol2.get());
    }
    auto finish = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();

    std::cout << "thrift: time = " << duration << " milliseconds" << std::endl << std::endl;
}

void
protobuf_serialization_test(size_t iterations)
{
    using namespace protobuf_test;

    Record r1;

    for (size_t i = 0; i < kItegersCount; i++) {
        r1.add_ids(kIntegerValue);
    }

    for (size_t i = 0; i < kStringsCount; i++) {
        r1.add_strings(kStringValue);
    }

    std::string serialized;

    r1.SerializeToString(&serialized);

    // check if we can deserialize back
    Record r2;
    bool ok = r2.ParseFromString(serialized);
    if (!ok /*|| r2 != r1*/) {
        throw std::logic_error("protobuf's case: invariant failed");
    }

    std::cout << "protobuf: size = " << serialized.size() << " bytes" << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < iterations; i++) {
        serialized.clear();
        r1.SerializeToString(&serialized);
        r2.ParseFromString(serialized);
    }
    auto finish = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();

    std::cout << "protobuf: time = " << duration << " milliseconds" << std::endl << std::endl;
}

int
main(int argc, char **argv)
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " iterations" << std::endl;
        exit(0);
    }

    auto iterations = boost::lexical_cast<size_t>(argv[1]);

    std::cout << "performing " << iterations << " iterations" << std::endl << std::endl;

    /*std::cout << "total size: " << sizeof(kIntegerValue) * kItegersCount + kStringValue.size() * kStringsCount << std::endl;*/

    thrift_serialization_test(iterations);
    protobuf_serialization_test(iterations);

    return 0;
}
