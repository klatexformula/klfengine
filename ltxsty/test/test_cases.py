import re
import os
import os.path
import sys
import unittest

sys.path.insert(0, '.')
import helpers

class _BaseTestCase(helpers.HelpersForKlfimplTests):
    def __init__(self, *, klfimpl_sty_path, test_cases_dir,
                 test_debug_dir, test_case_name,
                 latex_engine="pdflatex", dpi=180):
        super().__init__(klfimpl_sty_path=klfimpl_sty_path)
        self.test_cases_dir = test_cases_dir
        self.test_debug_dir = test_debug_dir
        self.test_case_name = test_case_name
        self.kwargs = dict(latex_engine=latex_engine, dpi=dpi)

    def run_test_case(self):

        self._testMethodName = "test_case" # ????

        print(f"Running test case {self.test_case_name} with kwargs={self.kwargs!r}")

        with open(os.path.join(self.test_cases_dir, self.test_case_name+".tex"), "r") as f:
            latex_code = f.read()

        # replace \XXXLATEXENGINEXXX in string
        latex_code = latex_code.replace(r"\XXXLATEXENGINEXXX", self.kwargs['latex_engine'])

        self.assertLatexCompilesToImage(
            latex_code=latex_code,
            image_filename=os.path.join(self.test_cases_dir, self.test_case_name+".png"),
            **self.kwargs,
            dir_copy_for_debug=self.test_debug_dir,
        )

klfimpl_sty_path = os.path.join(os.path.dirname(__file__), "..", "klfimpl.sty")
test_cases_dir = os.path.join(os.path.dirname(__file__), "cases")
test_debug_dir = os.path.join(os.path.dirname(__file__), "debug")
def make_test_case(test_case_name, **kwargs):
    test_case_name_id = test_case_name
    if kwargs:
        test_case_name_id += "_" \
            + "_".join(re.sub(r"[^a-zA-Z0-9]", "", "{}".format(v))
                       for (k, v) in sorted(kwargs.items()))
    if test_case_name_id in globals():
        raise ValueError("Non-unique test case ID {}".format(repr(test_case_name_id)))
    str_to_exec = f"""
class _Test_{test_case_name_id}(_BaseTestCase, unittest.TestCase):
    def __init__(self, **innerkwargs):
        super().__init__(
            klfimpl_sty_path=klfimpl_sty_path,
            test_cases_dir=test_cases_dir,
            test_debug_dir=test_debug_dir,
            test_case_name={test_case_name!r},
            **{kwargs!r},
            **innerkwargs
        )

    def test_case(self):
        print("Running case {test_case_name_id}")
        self.run_test_case()

global Test_{test_case_name_id}
Test_{test_case_name_id} = _Test_{test_case_name_id}
"""
    print(str_to_exec)
    exec(str_to_exec, globals(), locals())

    return test_case_name_id

#
# List test cases here, with appropriate settings if required
#
make_test_case("inline_a", latex_engine="pdflatex")
make_test_case("inline_a", latex_engine="xelatex")
make_test_case("inline_a", latex_engine="lualatex")

a = Test_inline_a_pdflatex()
a.test_case()

# # debug
# print(globals())


if __name__ == "__main__":
    unittest.main()
