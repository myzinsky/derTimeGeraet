#include "dertimegeraet.h"
#include <QApplication>

#include <gtest/gtest.h>
#include "regressionTests.h"

int main(int argc, char *argv[])
{
    std::string prefix = "--gtest";
    if (argc > 1 && std::string(argv[1]).rfind(prefix, 0) == 0) {
        testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    } else {
        // Run normal Programm
        QApplication a(argc, argv);
        derTimeGeraet w;
        w.show();
        return a.exec();
    }
}
