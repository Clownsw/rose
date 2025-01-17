static const char *purpose = "generate low-level C from a binary";
static const char *description =
    "This tool generates a low-level C program with the same semantics as the specified binary specimen. It is "
    "intended as a way to test whether the algorithms used by ROSE source analysis tools would be able to analyze "
    "the binary after most types and variables are erased.";

#include <rose.h>
#include <batSupport.h>

#include <Rose/BinaryAnalysis/ToSource.h>
#include <Rose/Diagnostics.h>
#include <Rose/BinaryAnalysis/Partitioner2/Engine.h>
#include <Rose/BinaryAnalysis/Partitioner2/Partitioner.h>
#include <Sawyer/CommandLine.h>

using namespace Sawyer::Message::Common;
using namespace Rose::BinaryAnalysis;
namespace P2 = Rose::BinaryAnalysis::Partitioner2;

Sawyer::Message::Facility mlog;

// Global settings adjusted from the command-line
struct Settings {
    BinaryToSource::Settings generator;
};

static std::vector<std::string>
parseCommandLine(int argc, char *argv[], P2::Engine &engine, Settings &settings) {
    using namespace Sawyer::CommandLine;

    Parser parser = engine.commandLineParser(purpose, description);
    parser.errorStream(mlog[FATAL]);
    SwitchGroup generator = BinaryToSource::commandLineSwitches(settings.generator);
    return parser.with(generator).parse(argc, argv).apply().unreachedArgs();
}

int
main(int argc, char *argv[]) {
    ROSE_INITIALIZE;
    Rose::Diagnostics::initAndRegister(&mlog, "tool");
    mlog.comment("binary to source");
    Bat::checkRoseVersionNumber(MINIMUM_ROSE_LIBRARY_VERSION, mlog[FATAL]);
    Bat::registerSelfTests();

    // Parse the command-line switches
    Settings settings;
    P2::Engine *engine = P2::Engine::instance();
    std::vector<std::string> args = parseCommandLine(argc, argv, *engine, settings);
    if (args.empty()) {
        mlog[FATAL] <<"no binary specimen specified; see --help\n";
        exit(1);
    }

    // Parse the binary specimen. We're not actually adding it to the AST.
    P2::Partitioner::Ptr binary = engine->partition(args);

    // Process the binary to add its instructions to the source template
    BinaryToSource(settings.generator).generateSource(binary, std::cout);

    delete engine;
}
