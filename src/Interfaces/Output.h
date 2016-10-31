#ifndef OUTPUT
#define OUTPUT

#include <memory>
#include <string>
#include <vector>

namespace TCorrPT {

class Output
{
public:
    ///
    /// \brief Output constructor
    /// \param database
    ///
    Output( );

    auto toCSV ( ) -> void;


private:
    struct Impl;
    std::shared_ptr<Impl> pimpl;

//    auto setHeader(std::vector<string> substanceSymbols, map<int, string> thermoProperties, map<string, string> units ) -> void;


};

}

#endif // OUTPUT

