#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "../argh.h"

namespace test
{		
	TEST_CLASS(ArghTest)
	{
	public:
		
    TEST_METHOD(LoadFileBad)
    {
      Argh argh;
      argh.load("badfile");
    }

    TEST_METHOD(LoadFile)
    {
      Argh argh;
      int i1;
      argh.addOption<int>(i1, 789, "--intvalue", "Integer value");
      argh.load("../argh.opts");
      Assert::IsTrue(i1 == 123);
    }

    TEST_METHOD(BadArgs)
    {
      Argh argh;
      int i1;
      argh.addOption<int>(i1, 789, "--intvalue", "Integer value");
      const int argc = 1;
      char const* argv[argc] = { "--intvalue" };
      argh.parse(argc, argv);
    }

    TEST_METHOD(SkippedArgs)
    {
      Argh argh;
      int i1, i2;
      argh.addOption<int>(i1, 789, "--intvalue", "Integer value");
      argh.addOption<int>(i2, 123, "--intvalue2", "Integer value 2");
      const int argc = 2;
      char const* argv[argc] = { "--intvalue", "--intvalue2" };
      argh.parse(argc, argv);
    }

    TEST_METHOD(FlagInCommandLine)
    {
      Argh argh;
      argh.addFlag("--flagvalue", "Flag value");
      const int argc = 1;
      char const* argv[argc] = { "--flagvalue" };
      argh.parse(argc, argv);
      Assert::IsTrue(argh.hasFlag("--flagvalue"));
    }

    TEST_METHOD(FlagInFile)
    {
      Argh argh;
      argh.addFlag("--flagvalue", "Flag value");
      argh.load("../argh.opts");
      Assert::IsTrue(argh.hasFlag("--flagvalue"));
    }

    TEST_METHOD(LoadAndParse)
    {
      Argh argh;
      const int argc = 4;
      char const* argv[argc] = { "--cl_only", "456", "--intvalue", "456" };
      int default_only;
      int cl_only;
      bool file_only;
      int cl_and_file;
      argh.addOption<int>(default_only, 789, "--default_only", "Default only");
      argh.addOption<int>(cl_only, 0, "--cl_only", "Command line only");
      argh.addOption<bool>(file_only, false, "--boolvalue", "File only");
      argh.addOption<int>(cl_and_file, 0, "--intvalue", "Command line and file");
      argh.load("../argh.opts");
      argh.parse(argc, argv);
      Assert::IsTrue(default_only == 789);
      Assert::IsTrue(cl_only == 456);
      Assert::IsTrue(file_only);
      Assert::IsTrue(cl_and_file == 456);
    }
	};
}