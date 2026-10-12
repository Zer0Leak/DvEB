
#include "bigdata/tree_rpc.hh"

namespace bigdata {

const std::string & TreeRpc::to_string(Method method) {
    static const std::string mname[] =  {
        "kInvalid",
        "kAllMethods",
        "kAllAnswers",
        "kAll",
        "kAck",
        "kAck2",
        "kAckF",
        "kNack",

        "kId",
        "kInsert",
        "kRemove",
        "kSearch",
        "kSuccessor",
        "kPredecessor",
        "kMin",
        "kMax",
        "kUniverse",
        "kVebexpanded",

        "kAnswerId",
        "kAnswerInsert",
        "kAnswerRemove",
        "kAnswerSearch",
        "kAnswerSuccessor",
        "kAnswerPredecessor",
        "kAnswerMin",
        "kAnswerMax",
        "kAnswerUniverse",
        "kAnswerVeb_expanded",
        "kAnswerAvailable",
        "kAnswerAlloc",

        "kAvailable",
        "kAlloc",

        "kControlForceStop",
        "kAnswerAllocCheater"

    };

    return mname[method];
}

std::ostream& operator <<(std::ostream& stream, TreeRpc::Method method) {


    stream << TreeRpc::to_string(method);

    return stream;
}

}
