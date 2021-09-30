import sys
import os.path
import datetime
import tempfile
import subprocess
import shutil
import distutils.spawn
import unittest


class HelpersForKlfimplTests:
    def __init__(self, *, klfimpl_sty_path):
        # find kpsewhich
        self.latex_bin_dir = os.environ.get("LATEX_BIN_DIR")
        if self.latex_bin_dir is None:
            self.kpsewhich = distutils.spawn.find_executable("kpsewhich")
            if self.kpsewhich is None:
                self.kpsewhich = "kpsewhich"
            self.latex_bin_dir = subprocess.check_output(
                [self.kpsewhich, "-var-value=SELFAUTOLOC"]
            ).decode('utf-8').strip()
        else:
            self.kpsewhich = os.path.join(self.latex_bin_dir, "kpsewhich")

        self.gs_executable_path = os.environ.get("GS")
        if self.gs_executable_path is None:
            self.gs_executable_path = distutils.spawn.find_executable("gs")

        self.magick_path = os.environ.get("MAGICK")
        if self.magick_path is None:
            self.magick_path = distutils.spawn.find_executable("magick")

        self.klfimpl_sty_path = klfimpl_sty_path

    # ------------------
    
    def assertLatexCompilesToImage(
            self,
            *,
            latex_code,
            image_filename,
            latex_engine,
            dpi,
            dir_copy_for_debug=None,
            ):

        output_image_filename = "klfimpltest-gs-out.png"

        with tempfile.TemporaryDirectory() as tempdirname:
            try:
                # copy our klfimpl.sty file there
                shutil.copy(self.klfimpl_sty_path, tempdirname)

                # compile the latex code to a PNG image
                self.compileLatexToPngImage(
                    work_dir=tempdirname,
                    latex_code=latex_code,
                    output_image_filename=output_image_filename,
                    latex_engine=latex_engine,
                    dpi=dpi,
                    pngdevice="png16m",
                )

                # now, compare with reference image
                self.assertImagesCloseEnough(
                    os.path.join(tempdirname, output_image_filename),
                    image_filename,
                    diff_image=os.path.join(tempdirname, "diff-compare-result.png")
                )
            except Exception as e:
                # copy the image outside of temporary directory, for debugging
                if dir_copy_for_debug is not None:
                    outd = tempfile.mkdtemp(
                        prefix="dbg"+datetime.datetime.now().strftime('%Y%m%d_%H%M%S_'),
                        dir=dir_copy_for_debug
                    )
                    shutil.copytree(tempdirname, outd, dirs_exist_ok=True)
                    print("ERROR IN TEST --- FILES COPIED TO {} FOR YOUR DEBUGGING CONVENIENCE"
                          .format(outd))
                raise

    def assertImagesCloseEnough(self, image1, image2, diff_image=None):

        try:
            subprocess.run(
                [
                    self.magick_path,
                    "compare",
                    "-verbose",
                    "-define",  "profile:skip=ICC",
                    "-metric",  "AE",
                    image1,
                    image2,
                    diff_image if diff_image else "/dev/null"
                ],
                check=True,
            )
        except subprocess.CalledProcessError:
            raise AssertionError("Images {} and {} are not close".format(image1, image2))


    # ------------------

    def compileLatexToPngImage(self, *,
                               work_dir, latex_code,
                               output_image_filename, # absolute or relative to work_dir
                               latex_engine,
                               dpi,
                               pngdevice="pngalpha",
                               jobname="klfimpltestmain"):

        #
        # create LaTeX source file
        #
        with open(os.path.join(work_dir, jobname+".tex"), "w") as fwr:
            fwr.write(latex_code)

        latexexe = os.path.join(self.latex_bin_dir, latex_engine)

        subprocess.run(
            [ latexexe, jobname+".tex" ],
            cwd=work_dir,
            check=True
        )

        gs_input_file_name = jobname+".pdf"
        if latex_engine == "latex":
            # need to run dvips to get input for GS
            subprocess.run(
                [ os.path.join(self.latex_bin_dir, "dvips"), jobname+".dvi" ],
                cwd=work_dir,
                check=True
            )
            gs_input_file_name = jobname+".ps"

        # got the PDF or PS, run ghostscript

        subprocess.run(
            [ self.gs_executable_path,
              "-q", "-dBATCH", "-dSAFER", "-dNOPAUSE",
              "-sDEVICE="+pngdevice,
              "-dMaxBitmap=2147483647",
              "-r{}".format(dpi),
              "-dTextAlphaBits=4",
              "-dGraphicsAlphaBits=4",
              "-sOutputFile="+output_image_filename,
              gs_input_file_name ],
            cwd=work_dir,
            check=True,
        )

        # got the PNG Image!
        return

