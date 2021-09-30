import re
import os
import os.path
import sys
import unittest

sys.path.insert(0, '.')
import helpers

class _InnerTestCaseRunner(helpers.HelpersForKlfimplTests):
    def setup_runner(self, *, klfimpl_sty_path, test_cases_dir, test_debug_dir):
        self.test_cases_dir = test_cases_dir
        self.test_debug_dir = test_debug_dir
        super().setup_helper(klfimpl_sty_path=klfimpl_sty_path)

    def run_test_case(self, test_case_name,
                      latex_engine="pdflatex", dpi=180):

        kwargs = dict(latex_engine=latex_engine, dpi=dpi)

        print(f"Running test case {test_case_name} with kwargs={kwargs!r}")

        with open(os.path.join(self.test_cases_dir, test_case_name+".tex"), "r") as f:
            latex_code = f.read()

        # replace \XXXLATEXENGINEXXX in string
        latex_code = latex_code.replace(r"\XXXLATEXENGINEXXX", kwargs['latex_engine'])

        self.assertLatexCompilesToImage(
            latex_code=latex_code,
            image_filename=os.path.join(self.test_cases_dir, test_case_name+".png"),
            **kwargs,
            dir_copy_for_debug=self.test_debug_dir,
        )


class TestRunCases(unittest.TestCase, _InnerTestCaseRunner):
    def __init__(self, *args, **kwargs):
        print("Yo Yo!")

        self.klfimpl_sty_path = os.path.join(os.path.dirname(__file__), "..", "klfimpl.sty")
        self.test_cases_dir = os.path.join(os.path.dirname(__file__), "cases")
        self.test_debug_dir = os.path.join(os.path.dirname(__file__), "debug")
        
        print(f"Using {self.klfimpl_sty_path=}, {self.test_cases_dir=}, {self.test_debug_dir=}")
        
        print(self.__dict__)

        super().__init__(*args, **kwargs)

        self.setup_runner(
            klfimpl_sty_path=self.klfimpl_sty_path,
            test_cases_dir=self.test_cases_dir,
            test_debug_dir=self.test_debug_dir,
        )

# looks like we need all test_* methods to be defined on the class itself, not
# only on the instances.

def _make_case_test(tstcls, test_case_name, **kwargs):
    print("Working")
    test_case_name_id = 'test_' + test_case_name
    if kwargs:
        test_case_name_id += "_" \
            + "_".join(re.sub(r"[^a-zA-Z0-9]", "", "{}".format(v))
                       for (k, v) in sorted(kwargs.items()))
    if hasattr(tstcls, test_case_name_id):
        raise ValueError("Internal error, non-unique test name identifier {}"
                         .format(test_case_name_id))
    setattr(
        tstcls,
        test_case_name_id,
        lambda slf, test_case_name=test_case_name, kwargs2=dict(kwargs):
            slf.run_test_case(test_case_name, **kwargs2)
    )
    print("Added method {}".format(test_case_name_id))


_make_case_test(TestRunCases, "inline_a", latex_engine="pdflatex")
_make_case_test(TestRunCases, "inline_a", latex_engine="xelatex")
_make_case_test(TestRunCases, "inline_a", latex_engine="lualatex")

_make_case_test(TestRunCases, "inline_text", latex_engine="pdflatex")

_make_case_test(TestRunCases, "inlinemathaligned")

_make_case_test(TestRunCases, "inlinemathalignedcolor")



print("Go!")

# def make_test_case(test_case_name, **kwargs):
#     test_case_name_id = test_case_name
#     if kwargs:
#         test_case_name_id += "_" \
#             + "_".join(re.sub(r"[^a-zA-Z0-9]", "", "{}".format(v))
#                        for (k, v) in sorted(kwargs.items()))
#     if test_case_name_id in globals():
#         raise ValueError("Non-unique test case ID {}".format(repr(test_case_name_id)))
#     str_to_exec = f"""
# class _Test_{test_case_name_id}(_BaseTestCase, unittest.TestCase):
#     def __init__(self, **innerkwargs):
#         super().__init__(
#             klfimpl_sty_path=klfimpl_sty_path,
#             test_cases_dir=test_cases_dir,
#             test_debug_dir=test_debug_dir,
#             test_case_name={test_case_name!r},
#             **{kwargs!r},
#             **innerkwargs
#         )

#     def test_case(self):
#         print("Running case {test_case_name_id}")
#         self.run_test_case()

# global Test_{test_case_name_id}
# Test_{test_case_name_id} = _Test_{test_case_name_id}
# """
#     print(str_to_exec)
#     exec(str_to_exec, globals(), locals())

#     return test_case_name_id

# #
# # List test cases here, with appropriate settings if required
# #
# make_test_case("inline_a", latex_engine="pdflatex")
# make_test_case("inline_a", latex_engine="xelatex")
# make_test_case("inline_a", latex_engine="lualatex")

# a = Test_inline_a_pdflatex()
# a.test_case()

# # debug
# print(globals())


if __name__ == "__main__":
    unittest.main()
