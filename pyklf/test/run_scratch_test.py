
import sys
sys.path.insert(0, ".")
sys.path.insert(0, "../build")

import _cxx_pyklf as pyklf

inp = pyklf.input(
    latex=r"Z = \int_0^\infty f(x)\,dx",
    font_size=12,
    preamble="\\usepackage{amsmath}",
    math_mode=(r"$\begin{aligned}", r"\end{aligned}$")
)

inp.latex_engine = "pdflatex"


inp.parameters = {
    #"baseline_rule": False
    "baseline_rule": {"type": "line", "setup": r"\color{green}", "thickness": r"0.25pt"}
}
inp.preamble += r"\usepackage{xcolor}"

print(pyklf.margins(0.0, 3.0))
print(pyklf.margins(*[0.5, 1.5, 2.52, 3.53]))

#inp.fg_color = pyklf.color(0,80,80)
inp.margins = pyklf.margins(1,3.5120421,1,3.5001)

pdf_data = pyklf.klfimplpkg_engine.compile_to(inp, "PDF")

#print(pdf_data)

with open("tmp-klf-test-output.pdf", "wb") as f:
    f.write(pdf_data)
