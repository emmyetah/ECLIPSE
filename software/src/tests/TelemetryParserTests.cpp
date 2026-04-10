
#include <QDebug>
#include <vector>
#include <string>


using eclipse::io::parsing::TelemetryParser;

struct ParserTestCase {
    const char* testName;
    std::string input;
    bool expectedAccepted;
    int expectedSampleCount;
};

void RunTelemetryParserTests() {
    TelemetryParser parser;

    std::vector<ParserTestCase> tests = {
        {
            "Valid telemetry line",
            "76048,23.21,29.70,1013.17,635.88,25.08,31.10,0",
            true,
            7
        },
        {
            "Missing final token",
            "76048,23.21,29.70,1013.17,635.88,25.08,31.10",
            false,
            0
        },
        {
            "Too few tokens",
            "76048,23.21,29.70,1013.17",
            false,
            0
        },
        {
            "Invalid timestamp",
            "hello,23.21,29.70,1013.17,635.88,25.08,31.10,0",
            false,
            0
        },
        {
            "Invalid numeric token",
            "76048,23.21,abc,1013.17,635.88,25.08,31.10,0",
            true,
            6
        },
        {
            "Pressure token NA",
            "76048,23.21,29.70,NA,635.88,25.08,31.10,0",
            true,
            6
        },
        {
            "Multiple NA tokens",
            "76048,23.21,NA,1013.17,NA,25.08,31.10,0",
            true,
            5
        },
        {
            "Empty token treated as missing",
            "76048,23.21,,1013.17,635.88,25.08,31.10,0",
            true,
            6
        },
        {
            "All sensor values missing",
            "76048,NA,NA,NA,NA,NA,NA,NA",
            true,
            0
        },
        {
            "Whitespace around values",
            " 76048 , 23.21 , 29.70 , 1013.17 , 635.88 , 25.08 , 31.10 , 0 ",
            true,
            7
        }
    };

    qDebug() << "========== Telemetry Parser Validation Tests ==========";

    int passed = 0;
    int total = static_cast<int>(tests.size());

    for (const auto& test : tests) {
        auto result = parser.parseLine(test.input);

        const bool actualAccepted = result.has_value();
        const int actualSampleCount = result.has_value()
            ? static_cast<int>(result->size())
            : 0;

        const bool pass =
            (actualAccepted == test.expectedAccepted) &&
            (actualSampleCount == test.expectedSampleCount);

        if (pass) {
            ++passed;
        }

        qDebug().noquote()
            << QString("%1 | Expected Accepted: %2 | Actual Accepted: %3 | Expected Samples: %4 | Actual Samples: %5 | Result: %6")
            .arg(test.testName)
            .arg(test.expectedAccepted ? "true" : "false")
            .arg(actualAccepted ? "true" : "false")
            .arg(test.expectedSampleCount)
            .arg(actualSampleCount)
            .arg(pass ? "Pass" : "Fail");
    }

    qDebug() << "======================================================";
    qDebug() << "Passed" << passed << "out of" << total << "tests";
}